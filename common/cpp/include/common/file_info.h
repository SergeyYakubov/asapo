#ifndef HIDRA2_FILE_INFO_H
#define HIDRA2_FILE_INFO_H

#include <cinttypes>
#include <chrono>
#include <memory>
#include <vector>
#include <string>

namespace hidra2 {

struct FileInfo {
    std::string base_name;
    std::string relative_path;
    std::chrono::system_clock::time_point modify_date;
    uint64_t size{0};
    uint64_t id{0};
    std::string Json() const {
        auto periods = modify_date.time_since_epoch().count();
        std::string s = "{\"_id\":" + std::to_string(id) + ","
                        "\"size\":" + std::to_string(size) + ","
                        "\"base_name\":\"" + base_name + "\","
                        "\"lastchange\":" + std::to_string(periods) + ","
                        "\"relative_path\":\"" + relative_path + "\"}";
        return s;
    }
};

using FileData = std::unique_ptr<uint8_t[]>;
using FileInfos = std::vector<FileInfo>;

}
#endif //HIDRA2_FILE_INFO_H
