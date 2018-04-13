#include <iostream>
#include <chrono>
#include <vector>
#include <tuple>

#include "hidra2_producer.h"

using std::chrono::high_resolution_clock;

typedef std::tuple<std::string, size_t, uint64_t> ArgumentTuple;
ArgumentTuple ProcessCommandArguments(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout <<
                  "Usage: " << argv[0] << " <receiver_address> <number_of_byte> <iterations>"
                  << std::endl;
        exit(EXIT_FAILURE);
    }
    try {
        return ArgumentTuple(argv[1], std::stoull(argv[2]), std::stoull(argv[3]));
    } catch(std::exception& e) {
        std::cerr << "Fail to parse arguments" << std::endl;
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}

bool SendDummyData(hidra2::Producer* producer, size_t number_of_byte, uint64_t iterations) {
    auto buffer = std::unique_ptr<uint8_t>(new uint8_t[number_of_byte]);

    for(uint64_t i = 0; i < iterations; i++) {
//        std::cerr << "Send file " << i + 1 << "/" << iterations << std::endl;

        auto err = producer->Send(i, buffer.get(), number_of_byte);

        if (err) {
            std::cerr << "File was not successfully send: " << err << std::endl;
            return false;
        } else {
//            std::cerr << "File was successfully send." << std::endl;
        }
    }

    return true;
}

int main (int argc, char* argv[]) {
    std::string receiver_address;
    size_t number_of_kbytes;
    uint64_t iterations;
    std::tie(receiver_address, number_of_kbytes, iterations) = ProcessCommandArguments(argc, argv);

    std::cout << "receiver_address: " << receiver_address << std::endl
              << "Package size: " << number_of_kbytes << "k" << std::endl
              << "iterations: " << iterations << std::endl
              << std::endl;

    auto producer = hidra2::Producer::Create();
    auto err = producer->ConnectToReceiver(receiver_address);
    if(err) {
        std::cerr << "Failed to connect to receiver. ProducerError: " << err << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Successfully connected" << std::endl;

    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    if(!SendDummyData(producer.get(), number_of_kbytes * 1024, iterations)) {
        return EXIT_FAILURE;
    }
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    double duration_sec = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count() / 1000.0;
    double size_gb = double(number_of_kbytes) * iterations / 1024.0 / 1024.0 * 8.0;
    double rate = iterations / duration_sec;
    std::cout << "Rate: " << rate << " Hz" << std::endl;
    std::cout << "Bandwidth " << size_gb / duration_sec << " Gbit/s" << std::endl;

    return EXIT_SUCCESS;
}

