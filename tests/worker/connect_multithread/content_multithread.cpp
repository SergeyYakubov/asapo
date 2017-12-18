#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include <thread>

#include "worker/data_broker.h"
#include "testing.h"

using hidra2::M_AssertEq;
using hidra2::WorkerErrorCode;

void Assert(std::vector<WorkerErrorCode>& errors, int nthreads) {
    int count_ok = std::count(std::begin(errors),
                              std::end(errors),
                              WorkerErrorCode::OK);

    int count_already_connected = std::count(std::begin(errors),
                                             std::end(errors),
                                             WorkerErrorCode::SOURCE_ALREADY_CONNECTED);

    M_AssertEq(1, count_ok);
    M_AssertEq(nthreads - 1, count_already_connected);
}

struct Args {
    std::string folder;
    int nthreads;
};

Args GetArgs(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Wrong number of arguments" << std::endl;
        abort();
    }
    std::string folder{argv[1]};
    int nthreads = std::stoi(argv[2]);
    return Args{folder, nthreads};
}

int main(int argc, char* argv[]) {

    auto args = GetArgs(argc, argv);

    hidra2::WorkerErrorCode err;
    auto broker = hidra2::DataBrokerFactory::Create(args.folder, &err);

    std::vector<WorkerErrorCode>errors(args.nthreads, WorkerErrorCode::UNKNOWN_IO_ERROR);

    std::vector<std::thread> threads;
    for (int i = 0; i < args.nthreads; i++) {
        threads.push_back(std::thread([&, i] {
            errors[i] = broker->Connect();
        }));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    Assert(errors, args.nthreads);

    return 0;
}
