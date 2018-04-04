#include <iostream>
#include <system_wrappers/system_io.h>

#include "testing.h"

using hidra2::SystemIO;
using hidra2::Error;
using hidra2::FileData;


struct Params {
    std::string fname;
    std::string result;
    std::string message;
    int length;
};

Params GetParams(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Wrong number of arguments" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::string fname{argv[1]};
    std::string result{argv[2]};
    std::string message{argv[3]};

    return Params{fname, result, message,3};
}

void AssertGoodResult(const std::unique_ptr<SystemIO>& io, const Error& err, const FileData& data,
                      const Params& params) {
    if (err) {
        std::cerr << err << std::endl;
        exit(EXIT_FAILURE);
    }
    Error read_err;
    auto read_data = io->GetDataFromFile(params.fname, params.length, &read_err);
    hidra2::M_AssertContains(std::string(read_data.get(), read_data.get() + params.length), "123");
}

void AssertBadResult(const Error& err, const Params& params) {
    if (err == nullptr) {
        std::cerr << "Should be error" << std::endl;
        exit(EXIT_FAILURE);
    }
    hidra2::M_AssertContains(err->Explain(), params.message);
}


int main(int argc, char* argv[]) {
    auto params = GetParams(argc, argv);

    auto io = std::unique_ptr<SystemIO> {new SystemIO};
    FileData data{new uint8_t[params.length]{'1', '2', '3'}};

    auto err = io->WriteDataToFile(params.fname, data, params.length);

    if (params.result == "ok") {
        AssertGoodResult(io, err, data, params);
    } else {
        AssertBadResult(err, params);
    }

    return 0;
}
