#include <iostream>
#include <memory>

#include "system_wrappers/system_io.h"
#include "testing.h"

using hidra2::SystemIO;
using hidra2::IOErrors;

using hidra2::M_AssertEq;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Wrong number of arguments" << std::endl;
        return 1;
    }
    std::string expect{argv[2]};

    IOErrors err;
    auto io = std::unique_ptr<SystemIO> {new SystemIO};
    auto files = io->FilesInFolder(argv[1], &err);

    std::string result;

    switch (err) {
    case IOErrors::FILE_NOT_FOUND:
        result = "notfound";
        break;
    case IOErrors::NO_ERROR:
        for(auto file_info : files)
            result += file_info.relative_path + file_info.base_name;
        break;
    case IOErrors::PERMISSIONS_DENIED:
        result = "noaccess";
        break;
    default:
        result = "";
        break;
    }

    M_AssertEq(expect, result);

    return 0;
}
