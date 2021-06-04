#include <iostream>
#include <chrono>
#include <thread>

#include "../../../common/cpp/src/database/mongodb_client.h"
#include "../../../common/cpp/src/database/encoding.h"

#include "testing.h"
#include "asapo/common/data_structs.h"

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
    printf("%s %s",argv[1],argv[2]) ;
    return Args{argv[1], atoi(argv[2])};
}


int main(int argc, char* argv[]) {
    auto args = GetArgs(argc, argv);
    asapo::MongoDBClient db;

    asapo::MessageMeta fi;
    fi.size = 100;
    fi.name = "relpath/1";
    fi.id = args.file_id;
    fi.timestamp = std::chrono::system_clock::now();
    fi.buf_id = 18446744073709551615ull;
    fi.source = "host:1234";
    
    auto db_name = R"(data_/ \."$)";
    auto stream_name = R"(bla/test_/\ ."$)";

    if (args.keyword != "Notconnected") {
        db.Connect("127.0.0.1", db_name);
    }

    auto err = db.Insert(std::string("data_")+stream_name, fi, false);

    if (args.keyword == "DuplicateID") {
        Assert(err, "OK");
        err = db.Insert(std::string("data_")+stream_name, fi, false);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    auto fi1 = fi;
    auto fi2 = fi;
    fi2.id = 123;
    fi1.timestamp = std::chrono::system_clock::now();
    fi2.timestamp = std::chrono::system_clock::now()+std::chrono::minutes(1);
    fi2.name = asapo::kFinishStreamKeyword;
    fi2.metadata=R"({"next_stream":"ns"})";
    db.Insert("data_test1", fi1, false);
    db.Insert("data_test1", fi2, false);

    Assert(err, args.keyword);

    if (args.keyword == "OK") { // check retrieve and stream delete
        asapo::MessageMeta fi_db;
        asapo::MongoDBClient db_new;
        db_new.Connect("127.0.0.1", db_name);
        err = db_new.GetById(std::string("data_")+stream_name, fi.id, &fi_db);
        M_AssertTrue(fi_db == fi, "get record from db");
        M_AssertEq(nullptr, err);
        err = db_new.GetById(std::string("data_")+stream_name, 0, &fi_db);
        Assert(err, "No record");


        asapo::StreamInfo info;

        err = db.GetStreamInfo(std::string("data_")+stream_name, &info);
        M_AssertEq(nullptr, err);
        M_AssertEq(fi.id, info.last_id);

        err = db.GetLastStream(&info);
        M_AssertEq(nullptr, err);
        M_AssertEq(fi2.id, info.last_id);
        M_AssertEq("test1", info.name);
        M_AssertEq(true, info.finished);
        M_AssertEq("ns",info.next_stream);

// delete stream
        db.Insert(std::string("inprocess_")+stream_name+"_blabla", fi, false);
        db.Insert(std::string("inprocess_")+stream_name+"_blabla1", fi, false);
        db.Insert(std::string("acks_")+stream_name+"_blabla", fi, false);
        db.Insert(std::string("acks_")+stream_name+"_blabla1", fi, false);
        db.DeleteStream(stream_name);
        err = db.GetStreamInfo(std::string("data_")+stream_name, &info);
        M_AssertTrue(info.last_id == 0);
        err = db.GetStreamInfo(std::string("inprocess_")+stream_name+"_blabla", &info);
        M_AssertTrue(info.last_id == 0);
        err = db.GetStreamInfo(std::string("inprocess_")+stream_name+"_blabla1", &info);
        M_AssertTrue(info.last_id == 0);
        err = db.GetStreamInfo(std::string("acks_")+stream_name+"_blabla", &info);
        M_AssertTrue(info.last_id == 0);
        err = db.GetStreamInfo(std::string("acks_")+stream_name+"_blabla1", &info);
        M_AssertTrue(info.last_id == 0);
        err = db.DeleteStream("test1");
        M_AssertTrue(err==nullptr);
    }

    return 0;
}
