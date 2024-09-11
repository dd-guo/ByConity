#pragma once

#include "Common/config.h"
#include "Storages/SelectQueryInfo.h"
#if USE_HIVE

#include "Core/NamesAndTypes.h"
#include "Disks/IDisk.h"
#include "Processors/ISource.h"
#include "Storages/Hive/CnchHiveSettings.h"
#include "Storages/Hive/HiveFile/IHiveFile_fwd.h"
#include "Storages/MergeTree/KeyCondition.h"
#include "Storages/KeyDescription.h"

namespace DB
{

namespace Protos
{
    class ProtoHiveFile;
    class ProtoHiveFiles;
}

class IHiveFile
{
public:
    friend struct HiveFilesSerDe;
    enum class FileFormat
    {
        PARQUET,
        ORC,
        HUDI,
        InputSplit,
    };
    static FileFormat fromHdfsInputFormatClass(const String & class_name);
    static FileFormat fromFormatName(const String & format_name);
    static String toString(FileFormat format);

    /// a very basic factory method
    static HiveFilePtr create(
        FileFormat format,
        String file_path,
        size_t file_size,
        const DiskPtr & disk,
        const HivePartitionPtr & partition);

    virtual ~IHiveFile() = default;

    virtual void serialize(Protos::ProtoHiveFile & proto) const;
    virtual void deserialize(const Protos::ProtoHiveFile & proto);

    FileFormat getFormat() const { return format; }
    String getFormatName() const;
    std::unique_ptr<ReadBufferFromFileBase> readFile(const ReadSettings & settings = {}) const;

    virtual std::optional<size_t> numRows() { return {}; }

    // todo @caoliu impl this seconds
    UInt64 getLastModifiedTimestamp() { return 0; }

    struct ReadParams
    {
        size_t max_block_size;
        FormatSettings format_settings;
        ContextPtr context;
        std::optional<size_t> slice;
        ReadSettings read_settings;
        std::shared_ptr<SelectQueryInfo> query_info;
    };
    virtual SourcePtr getReader(const Block & block, const std::shared_ptr<ReadParams> & params);

    FileFormat format;
    String file_path;
    size_t file_size;

    DiskPtr disk;
    HivePartitionPtr partition;

protected:
    IHiveFile() = default;
    void load(FileFormat format, const String & file_path, size_t file_size, const DiskPtr & disk, const HivePartitionPtr & partition);
};

using HiveFilePtr = std::shared_ptr<IHiveFile>;
using HiveFiles = std::vector<HiveFilePtr>;

namespace RPCHelpers
{
    HiveFiles deserialize(
        const Protos::ProtoHiveFiles & proto,
        const ContextPtr & context,
        const StorageMetadataPtr & metadata,
        const CnchHiveSettings & settings);
}

}

#endif
