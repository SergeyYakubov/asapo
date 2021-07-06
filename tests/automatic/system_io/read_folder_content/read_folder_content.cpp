#include <iostream>

#include "asapo/io/io_factory.h"
#include "testing.h"

using asapo::IO;
using asapo::Error;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Wrong number of arguments" << std::endl;
        return 1;
    }
    std::string expect{argv[2]};

    Error err;
    auto io = std::unique_ptr<IO> {asapo::GenerateDefaultIO() };
    auto files = io->FilesInFolder(argv[1], &err);

    std::string result{};
    if (err == nullptr) {
        uint64_t id = 0;
        for(auto message_meta : files) {
            M_AssertEq(message_meta.id, ++id);
            if (message_meta.name == "1") {
                M_AssertEq(static_cast<uint64_t>(4), message_meta.size);
            }
            result += message_meta.name;
        }
    } else {
        result = err->Explain();
    }

    M_AssertContains(result, expect);

    return 0;
}
