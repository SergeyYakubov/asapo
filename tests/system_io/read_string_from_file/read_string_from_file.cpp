#include <iostream>
#include "io/io_factory.h"

#include "testing.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Wrong number of arguments" << std::endl;
        return 1;
    }
    std::string expect{argv[2]};

    hidra2::Error err;
    auto io = std::unique_ptr<hidra2::IO> {hidra2::GenerateDefaultIO()};
    auto str = io->ReadFileToString(argv[1], &err);

    std::string result;

    if (err == nullptr) {
        result = str;
    } else {
        result = err->Explain();
    }

    hidra2::M_AssertContains(result, expect);
    return 0;
}
