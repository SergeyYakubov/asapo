#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>
#include "worker/data_broker.h"
#include "testing.h"

using asapo::M_AssertEq;

void Assert(std::vector<asapo::Error>& errors, int nthreads) {
    int count_ok = (int) std::count(std::begin(errors),
                                    std::end(errors),
                                    nullptr);

    int count_already_connected = 0;
    for (auto& error : errors) {
        if (!error) continue;
        if (error->Explain().find(asapo::WorkerErrorMessage::kSourceAlreadyConnected) != std::string::npos)
            count_already_connected++;
    }

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
        exit(EXIT_FAILURE);
    }
    std::string folder{argv[1]};
    int nthreads = std::stoi(argv[2]);
    return Args{folder, nthreads};
}

int main(int argc, char* argv[]) {

    auto args = GetArgs(argc, argv);

    asapo::Error err;
    auto broker = asapo::DataBrokerFactory::CreateFolderBroker(args.folder, &err);

    std::vector<asapo::Error>errors(args.nthreads);

    std::vector<std::thread> threads;
    for (int i = 0; i < args.nthreads; i++) {
        threads.emplace_back(std::thread([&, i] {
            errors[i] = broker->Connect();
        }));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    Assert(errors, args.nthreads);

    return 0;
}
