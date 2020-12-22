#include <iostream>
#include <chrono>

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

    asapo::MessageMeta fi;
    fi.size = 100;
    fi.name = "relpath/1";
    fi.timestamp = std::chrono::system_clock::now();
    fi.buf_id = 18446744073709551615ull;
    fi.source = "host:1234";
    fi.id = args.file_id;
    fi.dataset_substream = 10;

    uint64_t dataset_size = 2;

    if (args.keyword != "Notconnected") {
        db.Connect("127.0.0.1", "data");
    }

    auto err =  db.InsertAsDatasetMessage("test", fi, dataset_size, true);


    if (args.keyword == "DuplicateID") {
        Assert(err, "OK");
        err =  db.InsertAsDatasetMessage("test", fi, dataset_size, true);
        err =  db.InsertAsDatasetMessage("test", fi, dataset_size, false);
    }

    Assert(err, args.keyword);

    if (args.keyword == "OK") { // check retrieve
        asapo::MessageMeta fi_db;
        err = db.GetDataSetById("test", fi.dataset_substream,fi.id, &fi_db);
        M_AssertTrue(fi_db == fi, "get record from db");
        M_AssertEq(nullptr, err);
        err = db.GetDataSetById("test", 0, 0, &fi_db);
        Assert(err, "No record");
    }


    return 0;
}
