#ifndef HIDRA2_FILE_INFO_H
#define HIDRA2_FILE_INFO_H

#include <cinttypes>
#include <chrono>
#include <memory>
#include <vector>
#include <string>

namespace hidra2 {

class FileInfo {
  public:
    std::string base_name;
    std::string relative_path;
    std::chrono::system_clock::time_point modify_date;
    uint64_t size{0};
    uint64_t id{0};
    std::string Json() const;
    bool SetFromJson(const std::string& json_string);
    std::string FullName(const std::string& base_path);
};

inline bool operator==(const FileInfo& lhs, const FileInfo& rhs) {
    return  (lhs.base_name == rhs.base_name &&
             lhs.id == rhs.id &&
             lhs.relative_path == rhs.relative_path &&
             lhs.modify_date == rhs.modify_date &&
             lhs.size == rhs.size);

}


using FileData = std::unique_ptr<uint8_t[]>;
using FileInfos = std::vector<FileInfo>;

}
#endif //HIDRA2_FILE_INFO_H