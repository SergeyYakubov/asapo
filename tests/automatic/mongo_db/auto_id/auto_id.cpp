#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>

#include "../../../common/cpp/src/database/mongodb_client.h"
#include "asapo/database/db_error.h"

#include "testing.h"
#include "asapo/common/data_structs.h"

using asapo::Error;

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;

std::atomic<uint64_t> global_count{0};

enum class Mode {
    kTransaction,
    kUpdateCounterThenIngest
};

struct Args {
    Mode mode;
    std::string str_mode;
    int n_threads;
    int n_messages_per_thread;
};

Args GetArgs(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Wrong number of arguments" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::string mode = argv[1];
    Args args;
    if (mode == "trans" ) {
        args.mode = Mode::kTransaction;
    } else if (mode == "seq" ) {
        args.mode = Mode::kUpdateCounterThenIngest;
    } else {
        printf("wrong mode");
        exit(1);
    }
    args.str_mode = mode;
    args.n_threads = atoi(argv[2]);
    args.n_messages_per_thread = atoi(argv[3]);
    return args;
}

void insert(const asapo::MongoDBClient& db, const std::string& name, asapo::MessageMeta fi, const Args& args) {
    auto start = fi.id;
    for (int i = 0; i < args.n_messages_per_thread; i++) {
        switch (args.mode) {
        case Mode::kTransaction:
            fi.id = 0;
            break;
        case Mode::kUpdateCounterThenIngest:
            fi.id = start + i + 1;
            break;
        default:
            abort();
        }
        uint64_t  inserted_id{0};
        Error err = db.Insert(std::string("data_") + name, fi, false, &inserted_id);
        if (err != nullptr) {
            printf("%s\n", err->Explain().c_str());
//            break;
        } else {
            if (inserted_id == 0) {
                M_AssertTrue(false);
            }
            global_count++;
        }
    }
}

int main(int argc, char* argv[]) {
    auto args = GetArgs(argc, argv);
    auto db_name = R"(data_/ \."$)";

    auto exec_next = [&args, db_name](int i) {
        asapo::MessageMeta fi;
        asapo::MongoDBClient db;
        fi.size = 100;
        fi.name = "relpath/1";
        fi.timestamp = std::chrono::system_clock::now();
        fi.buf_id = 18446744073709551615ull;
        fi.source = "host:1234";
        fi.id = args.n_messages_per_thread * i;
        db.Connect("127.0.0.1", db_name);
        insert(db, "stream", fi, args);
    };

    auto t1 = high_resolution_clock::now();

    std::vector<std::thread> threads;
    for (int i = 0; i < args.n_threads; i++) {
        threads.emplace_back(std::thread(exec_next, i));
    }
    for (auto& thread : threads) {
        thread.join();
    }

    auto messages_sent = global_count.load();

    printf("Sent %llu messages \n",  messages_sent);
    M_AssertTrue(messages_sent == args.n_threads * args.n_messages_per_thread);

    auto t2 = high_resolution_clock::now();
    auto ms_int = duration_cast<milliseconds>(t2 - t1).count();
    printf("mode: %s, throughput %llu messages/sec with %d threads\n", args.str_mode.c_str(), 1000 * messages_sent / ms_int,
           args.n_threads);

    asapo::MongoDBClient db;
    db.Connect("127.0.0.1", db_name);
    db.DeleteStream("stream");


    return 0;
}
