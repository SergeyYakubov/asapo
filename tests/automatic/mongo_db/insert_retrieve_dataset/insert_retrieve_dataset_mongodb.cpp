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

    auto err =  db.InsertAsDatasetMessage("data_test", fi, dataset_size, true);


    if (args.keyword == "DuplicateID") {
        Assert(err, "OK");
        err =  db.InsertAsDatasetMessage("data_test", fi, dataset_size, true);
        err =  db.InsertAsDatasetMessage("data_test", fi, dataset_size, false);
    }

    Assert(err, args.keyword);

    if (args.keyword == "OK") { // check retrieve
        asapo::MessageMeta fi_db;
        err = db.GetDataSetById("data_test", fi.dataset_substream,fi.id, &fi_db);
        M_AssertTrue(fi_db == fi, "get record from db");
        M_AssertEq(nullptr, err);
        err = db.GetDataSetById("data_test", 0, 0, &fi_db);
        Assert(err, "No record");

        asapo::StreamInfo info;

        err = db.GetStreamInfo("data_test", &info);
        M_AssertEq(nullptr, err);
        M_AssertEq(fi.id, info.last_id);

        asapo::StreamInfo info_last;

        err = db.GetLastStream(&info_last);
        M_AssertEq(nullptr, err);
        M_AssertEq("test", info_last.name);
        M_AssertEq(fi.id, info_last.last_id);
        M_AssertEq(false, info_last.finished);

        auto fi2 = fi;
        fi2.id = 123;
        fi2.timestamp = std::chrono::system_clock::now()+std::chrono::minutes(1);
        fi2.name = asapo::kFinishStreamKeyword;
        fi2.metadata=R"({"next_stream":"ns"})";
        db.Insert("data_test", fi2, false);
        err = db.GetLastStream(&info_last);
        M_AssertEq(nullptr, err);
        M_AssertEq("test", info_last.name);
        M_AssertEq(fi2.id, info_last.last_id);
        M_AssertEq(true, info_last.finished);
    }

    return 0;
}
