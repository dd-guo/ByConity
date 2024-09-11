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

#pragma once

#include <Core/Types.h>
#include <IO/WriteHelpers.h>
#include <Common/HostWithPorts.h>


namespace DB
{
namespace Protos
{
    class AddressInfo;
}

    class WriteBuffer;
    class ReadBuffer;

    class AddressInfo
    {
    public:
        AddressInfo() = default;
        AddressInfo(const String & host_name_, UInt16 port_, const String & user_, const String & password_);
        AddressInfo(const String & host_name_, UInt16 port_, const String & user_, const String & password_, UInt16 exchange_port_);
        AddressInfo(const Protos::AddressInfo & proto_);

        void serialize(WriteBuffer &) const;
        void deserialize(ReadBuffer &);
        void toProto(Protos::AddressInfo & proto) const;
        void fillFromProto(const Protos::AddressInfo & proto);

        const String & getHostName() const { return host_name; }
        UInt16 getPort() const { return port; }
        UInt16 getExchangePort() const { return exchange_port;}
        const String & getUser() const { return user; }
        const String & getPassword() const { return password; }

        String toString() const;
        String toShortString() const;
        inline bool operator == (AddressInfo const& rhs) const
        {
            return (this->host_name == rhs.host_name && this->port == rhs.port);
        }
        inline bool operator < (AddressInfo const& rhs) const
        {
            int ret = host_name.compare(rhs.host_name);
            if (ret)
                return ret < 0;
            return port < rhs.port;
        }
        class Hash
        {
        public:
            size_t operator()(const AddressInfo & key) const
            {
                return std::hash<std::string_view>{}(key.host_name) + static_cast<size_t>(key.port);
            }
        };

    private:
        String host_name;
        UInt16 port;
        String user;
        String password;
        UInt16 exchange_port;
    };

    using AddressInfos = std::vector<AddressInfo>;

    inline String extractHostPort(const AddressInfo & address) { return createHostPortString(address.getHostName(), address.getPort()); }
    inline String extractExchangeHostPort(const AddressInfo & address) {return createHostPortString(address.getHostName(), toString(address.getExchangePort())); }


}
