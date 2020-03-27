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


    if (args.keyword != "Notconnected") {
        db.Connect("127.0.0.1", "test");
    }

    auto err = db.Upsert("meta", 0, reinterpret_cast<const uint8_t*>(json.c_str()), json.size());
    if (err) {
        std::cout << err->Explain() << std::endl;
    }

    Assert(err, args.keyword);

    err = db.Upsert("meta", 0, reinterpret_cast<const uint8_t*>(json.c_str()), json.size());
    if (err) {
        std::cout << err->Explain() << std::endl;
    }

    Assert(err, args.keyword);


    return 0;
}
