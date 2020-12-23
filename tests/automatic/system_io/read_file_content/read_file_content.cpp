#include <iostream>
#include "asapo/io/io_factory.h"

#include "testing.h"


int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Wrong number of arguments" << std::endl;
        return 1;
    }
    std::string expect{argv[2]};

    asapo::Error err;
    auto io = std::unique_ptr<asapo::IO> {asapo::GenerateDefaultIO()};

    asapo::MessageData data;
    uint64_t size = 0;
    if (expect == "unknown_size") {
        data = io->GetDataFromFile(argv[1], &size, &err);
    } else {
        size = expect.size();
        data = io->GetDataFromFile(argv[1], &size, &err);
    }

    std::string result;

    if (err == nullptr) {
        for(unsigned int i = 0; i < expect.size(); i++)
            result += data[i];
    } else {
        result = err->Explain();
    }

    M_AssertContains(result, expect);
    return 0;
}
