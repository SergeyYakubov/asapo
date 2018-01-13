#include <iostream>


#include "../../../common/cpp/src/database/mongodb_client.h"
#include "testing.h"


using hidra2::M_AssertEq;
using hidra2::DBError;


void Assert(DBError error, const std::string& expect) {
    std::string result;
    switch (error) {
    case DBError::kConnectionError:
        result = "ConnectionError";
        break;
    case DBError::kAlreadyConnected:
        result = "AlreadyConnected";
        break;
    case DBError::kBadAddress:
        result = "BadAddress";
        break;
    default:
        result = "OK";
        break;
    }

    M_AssertEq(expect, result);
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
    hidra2::MongoDBClient db;

    auto err = db.Connect(args.address, args.database_name, args.collection_name);
    Assert(err, args.keyword);

    if (err == DBError::kNoError) {
        err = db.Connect(args.address, args.database_name, args.collection_name);
        Assert(err, "AlreadyConnected");
    }
    return 0;
}


