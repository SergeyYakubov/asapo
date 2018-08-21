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
    std::string beamtime_id;
    size_t number_of_bytes;
    uint64_t iterations;
    uint64_t nthreads;
    uint64_t mode;
    uint64_t timeout_sec;
};

void PrintCommandArguments(const Args& args) {
    std::cout << "receiver_address: " << args.receiver_address << std::endl
              << "beamtime_id: " << args.beamtime_id << std::endl
              << "Package size: " << args.number_of_bytes / 1000 << "k" << std::endl
              << "iterations: " << args.iterations << std::endl
              << "nthreads: " << args.nthreads << std::endl
              << "mode: " << args.mode << std::endl
              << "timeout: " << args.timeout_sec << std::endl
              << std::endl;
}


void ProcessCommandArguments(int argc, char* argv[], Args* args) {
    if (argc != 8) {
        std::cout <<
                  "Usage: " << argv[0] <<
                  " <destination> <beamtime_id> <number_of_byte> <iterations> <nthreads>"
                  " <mode 0 -t tcp, 1 - filesystem> <timeout (sec)>"
                  << std::endl;
        exit(EXIT_FAILURE);
    }
    try {
        args->receiver_address = argv[1];
        args->beamtime_id = argv[2];
        args->number_of_bytes = std::stoull(argv[3]) * 1000;
        args->iterations = std::stoull(argv[4]);
        args->nthreads = std::stoull(argv[5]);
        args->mode = std::stoull(argv[6]);
        args->timeout_sec = std::stoull(argv[7]);
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

asapo::FileData CreateMemoryBuffer(size_t size) {
    return asapo::FileData(new uint8_t[size]);
}


bool SendDummyData(asapo::Producer* producer, size_t number_of_byte, uint64_t iterations) {

    for(uint64_t i = 0; i < iterations; i++) {
        auto buffer = CreateMemoryBuffer(number_of_byte);
        asapo::EventHeader event_header{i + 1, number_of_byte, std::to_string(i)};
        auto err = producer->Send(event_header, std::move(buffer), &ProcessAfterSend);
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
                                            args.mode == 0 ? asapo::RequestHandlerType::kTcp : asapo::RequestHandlerType::kFilesystem,
                                            args.beamtime_id, &err);
    if(err) {
        std::cerr << "Cannot start producer. ProducerError: " << err << std::endl;
        exit(EXIT_FAILURE);
    }

    producer->EnableLocalLog(true);
    producer->SetLogLevel(asapo::LogLevel::Info);
    return producer;
}

void WaitThreadsFinished(const Args& args) {
    uint64_t elapsed_ms = 0;
    while (true) {
        mutex.lock();
        if (iterations_remained <= 0) {
            mutex.unlock();
            break;
        }
        mutex.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        elapsed_ms += 100;
        if (elapsed_ms > args.timeout_sec * 1000) {
            std::cerr << "Exit on timeout " << std::endl;
            exit(EXIT_FAILURE);
        }
    }

}

void PrintOutput(const Args& args, const high_resolution_clock::time_point& start) {
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    double duration_sec = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - start ).count() / 1000.0;
    double size_gb = double(args.number_of_bytes) * args.iterations / 1000.0  / 1000.0 / 1000.0 * 8.0;
    double rate = args.iterations / duration_sec;
    std::cout << "Rate: " << rate << " Hz" << std::endl;
    std::cout << "Bandwidth " << size_gb / duration_sec << " Gbit/s" << std::endl;
}



int main (int argc, char* argv[]) {
    Args args;
    ProcessCommandArguments(argc, argv, &args);

    auto producer = CreateProducer(args);

    iterations_remained = args.iterations;

    high_resolution_clock::time_point start_time = high_resolution_clock::now();

    if(!SendDummyData(producer.get(), args.number_of_bytes, args.iterations)) {
        return EXIT_FAILURE;
    }

    WaitThreadsFinished(args);
    PrintOutput(args, start_time);

    return EXIT_SUCCESS;
}

