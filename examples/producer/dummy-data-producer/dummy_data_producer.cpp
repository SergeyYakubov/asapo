#include <iostream>
#include <chrono>
#include <vector>
#include <mutex>
#include <thread>

#include "asapo_producer.h"


using std::chrono::high_resolution_clock;

std::mutex mutex;
int iterations_remained;

struct Args {
    std::string receiver_address;
    size_t number_of_bytes;
    uint64_t iterations;
    uint64_t nthreads;
    uint64_t mode;
};

void PrintCommandArguments(const Args& args) {
    std::cout << "receiver_address: " << args.receiver_address << std::endl
              << "Package size: " << args.number_of_bytes / 1024 << "k" << std::endl
              << "iterations: " << args.iterations << std::endl
              << "nthreads: " << args.nthreads << std::endl
              << "mode: " << args.mode << std::endl
              << std::endl;
}


void ProcessCommandArguments(int argc, char* argv[], Args* args) {
    if (argc != 6) {
        std::cout <<
                  "Usage: " << argv[0] << " <destination> <number_of_byte> <iterations> <nthreads> <mode 0 -t tcp, 1 - filesystem>"
                  << std::endl;
        exit(EXIT_FAILURE);
    }
    try {
        args->receiver_address = argv[1];
        args->number_of_bytes = std::stoull(argv[2]) * 1024;
        args->iterations = std::stoull(argv[3]);
        args->nthreads = std::stoull(argv[4]);
        args->mode = std::stoull(argv[5]);
        PrintCommandArguments(*args);
        return;
    } catch(std::exception& e) {
        std::cerr << "Fail to parse arguments" << std::endl;
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}

void ProcessAfterSend(asapo::GenericRequestHeader header, asapo::Error err) {
    mutex.lock();
    iterations_remained--;
    if (err) {
        std::cerr << "File was not successfully send: " << err << std::endl;
        mutex.unlock();
        return;
    }
    mutex.unlock();
}

bool SendDummyData(asapo::Producer* producer, uint8_t* data, size_t number_of_byte, uint64_t iterations) {

    for(uint64_t i = 0; i < iterations; i++) {
        auto err = producer->Send(i + 1, data, number_of_byte, std::to_string(i), &ProcessAfterSend);
        if (err) {
            std::cerr << "Cannot send file: " << err << std::endl;
            return false;
        }
    }
    return true;
}

std::unique_ptr<asapo::Producer> CreateProducer(const Args& args) {
    asapo::Error err;
    auto producer = asapo::Producer::Create(args.receiver_address, args.nthreads,
                                            args.mode == 0 ? asapo::RequestHandlerType::kTcp : asapo::RequestHandlerType::kFilesystem, &err);
    if(err) {
        std::cerr << "Cannot start producer. ProducerError: " << err << std::endl;
        exit(EXIT_FAILURE);
    }

    producer->EnableLocalLog(true);
    producer->SetLogLevel(asapo::LogLevel::Debug);
    return producer;
}

void WaitThreadsFinished(const Args& args) {
    uint64_t elapsed_ms = 0;
    uint64_t timeout_sec = 30;
    while (true) {
        mutex.lock();
        if (iterations_remained <= 0) {
            mutex.unlock();
            break;
        }
        mutex.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        elapsed_ms += 100;
        if (elapsed_ms > timeout_sec * 1000) {
            std::cerr << "Exit on timeout " << std::endl;
            exit(EXIT_FAILURE);
        }
    }

}

void PrintOutput(const Args& args, const high_resolution_clock::time_point& start) {
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    double duration_sec = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - start ).count() / 1000.0;
    double size_gb = double(args.number_of_bytes) * args.iterations / 1024.0  / 1024.0 / 1024.0 * 8.0;
    double rate = args.iterations / duration_sec;
    std::cout << "Rate: " << rate << " Hz" << std::endl;
    std::cout << "Bandwidth " << size_gb / duration_sec << " Gbit/s" << std::endl;
}


std::unique_ptr<uint8_t> CreateMemoryBuffer(const Args& args) {
    return std::unique_ptr<uint8_t>(new uint8_t[args.number_of_bytes]);
}

int main (int argc, char* argv[]) {
    Args args;
    ProcessCommandArguments(argc, argv, &args);

    auto producer = CreateProducer(args);

    iterations_remained = args.iterations;

    auto buffer = CreateMemoryBuffer(args);

    high_resolution_clock::time_point start_time = high_resolution_clock::now();

    if(!SendDummyData(producer.get(), buffer.get(), args.number_of_bytes, args.iterations)) {
        return EXIT_FAILURE;
    }

    WaitThreadsFinished(args);
    PrintOutput(args, start_time);

    return EXIT_SUCCESS;
}

