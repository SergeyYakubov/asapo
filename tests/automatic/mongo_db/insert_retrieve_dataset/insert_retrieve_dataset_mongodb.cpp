#include <iostream>
#include <chrono>

#include "../../../common/cpp/src/database/mongodb_client.h"
#include "testing.h"


using asapo::M_AssertContains;
using asapo::M_AssertTrue;

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
    uint64_t subset_id = args.file_id;
    fi.modify_date = std::chrono::system_clock::now();
    fi.buf_id = 18446744073709551615ull;
    fi.source = "host:1234";
    fi.id = 10;

    uint64_t subset_size = 2;

    if (args.keyword != "Notconnected") {
        db.Connect("127.0.0.1", "data");
    }

    auto err =  db.InsertAsSubset("test", fi, subset_id, subset_size, true);


    if (args.keyword == "DuplicateID") {
        Assert(err, "OK");
        fi.id = 2;
        err =  db.InsertAsSubset("test", fi, subset_id, subset_size, true);
//        Assert(err, "OK");
        err =  db.InsertAsSubset("test", fi, subset_id, subset_size, false);
    }

    Assert(err, args.keyword);

    if (args.keyword == "OK") { // check retrieve
        asapo::FileInfo fi_db;
        err = db.GetDataSetById("test", subset_id, fi.id, &fi_db);
        M_AssertTrue(fi_db == fi, "get record from db");
        Assert(err, "OK");
        err = db.GetDataSetById("test", 0, 0, &fi_db);
        Assert(err, "No record");
    }


    return 0;
}
