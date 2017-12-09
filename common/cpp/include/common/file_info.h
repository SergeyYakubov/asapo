#ifndef HIDRA2_FILE_INFO_H
#define HIDRA2_FILE_INFO_H

#include <cinttypes>
#include <vector>
#include <chrono>

namespace hidra2 {

struct FileInfo {
    std::string base_name;
    std::string relative_path;
    std::chrono::system_clock::time_point modify_date;
};

typedef std::vector<uint8_t> FileData;


}
#endif //HIDRA2_FILE_INFO_H
