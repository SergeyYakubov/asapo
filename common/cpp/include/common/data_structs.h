#ifndef ASAPO_FILE_INFO_H
#define ASAPO_FILE_INFO_H

#include <cinttypes>
#include <chrono>
#include <memory>
#include <vector>
#include <string>

#include "error.h"

namespace asapo {

class JsonStringParser;

uint64_t NanosecsEpochFromTimePoint(std::chrono::high_resolution_clock::time_point);
uint64_t  EpochNanosecsFromNow();
std::chrono::high_resolution_clock::time_point TimePointfromNanosec(uint64_t nanoseconds_from_epoch);


bool TimeFromJson(const JsonStringParser& parser, const std::string& name, std::chrono::high_resolution_clock::time_point* val);

class FileInfo {
  public:
    std::string name;
    std::chrono::high_resolution_clock::time_point timestamp;
    uint64_t size{0};
    uint64_t id{0};
    std::string source;
    std::string metadata;
    uint64_t buf_id{0};
    std::string Json() const;
    bool SetFromJson(const std::string& json_string);
    std::string FullName(const std::string& base_path) const;
};


struct StreamInfo {
    uint64_t last_id{0};
    std::string name;
    std::chrono::high_resolution_clock::time_point timestamp;
    std::string Json(bool add_last_id) const;
    bool SetFromJson(const std::string& json_string,bool read_last_id);
};

using StreamInfos = std::vector<StreamInfo>;

inline bool operator==(const FileInfo& lhs, const FileInfo& rhs) {
    return  (lhs.name == rhs.name &&
             lhs.id == rhs.id &&
             lhs.timestamp == rhs.timestamp &&
             lhs.size == rhs.size);
}

using FileData = std::unique_ptr<uint8_t[]>;

using FileInfos = std::vector<FileInfo>;


using IdList = std::vector<uint64_t>;

struct DataSet {
    uint64_t id;
    FileInfos content;
    bool SetFromJson(const std::string& json_string);
};

using SubDirList = std::vector<std::string>;

enum class SourceType {
  kProcessed,
  kRaw
};

Error GetSourceTypeFromString(std::string stype,SourceType *type);
std::string GetStringFromSourceType(SourceType type);

struct SourceCredentials {
    SourceCredentials(SourceType type, std::string beamtime, std::string beamline, std::string stream, std::string token):
        beamtime_id{std::move(beamtime)},
        beamline{std::move(beamline)},
        stream{std::move(stream)},
        user_token{std::move(token)},
        type{type}{};
    SourceCredentials() {};
    static const std::string kDefaultStream;
    static const std::string kDefaultBeamline;
    static const std::string kDefaultBeamtimeId;
    std::string beamtime_id;
    std::string beamline;
    std::string stream;
    std::string user_token;
    SourceType type = SourceType::kProcessed;
    std::string GetString() {
        return (type==SourceType::kRaw?std::string("raw"):std::string("processed")) + "%"+ beamtime_id + "%" + beamline + "%" + stream + "%" + user_token;
    };
};

enum IngestModeFlags : uint64_t {
    kTransferData = 1 << 0,
    kTransferMetaDataOnly = 1 << 1,
    kStoreInFilesystem = 1 << 2,
};

const uint64_t kDefaultIngestMode = kTransferData | kStoreInFilesystem;

const std::string kDefaultSubstream = "default";


}
#endif //ASAPO_FILE_INFO_H
