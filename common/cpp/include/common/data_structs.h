#ifndef ASAPO_FILE_INFO_H
#define ASAPO_FILE_INFO_H

#include <cinttypes>
#include <chrono>
#include <memory>
#include <vector>
#include <string>

namespace asapo {

class FileInfo {
  public:
    std::string name;
    std::chrono::system_clock::time_point modify_date;
    uint64_t size{0};
    uint64_t id{0};
    std::string source;
    std::string metadata;
    uint64_t buf_id{0};
    std::string Json() const;
    bool SetFromJson(const std::string& json_string);
    std::string FullName(const std::string& base_path) const;
};

inline bool operator==(const FileInfo& lhs, const FileInfo& rhs) {
    return  (lhs.name == rhs.name &&
             lhs.id == rhs.id &&
             lhs.modify_date == rhs.modify_date &&
             lhs.size == rhs.size);

}

using FileData = std::unique_ptr<uint8_t[]>;

using FileInfos = std::vector<FileInfo>;
using SubDirList = std::vector<std::string>;

}
#endif //ASAPO_FILE_INFO_H
