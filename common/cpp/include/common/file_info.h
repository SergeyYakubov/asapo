#ifndef HIDRA2_FILE_INFO_H
#define HIDRA2_FILE_INFO_H

#include <cinttypes>
#include <chrono>
#include <memory>
#include <vector>

namespace hidra2 {

struct FileInfo {
    std::string base_name;
    std::string relative_path;
    std::chrono::system_clock::time_point modify_date;
    uint64_t size{0};
};

using FileData = std::unique_ptr<uint8_t[]>;
using FileInfos = std::vector<FileInfo>;

}
#endif //HIDRA2_FILE_INFO_H
