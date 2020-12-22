#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>
#include "asapo/consumer/consumer.h"
#include "testing.h"

void Assert(std::vector<asapo::MessageMetas> message_metas, int nthreads, int nfiles) {
    std::vector<std::string> expect, result;
    for (int i = 1; i <= nfiles; i++) {
        expect.push_back(std::to_string(i));
    }
    int nfiles_read = 0;
    for (int i = 0; i < nthreads; i++) {
        nfiles_read += message_metas[i].size();
        for (const auto& fi : message_metas[i]) {
            result.push_back(fi.name);
        }
    }
    // file names created by setup.sh should be '1','2',... Each thread should access different files.
    if (nfiles != nfiles_read) {
        std::cout << "nfiles != nfiles_read" << std::endl;
        exit(EXIT_FAILURE);
    }
//    M_AssertEq(nfiles, nfiles_read);
    if (!std::is_permutation(expect.begin(), expect.end(), result.begin())) {
        std::cout << "!std::is_permutation" << std::endl;
        exit(EXIT_FAILURE);
    }
}

struct Args {
    std::string server;
    std::string run_name;
    std::string token;
    int nthreads;
    int nfiles;
};

Args GetArgs(int argc, char* argv[]) {
    if (argc != 6) {
        std::cout << "Wrong number of arguments" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::string server{argv[1]};
    std::string source_name{argv[2]};
    int nthreads = std::stoi(argv[3]);
    int nfiles = std::stoi(argv[4]);
    std::string token{argv[5]};

    return Args{server, source_name, token, nthreads, nfiles};
}

void TestAll(const Args& args) {
    asapo::Error err;
    auto consumer = asapo::ConsumerFactory::CreateConsumer(args.server,
                                                         "dummy",
                                                         true,
                                                         asapo::SourceCredentials{asapo::SourceType::kProcessed,
                                                                                  args.run_name, "", "", args.token},
                                                         &err);
    if (err) {
        std::cout << "Error CreateConsumer: " << err << std::endl;
        exit(EXIT_FAILURE);
    }

    auto group_id = consumer->GenerateNewGroupId(&err);
    consumer->SetTimeout(10000);
    std::vector<asapo::MessageMetas>message_metas(args.nthreads);
    auto exec_next = [&](int i) {
        asapo::MessageMeta fi;
        while ((err = consumer->GetNext(&fi, group_id, nullptr)) == nullptr) {
            message_metas[i].emplace_back(fi);
        }
        printf("%s\n", err->Explain().c_str());
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < args.nthreads; i++) {
        threads.emplace_back(std::thread(exec_next, i));
    }

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    Assert(message_metas, args.nthreads, args.nfiles);
}

int main(int argc, char* argv[]) {

    auto args = GetArgs(argc, argv);

    TestAll(args);
    return 0;
}
