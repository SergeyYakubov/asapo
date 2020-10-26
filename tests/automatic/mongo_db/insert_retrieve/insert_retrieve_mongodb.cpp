#include <iostream>
#include <chrono>
#include <thread>

#include "../../../common/cpp/src/database/mongodb_client.h"
#include "testing.h"

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
    fi.timestamp = std::chrono::system_clock::now();
    fi.buf_id = 18446744073709551615ull;
    fi.source = "host:1234";

    if (args.keyword != "Notconnected") {
        db.Connect("127.0.0.1", "data");
    }

    auto err = db.Insert("data_test", fi, false);

    if (args.keyword == "DuplicateID") {
        Assert(err, "OK");
        err = db.Insert("data_test", fi, false);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    auto fi1 = fi;
    fi1.id = 123;
    fi1.timestamp = std::chrono::system_clock::now();
    db.Insert("data_test1", fi, false);
    db.Insert("data_test1", fi1, false);

    Assert(err, args.keyword);

    if (args.keyword == "OK") { // check retrieve
        asapo::FileInfo fi_db;
        asapo::MongoDBClient db_new;
        db_new.Connect("127.0.0.1", "data");
        err = db_new.GetById("data_test", fi.id, &fi_db);
        M_AssertTrue(fi_db == fi, "get record from db");
        M_AssertEq(nullptr, err);
        err = db_new.GetById("data_test", 0, &fi_db);
        Assert(err, "No record");

        asapo::StreamInfo info;
        err = db.GetStreamInfo("data_test", &info);
        M_AssertEq(nullptr, err);
        M_AssertEq(fi.id, info.last_id);

        err = db.GetLastStream(&info);
        M_AssertEq(nullptr, err);
        M_AssertEq(fi1.id, info.last_id);
        M_AssertEq("test1",info.name);
    }

    return 0;
}
