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

    auto data = io->GetDataFromFile(argv[1], expect.size(), &err);

    std::string result;

    if (err == nullptr) {
        for(unsigned int i = 0; i < expect.size(); i++)
            result += data[i];
    } else {
        result = err->Explain();
    }

    hidra2::M_AssertContains(result, expect);
    return 0;
}
