#include <iostream>


#include "database/mongodb_client.h"
#include "testing.h"


using hidra2::M_AssertEq;
using hidra2::DBError;


void Assert(DBError error, const std::string& expect) {
    std::string result;
    switch (error) {
    case DBError::kImportError:
        result = "ImportError";
        break;
    case DBError::kNotConnected:
        result = "NotConnected";
        break;
    default:
        result = "OK";
        break;
    }

    M_AssertEq(expect, result);
}

struct Args {
    std::string instruction;
    std::string expect;

};

Args GetArgs(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Wrong number of arguments" << std::endl;
        exit(EXIT_FAILURE);
    }
    return Args{argv[1], argv[2]};
}


int main(int argc, char* argv[]) {
    auto args = GetArgs(argc, argv);
    hidra2::MongoDBClient db;


    hidra2::FileInfos files;
    hidra2::FileInfo fi;
    fi.size = 100;
    fi.base_name = "1";
    files.emplace_back(fi);
    fi.base_name = "2";
    files.emplace_back(fi);

    if (args.instruction != "notconnected") {
        db.Connect("127.0.0.1", "data", "test");
    }

    auto err = db.Import(files);

    Assert(err, args.expect);

    return 0;
}


