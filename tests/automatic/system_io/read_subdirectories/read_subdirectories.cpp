#include <iostream>

#include "asapo/io/io_factory.h"
#include "testing.h"

using asapo::IO;
using asapo::Error;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Wrong number of arguments" << std::endl;
        return EXIT_FAILURE;
    }
    std::string expect{argv[2]};

    Error err;
    auto io = std::unique_ptr<IO> {asapo::GenerateDefaultIO() };
    auto subdirs = io->GetSubDirectories(argv[1], &err);

    std::string result{};
    if (err == nullptr) {
        for(auto folder : subdirs) {
            result += folder;
        }
    } else {
        result = err->Explain();
    }

    M_AssertContains(result, expect);

    return EXIT_SUCCESS;
}
