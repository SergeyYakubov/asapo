#include <iostream>


#include "../../../common/cpp/src/database/mongodb_client.h"
#include "testing.h"
#include "asapo/database/db_error.h"

using asapo::Error;

void Assert(const Error& error, const std::string& expect) {
    std::string result;
    if (error == nullptr) {
        result = "OK";
    } else {
        result = error->Explain();
    }
    M_AssertContains(result, expect);
}

struct Args {
    std::string address;
    std::string database_name;
    std::string collection_name;
    std::string keyword;

};

Args GetArgs(int argc, char* argv[]) {
    if (argc != 5) {
        std::cout << "Wrong number of arguments" << std::endl;
        exit(EXIT_FAILURE);
    }
    return Args{argv[1], argv[2], argv[3], argv[4]};
}


int main(int argc, char* argv[]) {
    auto args = GetArgs(argc, argv);
    asapo::MongoDBClient db;

    auto err = db.Connect(args.address, args.database_name);
    Assert(err, args.keyword);

    if (err == nullptr) {
        err = db.Connect(args.address, args.database_name);
        Assert(err, asapo::DBErrorTemplates::kAlreadyConnected.Generate()->Explain());
    }
    return 0;
}


