#include <iostream>
#include <memory>

#include <system_wrappers/system_io.h>

using hidra2::SystemIO;
using hidra2::IOErrors;


void M_AssertEq(const std::string& expected, const std::string& got) {
    if (expected != got) {
        std::cerr << "Assert failed:\n"
                  << "Expected:\t'" << expected << "'\n"
                  << "Obtained:\t'" << got << "'\n";
        abort();
    }
}

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
    case IOErrors::FOLDER_NOT_FOUND:
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
