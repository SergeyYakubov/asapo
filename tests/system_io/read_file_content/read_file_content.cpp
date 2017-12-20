#include <iostream>
#include <system_wrappers/system_io.h>

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
    auto data = io->GetDataFromFile(argv[1], expect.size(), &err);

    std::string result;

    switch (err) {
    case IOErrors::FILE_NOT_FOUND:
        result = "notfound";
        break;
    case IOErrors::NO_ERROR:
        for(int i = 0; i < expect.size(); i++)
            result += data[i];
        break;
    case IOErrors::PERMISSIONS_DENIED:
        result = "noaccess";
        break;
    case IOErrors::READ_ERROR:
        result = "readerror";
        break;

    default:
        result = "";
        break;
    }

    M_AssertEq(expect, result);
    return 0;
}
