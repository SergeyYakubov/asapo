#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>
#include "worker/data_broker.h"
#include "testing.h"

using asapo::M_AssertEq;
using asapo::M_AssertTrue;

void Assert(std::vector<asapo::FileInfos> file_infos, int nthreads, int nfiles) {
    std::vector<std::string> expect, result;
    for (int i = 1; i <= nfiles; i++) {
        expect.push_back(std::to_string(i));
    }
    int nfiles_read = 0;
    for (int i = 0; i < nthreads; i++) {
        nfiles_read += file_infos[i].size();
        for (const auto& fi : file_infos[i]) {
            result.push_back(fi.name);
        }
    }
    // file names created by setup.sh should be '1','2',... Each thread should access different files.
    M_AssertEq(nfiles, nfiles_read);
    M_AssertTrue(std::is_permutation(expect.begin(), expect.end(), result.begin()));
}

struct Args {
    std::string server;
    std::string run_name;
    int nthreads;
    int nfiles;
};

Args GetArgs(int argc, char* argv[]) {
    if (argc != 5) {
        std::cout << "Wrong number of arguments" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::string server{argv[1]};
    std::string source_name{argv[2]};
    int nthreads = std::stoi(argv[3]);
    int nfiles = std::stoi(argv[4]);
    return Args{server, source_name, nthreads, nfiles};
}

void GetAllFromBroker(const Args& args) {
    asapo::Error err;
    auto broker = asapo::DataBrokerFactory::CreateServerBroker(args.server, args.run_name, &err);

    std::vector<asapo::FileInfos>file_infos(args.nthreads);
    auto exec_next = [&](int i) {
        asapo::FileInfo fi;
        while ((err = broker->GetNext(&fi, nullptr)) == nullptr) {
            file_infos[i].emplace_back(fi);
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < args.nthreads; i++) {
        threads.emplace_back(std::thread(exec_next, i));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    Assert(file_infos, args.nthreads, args.nfiles);
}

int main(int argc, char* argv[]) {

    auto args = GetArgs(argc, argv);

    GetAllFromBroker(args);
    return 0;
}
