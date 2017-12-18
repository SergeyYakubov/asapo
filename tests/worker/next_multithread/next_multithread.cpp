#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include <thread>
#include <string>

#include "worker/data_broker.h"
#include "testing.h"

using hidra2::M_AssertEq;
using hidra2::M_AssertTrue;

using hidra2::WorkerErrorCode;

void Assert(std::vector<hidra2::FileInfo> file_infos, int nthreads) {
    int nfiles = file_infos.size();
    M_AssertEq(nthreads, nfiles);

    std::vector<std::string> expect, result;
    for (int i = 0; i < nthreads; i++) {
        expect.push_back(std::to_string(i));
        result.push_back(file_infos[i].base_name);
    }
    // file names created by setup.sh should be '0','1',... Each thread should access different file.
    M_AssertTrue(std::is_permutation(expect.begin(), expect.end(), result.begin()));
}

struct Args {
    std::string folder;
    int nthreads;
    int nattempts;
};

Args GetArgs(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Wrong number of arguments" << std::endl;
        abort();
    }
    std::string folder{argv[1]};
    int nthreads = std::stoi(argv[2]);
    int nattempts = std::stoi(argv[3]);
    return Args{folder, nthreads, nattempts};
}

void ReadFiles(const Args& args) {
    hidra2::WorkerErrorCode err;
    auto broker = hidra2::DataBrokerFactory::Create(args.folder, &err);
    broker->Connect();

    std::vector<hidra2::FileInfo>file_infos(args.nthreads);
    auto exec_next = [&](int i) {
        broker->GetNext(&file_infos[i], nullptr);
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < args.nthreads; i++) {
        threads.push_back(std::thread(exec_next, i));
    }

    for (auto& thread : threads) {
        thread.join();
    }
    Assert(file_infos, args.nthreads);
}

int main(int argc, char* argv[]) {

    auto args = GetArgs(argc, argv);

    for (int nattempt = 0; nattempt < args.nattempts; nattempt++) {
        ReadFiles(args);
    }
    return 0;
}
