#include <iostream>
#include "io/io_factory.h"

#include "testing.h"

using asapo::IO;
using asapo::Error;
using asapo::FileData;


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

    return Params{fname, result, message, 3};
}

void AssertGoodResult(const std::unique_ptr<IO>& io, const Error& err, const FileData& data,
                      const Params& params) {
    if (err) {
        std::cerr << err << std::endl;
        exit(EXIT_FAILURE);
    }
    Error read_err;
    auto read_data = io->GetDataFromFile(params.fname, params.length, &read_err);
    asapo::M_AssertContains(std::string(read_data.get(), read_data.get() + params.length), "123");
}

void AssertBadResult(const Error& err, const Params& params) {
    if (err == nullptr) {
        std::cerr << "Should be error" << std::endl;
        exit(EXIT_FAILURE);
    }
    asapo::M_AssertContains(err->Explain(), params.message);
}


int main(int argc, char* argv[]) {
    auto params = GetParams(argc, argv);

    auto io = std::unique_ptr<asapo::IO> {asapo::GenerateDefaultIO()};
    auto array = new uint8_t[params.length];
    array[0] = '1';
    array[1] = '2';
    array[2] = '3';
    FileData data{array};

    auto err = io->WriteDataToFile(params.fname, data, params.length);

    if (params.result == "ok") {
        AssertGoodResult(io, err, data, params);
    } else {
        AssertBadResult(err, params);
    }

    return 0;
}
