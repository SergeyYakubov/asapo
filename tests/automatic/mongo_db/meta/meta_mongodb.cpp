#include <iostream>
#include <chrono>

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
    std::string keyword;
};

Args GetArgs(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Wrong number of arguments" << std::endl;
        exit(EXIT_FAILURE);
    }
    return Args{argv[1]};
}


int main(int argc, char* argv[]) {
    auto args = GetArgs(argc, argv);
    asapo::MongoDBClient db;

    std::string json;
    if (args.keyword == "parseerror") {
        json = R"("id1":{"test1":2}})";
    } else {
        json = R"({"id1":{"test1":2}})";
    }

    auto stream_name = R"(stream/test_/\ ."$)";


    if (args.keyword != "Notconnected") {
        db.Connect("127.0.0.1", "test");
    }

    auto mode = asapo::MetaIngestMode{asapo::MetaIngestOp::kReplace, true};
    auto err = db.InsertMeta("meta", "0", reinterpret_cast<const uint8_t*>(json.c_str()), json.size(), mode);
    if (err) {
        std::cout << err->Explain() << std::endl;
    }
    Assert(err, args.keyword);

    err = db.InsertMeta("meta", "0", reinterpret_cast<const uint8_t*>(json.c_str()), json.size(), mode);
    if (err) {
        std::cout << err->Explain() << std::endl;
    }
    Assert(err, args.keyword);

    if (args.keyword == "OK") {
        asapo::MetaIngestMode mode{asapo::MetaIngestOp::kInsert, false};
        std::string meta = R"({"data":"test"})";
        err =
            db.InsertMeta("meta", stream_name, reinterpret_cast<const uint8_t*>(meta.c_str()), meta.size(), mode);
        M_AssertEq(nullptr, err);
        err =
            db.InsertMeta("meta", stream_name, reinterpret_cast<const uint8_t*>(meta.c_str()), meta.size(), mode);
        M_AssertTrue(err == asapo::DBErrorTemplates::kDuplicateID);
        mode.op = asapo::MetaIngestOp::kReplace;
        err =
            db.InsertMeta("meta", stream_name, reinterpret_cast<const uint8_t*>(meta.c_str()), meta.size(), mode);
        M_AssertEq(nullptr, err);
        err = db.InsertMeta("meta", "notexist", reinterpret_cast<const uint8_t*>(meta.c_str()), meta.size(), mode);
        M_AssertTrue(err == asapo::DBErrorTemplates::kInsertError);

        std::string meta_res;
        err = db.GetMetaFromDb("meta", "0", &meta_res);
        M_AssertEq(meta_res, json);


        db.DeleteStream(stream_name);
    }

    return 0;
}
