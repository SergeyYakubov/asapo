#include <iostream>

#include "system_wrappers/system_io.h"
#include "testing.h"

using hidra2::SystemIO;
using hidra2::Error;


using hidra2::M_AssertEq;
using hidra2::M_AssertContains;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Wrong number of arguments" << std::endl;
        return 1;
    }
    std::string expect{argv[2]};

    Error err;
    auto io = std::unique_ptr<SystemIO> {new SystemIO};
    auto files = io->FilesInFolder(argv[1], &err);

    std::string result{};
    if (err == nullptr) {
        int64_t id = 0;
        for(auto file_info : files) {
            M_AssertEq(file_info.id, ++id);
            result += file_info.relative_path + file_info.base_name;
        }
    } else {
        result = err->Explain();
    }

    M_AssertContains(result, expect);

    return 0;
}
