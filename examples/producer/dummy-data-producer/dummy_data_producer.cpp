#include <iostream>
#include <chrono>
#include <vector>
#include <tuple>
#include <mutex>
#include <thread>

#include "asapo_producer.h"


using std::chrono::high_resolution_clock;

std::mutex mutex;
int nfiles;

typedef std::tuple<std::string, size_t, uint64_t, uint64_t,uint64_t> ArgumentTuple;
ArgumentTuple ProcessCommandArguments(int argc, char* argv[]) {
    if (argc != 6) {
        std::cout <<
                  "Usage: " << argv[0] << " <destination> <number_of_byte> <iterations> <nthreads> <mode 0 -t tcp, 1 - filesystem>"
                  << std::endl;
        exit(EXIT_FAILURE);
    }
    try {
        return ArgumentTuple(argv[1], std::stoull(argv[2]), std::stoull(argv[3]), std::stoull(argv[4]),std::stoull(argv[5]));
    } catch(std::exception& e) {
        std::cerr << "Fail to parse arguments" << std::endl;
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}

void work(asapo::GenericRequestHeader header, asapo::Error err) {
    mutex.lock();
    nfiles--;
    if (err) {
        std::cerr << "File was not successfully send: " << err << std::endl;
        nfiles = 0;
        mutex.unlock();
        return;
    }
   // std::cerr << "File was successfully send." << header.data_id << std::endl;
    mutex.unlock();
}

bool SendDummyData(asapo::Producer* producer, size_t number_of_byte, uint64_t iterations) {
    auto buffer = std::unique_ptr<uint8_t>(new uint8_t[number_of_byte]);

    for(uint64_t i = 0; i < iterations; i++) {
//        std::cerr << "Send file " << i + 1 << "/" << iterations << std::endl;
        auto err = producer->Send(i + 1, buffer.get(), number_of_byte,std::to_string(i), &work);
        if (err) {
            std::cerr << "Cannot send file: " << err << std::endl;
            return false;
        }
    }

    return true;
}

int main (int argc, char* argv[]) {
    std::string receiver_address;
    size_t number_of_kbytes;
    uint64_t iterations;
    uint64_t nthreads;
    uint64_t mode;

    std::tie(receiver_address, number_of_kbytes, iterations, nthreads,mode) = ProcessCommandArguments(argc, argv);

    std::cout << "receiver_address: " << receiver_address << std::endl
              << "Package size: " << number_of_kbytes << "k" << std::endl
              << "iterations: " << iterations << std::endl
              << "nthreads: " << nthreads << std::endl
              << "mode: " << mode << std::endl
        << std::endl;

    nfiles = iterations;

    asapo::Error err;
    std::unique_ptr<asapo::Producer> producer;
    if (mode == 0) {
        producer = asapo::Producer::Create(receiver_address, nthreads, asapo::RequestHandlerType::kTcp,&err);
    } else {
        producer = asapo::Producer::Create(receiver_address, nthreads, asapo::RequestHandlerType::kFilesystem,&err);
    }
    producer->EnableLocalLog(true);
    producer->SetLogLevel(asapo::LogLevel::Debug);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    if(err) {
        std::cerr << "Cannot start producer. ProducerError: " << err << std::endl;
        return EXIT_FAILURE;
    }

    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    if(!SendDummyData(producer.get(), number_of_kbytes * 1024, iterations)) {
        return EXIT_FAILURE;
    }

    while (true) {
        mutex.lock();
        if (nfiles <= 0) {
            mutex.unlock();
            break;
        }
        mutex.unlock();
    }

    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    double duration_sec = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count() / 1000.0;
    double size_gb = double(number_of_kbytes) * iterations / 1024.0 / 1024.0 * 8.0;
    double rate = iterations / duration_sec;
    std::cout << "Rate: " << rate << " Hz" << std::endl;
    std::cout << "Bandwidth " << size_gb / duration_sec << " Gbit/s" << std::endl;

    return EXIT_SUCCESS;
}

