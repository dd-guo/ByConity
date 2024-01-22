#include "Storages/DataLakes/StorageCnchLas.h"
#if USE_HIVE and USE_JAVA_EXTENSIONS

#include <jni/JNIMetaClient.h>
#include <hudi.pb.h>
#include "Interpreters/evaluateConstantExpression.h"
#include "Parsers/ASTCreateQuery.h"
#include "Parsers/ASTLiteral.h"
#include <Protos/hive_models.pb.h>
#include "Storages/Hive/CnchHiveSettings.h"
#include "Storages/Hive/HivePartition.h"
#include "Storages/Hive/HiveFile/IHiveFile.h"
#include "Storages/Hive/HiveWhereOptimizer.h"
#include "Storages/Hive/Metastore/JNIHiveMetastore.h"
#include "Storages/Hive/StorageCnchHive.h"
#include "Storages/StorageFactory.h"

namespace DB
{
namespace ErrorCodes
{
    extern const int NUMBER_OF_ARGUMENTS_DOESNT_MATCH;
    extern const int NOT_IMPLEMENTED;
}

static constexpr auto LAS_CLASS = "org/byconity/las/LasMetaClient";

StorageCnchLas::StorageCnchLas(
    const StorageID & table_id_,
    const String & hive_metastore_url_,
    const String & hive_db_name_,
    const String & hive_table_name_,
    StorageInMemoryMetadata metadata_,
    ContextPtr context_,
    IMetaClientPtr client,
    std::shared_ptr<CnchHiveSettings> settings_)
    : StorageCnchHive(table_id_, hive_metastore_url_, hive_db_name_, hive_table_name_, std::nullopt, context_, nullptr, settings_)
{
    std::unordered_map<String, String> params = {
        {"database_name", hive_db_name_},
        {"table_name", hive_table_name_},
        {"metastore_uri", hive_metastore_url_},
        {"endpoint", storage_settings->endpoint},
        {"access_key", storage_settings->ak_id},
        {"secret_key", storage_settings->ak_secret},
        {"region", storage_settings->region},
    };

    DB::Protos::HudiMetaClientParams req;
    auto * prop = req.mutable_properties();
    for (const auto &kv : params)
    {
        auto *proto_kv = prop->add_properties();
        proto_kv->set_key(kv.first);
        proto_kv->set_value(kv.second);
    }

    auto metaclient = std::make_shared<JNIMetaClient>(LAS_CLASS, req.SerializeAsString());
    if (!client)
    {
        auto cli = std::make_shared<JNIHiveMetastoreClient>(std::move(metaclient), std::move(params));
        jni_meta_client = cli.get();
        client = std::move(cli);
    }
    else
        throw Exception(ErrorCodes::NOT_IMPLEMENTED, "StorageLas does not support external catalog");

    setHiveMetaClient(std::move(client));

    Stopwatch watch;
    initialize(metadata_);
    LOG_TRACE(log, "Elapsed {} ms to getTable from metastore", watch.elapsedMilliseconds());
}

PrepareContextResult StorageCnchLas::prepareReadContext(
    const Names & column_names,
    const StorageMetadataPtr & metadata_snapshot,
    SelectQueryInfo & query_info,
    ContextPtr & local_context,
    [[maybe_unused]] unsigned num_streams)
{
    metadata_snapshot->check(column_names, getVirtuals(), getStorageID());
    HiveWhereOptimizer optimizer(metadata_snapshot, query_info);

    Stopwatch watch;
    HivePartitions partitions = selectPartitions(local_context, metadata_snapshot, query_info, optimizer);
    LOG_TRACE(log, "Elapsed {} ms to select {} required partitions", watch.elapsedMilliseconds(), partitions.size());

    /// TODO: This looks very hacky I know
    /// but should be valid, because jni does not allow multi thread access
    auto & read_properties = jni_meta_client->getProperties();
    read_properties["input_format"] = hive_table->sd.inputFormat;
    read_properties["serde"] = hive_table->sd.serdeInfo.serializationLib;
    read_properties["hive_column_names"] = fmt::format("{}", fmt::join(getHiveColumnNames(), ","));
    read_properties["hive_column_types"] = fmt::format("{}", fmt::join(getHiveColumnTypes(), "#"));

    const auto & settings = local_context->getSettingsRef();
    read_properties["buffer_size"] = std::to_string(settings.max_read_buffer_size);

    watch.restart();
    HiveFiles hive_files;
    for (const auto & partition : partitions)
    {
        HiveFiles res = hive_client->getFilesInPartition(partition);
        hive_files.insert(hive_files.end(), std::make_move_iterator(res.begin()), std::make_move_iterator(res.end()));
    }

    LOG_TRACE(log, "Elapsed {} ms to get {} FileSplits", watch.elapsedMilliseconds(), hive_files.size());
    PrepareContextResult result{.hive_files = std::move(hive_files)};
    collectResource(local_context, result);
    return result;
}

StorageID StorageCnchLas::prepareTableRead(const Names & output_columns, SelectQueryInfo & query_info, ContextPtr local_context)
{
    auto prepare_result = prepareReadContext(output_columns, getInMemoryMetadataPtr(), query_info, local_context, 1);
    StorageID storage_id = getStorageID();
    storage_id.table_name = prepare_result.local_table_name;
    return storage_id;
}

std::optional<TableStatistics> StorageCnchLas::getTableStats([[maybe_unused]] const Strings & columns, [[maybe_unused]] ContextPtr local_context)
{
    /// TODO:
    return {};
}

void StorageCnchLas::serializeHiveFiles(Protos::ProtoHiveFiles & proto, const HiveFiles & hive_files)
{
    for (const auto & hive_file : hive_files)
    {
        auto * proto_file = proto.add_files();
        hive_file->serialize(*proto_file);
    }

    LOG_TRACE(log, "Hive files {}" + proto.DebugString());
}

Strings StorageCnchLas::getHiveColumnTypes() const
{
    Strings res;
    res.reserve(hive_table->sd.cols.size());
    for (const auto & col : hive_table->sd.cols)
        res.push_back(col.type);
    return res;
}

Strings StorageCnchLas::getHiveColumnNames() const
{
    Strings res;
    res.reserve(hive_table->sd.cols.size());
    for (const auto & col : hive_table->sd.cols)
        res.push_back(col.name);
    return res;
}

void registerStorageLas(StorageFactory & factory)
{
    StorageFactory::StorageFeatures features{
        .supports_settings = true,
        .supports_sort_order = true,
        .supports_schema_inference = true,
    };

    factory.registerStorage("CnchLas", [](const StorageFactory::Arguments & args)
    {
        ASTs & engine_args = args.engine_args;
        if (engine_args.size() != 3)
            throw Exception(ErrorCodes::NUMBER_OF_ARGUMENTS_DOESNT_MATCH,
                "Storage CnchHudi require 3 arguments: hive_metastore_url, hudi_db_name and hudi_table_name.");

        for (auto & engine_arg : engine_args)
            engine_arg = evaluateConstantExpressionOrIdentifierAsLiteral(engine_arg, args.getLocalContext());

        String hive_metastore_url = engine_args[0]->as<ASTLiteral &>().value.safeGet<String>();
        String hive_database = engine_args[1]->as<ASTLiteral &>().value.safeGet<String>();
        String hive_table = engine_args[2]->as<ASTLiteral &>().value.safeGet<String>();

        StorageInMemoryMetadata metadata;
        std::shared_ptr<CnchHiveSettings> hive_settings = std::make_shared<CnchHiveSettings>(args.getContext()->getCnchHiveSettings());
        if (args.storage_def->settings)
        {
            hive_settings->loadFromQuery(*args.storage_def);
            metadata.settings_changes = args.storage_def->settings->ptr();
        }

        if (!args.columns.empty())
            metadata.setColumns(args.columns);

        metadata.setComment(args.comment);

        if (args.storage_def->partition_by)
        {
            ASTPtr partition_by_key;
            partition_by_key = args.storage_def->partition_by->ptr();
            metadata.partition_key = KeyDescription::getKeyFromAST(partition_by_key, metadata.columns, args.getContext());
        }

        return std::make_shared<StorageCnchLas>(
            args.table_id,
            hive_metastore_url,
            hive_database,
            hive_table,
            std::move(metadata),
            args.getContext(),
            args.hive_client,
            hive_settings);
    },
    features);
}

}

#endif
