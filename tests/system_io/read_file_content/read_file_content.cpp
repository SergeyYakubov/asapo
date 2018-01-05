#include <iostream>
#include <system_wrappers/system_io.h>

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
    auto data = io->GetDataFromFile(argv[1], expect.size(), &err);

    std::string result;

    switch (err) {
    case IOError::kFileNotFound:
        result = "notfound";
        break;
    case IOError::kNoError:
        for(int i = 0; i < expect.size(); i++)
            result += data[i];
        break;
    case IOError::kPermissionDenied:
        result = "noaccess";
        break;
    case IOError::kReadError:
        result = "readerror";
        break;

    default:
        result = "";
        break;
    }

    M_AssertEq(expect, result);
    return 0;
}
