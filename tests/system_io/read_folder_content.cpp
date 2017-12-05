#include <iostream>
#include <string>

#include <system_wrappers/system_io.h>

using hidra2::SystemIO;
using hidra2::IOErrors;

#


void M_Assert(bool expr, std::string expected, std::string got, std::string msg) {
    if (!expr) {
        std::cerr << "Assert failed:\t" << msg << "\n"
                  << "Expected:\t'" << expected << "'\n"
                  << "Obtained:\t'" << got << "'\n";
        abort();
    }
}


int main(int argc, char* argv[]) {
    auto io = new SystemIO;

    if (argc != 3) {
        std::cout << "Wrong number of arguments" << std::endl;
        return 1;
    }

    std::string expect{argv[2]};

    IOErrors err;

    auto files = io->FilesInFolder(argv[1], &err);

    switch (err) {
    case IOErrors::FOLDER_NOT_FOUND:
        M_Assert(expect.compare("notfound")==0,expect,"notfound","Folder not found");
        return 0;
    }

    return 1;
}
