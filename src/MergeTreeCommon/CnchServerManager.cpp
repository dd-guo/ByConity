/*
 * Copyright (2022) Bytedance Ltd. and/or its affiliates
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <MergeTreeCommon/CnchServerManager.h>

#include <Common/Exception.h>
#include <Catalog/Catalog.h>
#include <Storages/PartCacheManager.h>
#include <ServiceDiscovery/IServiceDiscovery.h>

#include <Coordination/LeaderElection.h>
#include <Coordination/Defines.h>

namespace DB
{

CnchServerManager::CnchServerManager(ContextPtr context_, const Poco::Util::AbstractConfiguration & config)
    : WithContext(context_)
    , LeaderElectionBase(getContext()->getConfigRef().getUInt64("server_master.election_check_ms", 100))
{
    auto task_func = [this] (String task_name, std::function<bool ()> func, UInt64 interval, std::atomic<UInt64> & last_time, BackgroundSchedulePool::TaskHolder & task)
    {
        /// Check schedule delay
        UInt64 start_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        if (last_time && start_time > last_time && (start_time - last_time) > (1000 + interval))
            LOG_WARNING(log, "{} schedules over {}ms. Last finish time: {}, current time: {}", task_name, (1000 + interval), last_time, start_time);

        bool success = false;
        try
        {
            success = func();
        }
        catch (...)
        {
            tryLogCurrentException(log, __PRETTY_FUNCTION__);
        }
        /// Check execution time
        UInt64 finish_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        if (finish_time > start_time && finish_time - start_time > 1000)
            LOG_WARNING(log, "{} executed over 1000ms. Start time: {}, current time: {}", task_name, start_time, finish_time);
        
        last_time = finish_time;
        auto schedule_delay = success ? interval 
                                      : getContext()->getSettingsRef().topology_retry_interval_ms.totalMilliseconds(); 
        task->scheduleAfter(schedule_delay);        
    };

    topology_refresh_task = getContext()->getTopologySchedulePool().createTask("TopologyRefresher", [this, task_func](){
        task_func("TopologyRefresher"
            , [this]() -> bool { return refreshTopology(); }
            , getContext()->getSettingsRef().topology_refresh_interval_ms.totalMilliseconds()
            , refresh_topology_time
            , topology_refresh_task);
    });

    lease_renew_task = getContext()->getTopologySchedulePool().createTask("LeaseRenewer", [this, task_func](){
        task_func("LeaseRenewer"
            , [this]() -> bool { return renewLease(); }
            , getContext()->getSettingsRef().topology_lease_renew_interval_ms.totalMilliseconds()
            , renew_lease_time
            , lease_renew_task);
    });

    async_query_status_check_task = getContext()->getTopologySchedulePool().createTask("AsyncQueryStatusChecker", [this, task_func](){
        UInt64 async_query_status_check_interval = getContext()->getRootConfig().async_query_status_check_period * 1000;
        task_func("AsyncQueryStatusChecker"
            , [this]() -> bool { return checkAsyncQueryStatus(); }
            , async_query_status_check_interval
            , async_query_status_check_time
            , async_query_status_check_task);
    });

    updateServerVirtualWarehouses(config);
    if (!getContext()->hasZooKeeper())
    {
        // throw Exception(ErrorCodes::NOT_IMPLEMENTED, "Can't start server manage due to lack of zookeeper");
        LOG_ERROR(log, "There is no zookeeper, skip start background task for serverManager");

        onLeader();
        return;
    }

    startLeaderElection(getContext()->getTopologySchedulePool());
}

CnchServerManager::~CnchServerManager()
{
    shutDown();
}

void CnchServerManager::onLeader()
{
    /// sleep to prevent multiple leaders from appearing at the same time
    std::this_thread::sleep_for(std::chrono::milliseconds(wait_ms));

    auto current_address = getContext()->getHostWithPorts().getRPCAddress();

    try
    {
        is_leader = true;
        setLeaderStatus();
        lease_renew_task->activateAndSchedule();
        topology_refresh_task->activateAndSchedule();
        async_query_status_check_task->activateAndSchedule();
    }
    catch (...)
    {
        LOG_ERROR(log, "Failed to set leader status when current node becoming leader.");
    }

    LOG_DEBUG(log, "Current node {} become leader", current_address);
}


void CnchServerManager::enterLeaderElection()
{
    try
    {
        auto current_address = getContext()->getHostWithPorts().getRPCAddress();
        auto election_path = getContext()->getConfigRef().getString("server_master.election_path", SERVER_MASTER_ELECTION_DEFAULT_PATH);

        current_zookeeper = getContext()->getZooKeeper();
        current_zookeeper->createAncestors(election_path + "/");

        leader_election = std::make_unique<zkutil::LeaderElection>(
            getContext()->getTopologySchedulePool(),
            election_path,
            *current_zookeeper,
            [&]() { onLeader(); },
            current_address,
            false
        );
    }
    catch (...)
    {
        tryLogCurrentException(__PRETTY_FUNCTION__);
    }
}

void CnchServerManager::exitLeaderElection()
{
    partialShutdown();
}

bool CnchServerManager::refreshTopology()
{
    try
    {
        /// Stop and wait for next leader-election
        if (!is_leader || !leader_initialized)
            return true;

        auto service_discovery_client = getContext()->getServiceDiscoveryClient();
        String psm = getContext()->getConfigRef().getString("service_discovery.server.psm", "data.cnch.server");
        HostWithPortsVec server_vector = service_discovery_client->lookup(psm, ComponentType::SERVER);

        /// zookeeper is nullptr means there is no leader election available,
        /// in this case, we now only support one server in cluster.
        if (!current_zookeeper && server_vector.size() > 1)
        {
            LOG_ERROR(log, "More than one server in cluster without leader-election is not supported now, stop refreshTopology, psm: {}", psm);
            return true;
        }

        if (!server_vector.empty())
        {
            /// keep the servers sorted by host address to make it comparable
            std::sort(server_vector.begin(), server_vector.end(), [](auto & lhs, auto & rhs) {
                return std::forward_as_tuple(lhs.id, lhs.getHost(), lhs.rpc_port) < std::forward_as_tuple(rhs.id, rhs.getHost(), rhs.rpc_port);
            });

            {
                std::unique_lock lock(topology_mutex);
                auto temp_topology = Topology();
                for (const auto & server : server_vector)
                {
                    const auto & hostname = server.id;
                    String server_vw_name;
                    for (auto vw_it = server_virtual_warehouses.begin(); vw_it != server_virtual_warehouses.end(); ++vw_it)
                    {
                        if (hostname.starts_with(vw_it->first))
                        {
                            server_vw_name = vw_it->second;
                            break;
                        }
                    }
                    if (server_vw_name.empty())
                        temp_topology.addServer(server);
                    else
                        temp_topology.addServer(server, server_vw_name);
                }
                if (cached_topologies.empty() || !cached_topologies.back().isSameTopologyWith(temp_topology))
                    next_version_topology = temp_topology;
            }
        }
        else
        {
            LOG_ERROR(log, "Failed to get any server from service discovery, psm: {}", psm);
            return false;
        }
    }
    catch (...)
    {
        tryLogCurrentException(log, __PRETTY_FUNCTION__);
        return false;
    }

    return true;
}

bool CnchServerManager::renewLease()
{
    try
    {
        if (!is_leader)
            return true;

        if (!leader_initialized)
            setLeaderStatus();

        UInt64 current_time_ms = getContext()->getTimestamp() >> 18;
        UInt64 lease_life_ms = getContext()->getSettings().topology_lease_life_ms.totalMilliseconds();

        std::list<Topology> copy_topologies;
        {
            std::unique_lock lock(topology_mutex);

            ///clear outdated lease
            while (!cached_topologies.empty() && cached_topologies.front().getExpiration() < current_time_ms)
            {
                /// At least keep one topology to avoid no lease renew happens
                if (cached_topologies.size() == 1 && !next_version_topology)
                    break;
                LOG_DEBUG(log, "Removing expired topology : {}", cached_topologies.front().format());
                cached_topologies.pop_front();
            }

            if (cached_topologies.empty())
            {
                if (next_version_topology)
                {
                    next_version_topology->setExpiration(current_time_ms + lease_life_ms);
                    cached_topologies.push_back(*next_version_topology);
                    LOG_DEBUG(log, "Add new topology {}", cached_topologies.back().format());
                }
                else
                {
                    LOG_WARNING(log, "Cannot renew lease because there is no topology. Current ts : {}, current topology is empty.", current_time_ms);
                }
            }
            else if (cached_topologies.size() == 1)
            {
                if (next_version_topology)
                {
                    UInt64 latest_lease_time = cached_topologies.back().getExpiration();
                    next_version_topology->setExpiration(latest_lease_time + lease_life_ms);
                    cached_topologies.push_back(*next_version_topology);
                    LOG_DEBUG(log, "Add new topology {}", cached_topologies.back().format());
                }
                else
                {
                    cached_topologies.back().setExpiration(current_time_ms + lease_life_ms);
                }
            }
            else
            {
                LOG_WARNING(log, "Cannot renew lease because there is one pending topology. Current ts : {}, current topology : {}", current_time_ms, dumpTopologies(cached_topologies));
            }

            next_version_topology.reset();
            copy_topologies = cached_topologies;
        }

        getContext()->getCnchCatalog()->updateTopologies(copy_topologies);
    }
    catch (...)
    {
        tryLogCurrentException(__PRETTY_FUNCTION__);
        return false;
    }

    return true;
}

bool CnchServerManager::checkAsyncQueryStatus()
{
    /// Mark inactive jobs to failed.
    try
    {
        auto statuses = getContext()->getCnchCatalog()->getIntermidiateAsyncQueryStatuses();
        std::vector<Protos::AsyncQueryStatus> to_expire;
        for (const auto & status : statuses)
        {
            /// Find the expired statuses.
            UInt64 start_time = static_cast<UInt64>(status.start_time());
            UInt64 execution_time = static_cast<UInt64>(status.max_execution_time());
            /// TODO(WangTao): We could have more accurate ways to expire status whose execution time is unlimited, like check its real status from host server.
            if (execution_time == 0)
                execution_time = getContext()->getRootConfig().async_query_expire_time;
            if (time(nullptr) - start_time > execution_time)
            {
                to_expire.push_back(std::move(status));
            }
        }

        if (!to_expire.empty())
        {
            LOG_INFO(log, "Mark {} async queries to failed.", to_expire.size());
            getContext()->getCnchCatalog()->markBatchAsyncQueryStatusFailed(to_expire, "Status expired");
        }
    }
    catch (...)
    {
        tryLogCurrentException(__PRETTY_FUNCTION__);
        return false;
    }

    return true;
}

void CnchServerManager::setLeaderStatus()
{
    std::unique_lock lock(topology_mutex);
    cached_topologies = getContext()->getCnchCatalog()->getTopologies();
    leader_initialized = true;
    LOG_DEBUG(log , "Successfully set leader status.");
}

void CnchServerManager::shutDown()
{
    try
    {
        need_stop = true;
        lease_renew_task->deactivate();
        topology_refresh_task->deactivate();
        async_query_status_check_task->deactivate();
        leader_election.reset();
    }
    catch (...)
    {
        tryLogCurrentException(log);
    }
}

/// call me when zookeeper is expired
void CnchServerManager::partialShutdown()
{
    try
    {
        is_leader = false;
        leader_initialized = false;

        leader_election.reset();
        lease_renew_task->deactivate();
        topology_refresh_task->deactivate();
        async_query_status_check_task->deactivate();
    }
    catch (...)
    {
        tryLogCurrentException(log);
    }
}

void CnchServerManager::updateServerVirtualWarehouses(const Poco::Util::AbstractConfiguration & config, const String & config_name)
{
    Poco::Util::AbstractConfiguration::Keys config_keys;
    config.keys(config_name, config_keys);

    std::unordered_map<String, String> new_server_virtual_warehouses;

    for (const auto & server_virtual_warehouse : config_keys)
    {
        Poco::Util::AbstractConfiguration::Keys host_keys;
        auto prefix = config_name + "." + server_virtual_warehouse;
        config.keys(prefix, host_keys);
        for (const auto & host : host_keys)
        {
            auto host_name = config.getString(prefix + "." + host);
            if (new_server_virtual_warehouses.contains(host_name))
            {
                LOG_ERROR(log, "Invalid server virtual warehouse config, host `" + host_name + "` duplicated.");
                return;
            }
            else
            {
                new_server_virtual_warehouses[host_name] = server_virtual_warehouse;
            }
        }
    }

    std::lock_guard lock(topology_mutex);
    server_virtual_warehouses = new_server_virtual_warehouses;
}

void CnchServerManager::dumpServerStatus() const
{
    std::stringstream ss;
    ss << "[leader selection result] : \n"
       << "is_leader : " <<  is_leader << std::endl
       << "[tasks info] : \n"
       << "current_time : " << time(nullptr) << std::endl
       << "refresh_topology_time : " << refresh_topology_time << std::endl
       << "renew_lease_time : " << renew_lease_time << std::endl
       << "async_query_status_check_time : " << async_query_status_check_time << std::endl
       << "[current cached topology] : \n";

    {
        std::unique_lock lock(topology_mutex, std::defer_lock);
        if (lock.try_lock_for(std::chrono::milliseconds(1000)))
        {
            ss << DB::dumpTopologies(cached_topologies);
        }
        else
        {
            ss << "Failed to accquire server manager lock";
        }
    }
    LOG_INFO(log, "Dump server status :\n{}", ss.str());
}


}
