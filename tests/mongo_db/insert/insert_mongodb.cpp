#include <iostream>
#include <chrono>

#include "database/mongodb_client.h"
#include "testing.h"


using hidra2::M_AssertEq;
using hidra2::DBError;


void Assert(DBError error, const std::string& expect) {
    std::string result;
    switch (error) {
    case DBError::kInsertError:
        result = "InsertError";
        break;
    case DBError::kNotConnected:
        result = "NotConnected";
        break;
    case DBError::kDuplicateID:
        result = "DuplicateID";
        break;

    default:
        result = "OK";
        break;
    }

    M_AssertEq(expect, result);
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
    hidra2::MongoDBClient db;

    hidra2::FileInfo fi;
    fi.size = 100;
    fi.base_name = "1";
    fi.id = args.file_id;
    fi.relative_path = "relpath";
    fi.modify_date = std::chrono::system_clock::now();

    if (args.keyword != "NotConnected") {
        db.Connect("127.0.0.1", "data", "test");
    }

    auto err = db.Insert(fi);

    if (args.keyword == "DuplicateID") {
        Assert(err, "OK");
        err = db.Insert(fi);
    }

    Assert(err, args.keyword);



    return 0;
}
