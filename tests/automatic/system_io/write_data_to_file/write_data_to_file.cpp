#include <iostream>
#include "asapo/io/io_factory.h"

#include "testing.h"

using asapo::IO;
using asapo::Error;
using asapo::FileData;


struct Args {
    std::string fname;
    std::string result;
    std::string message;
    uint64_t length;
};

Args GetParams(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Wrong number of arguments" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::string fname{argv[1]};
    std::string result{argv[2]};
    std::string message{argv[3]};

    return Args{fname, result, message, 3};
}

void AssertGoodResult(const std::unique_ptr<IO>& io, const Error& err, const FileData& data,
                      const Args& params) {
    if (err) {
        std::cerr << err << std::endl;
        exit(EXIT_FAILURE);
    }
    Error read_err;
    uint64_t size = params.length;
    auto read_data = io->GetDataFromFile(params.fname, &size, &read_err);
    M_AssertContains(std::string(read_data.get(), read_data.get() + params.length), "123");
}

void AssertBadResult(const Error& err, const Args& params) {
    if (err == nullptr) {
        std::cerr << "Should be error" << std::endl;
        exit(EXIT_FAILURE);
    }
    M_AssertContains(err->Explain(), params.message);
}


int main(int argc, char* argv[]) {
    auto params = GetParams(argc, argv);

    auto io = std::unique_ptr<asapo::IO> {asapo::GenerateDefaultIO()};
    auto array = new uint8_t[params.length] {'1', '2', '3'};
    FileData data{array};

    auto err = io->WriteDataToFile("", params.fname, data, params.length, true, true);

    if (params.result == "ok") {
        AssertGoodResult(io, err, data, params);
        // check allow_overwrite works
        auto err = io->WriteDataToFile("", params.fname, data, params.length, true, false);
        params.message = asapo::IOErrorTemplates::kFileAlreadyExists.Generate()->Explain();
        AssertBadResult(err, params);
    } else {
        AssertBadResult(err, params);
    }

    return 0;
}
