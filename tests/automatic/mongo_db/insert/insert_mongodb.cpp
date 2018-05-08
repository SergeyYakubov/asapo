#include <iostream>
#include <chrono>

#include "../../../common/cpp/src/database/mongodb_client.h"
#include "testing.h"


using asapo::M_AssertContains;
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
    std::string keyword;
    int file_id;
};

Args GetArgs(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Wrong number of arguments" << std::endl;
        exit(EXIT_FAILURE);
    }
    return Args{argv[1], atoi(argv[2])};
}


int main(int argc, char* argv[]) {
    auto args = GetArgs(argc, argv);
    asapo::MongoDBClient db;

    asapo::FileInfo fi;
    fi.size = 100;
    fi.name = "relpath/1";
    fi.id = args.file_id;
    fi.modify_date = std::chrono::system_clock::now();

    if (args.keyword != "Notconnected") {
        db.Connect("127.0.0.1", "data", "test");
    }

    auto err = db.Insert(fi, false);

    if (args.keyword == "DuplicateID") {
        Assert(err, "OK");
        err = db.Insert(fi, false);
    }

    Assert(err, args.keyword);

    return 0;
}