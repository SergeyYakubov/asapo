#include <iostream>

#include "system_wrappers/system_io.h"
#include "testing.h"

using hidra2::SystemIO;
using hidra2::IOError;

using hidra2::M_AssertEq;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Wrong number of arguments" << std::endl;
        return 1;
    }
    std::string expect{argv[2]};

    IOError err;
    auto io = std::unique_ptr<SystemIO> {new SystemIO};
    auto files = io->FilesInFolder(argv[1], &err);

    std::string result;
    int64_t id = 0;
    switch (err) {
    case IOError::kFileNotFound:
        result = "notfound";
        break;
    case IOError::kNoError:
        for(auto file_info : files) {
            M_AssertEq(file_info.id, ++id);
            result += file_info.relative_path + file_info.base_name;
        }
        break;
    case IOError::kPermissionDenied:
        result = "noaccess";
        break;
    default:
        result = "";
        break;
    }

    M_AssertEq(expect, result);

    return 0;
}
