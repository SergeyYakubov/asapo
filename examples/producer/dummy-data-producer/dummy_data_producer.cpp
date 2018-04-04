
#include <producer/producer.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <tuple>

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
        std::cout << "Send file " << i + 1 << "/" << iterations << std::endl;

        auto err = producer->Send(i, buffer.get(), number_of_byte);

        if (err) {
            std::cerr << "File was not successfully send: " << err << std::endl;
            return false;
        } else {
            std::cout << "File was successfully send." << std::endl;
        }
    }

    return true;
}

int main (int argc, char* argv[]) {
    std::string receiver_address;
    size_t number_of_byte;
    uint64_t iterations;
    std::tie(receiver_address, number_of_byte, iterations) = ProcessCommandArguments(argc, argv);

    std::cout << "receiver_address: " << receiver_address << std::endl
              << "number_of_byte: " << number_of_byte << std::endl
              << "iterations: " << iterations << std::endl
              << std::endl;

    auto producer = hidra2::Producer::Create();
    auto err = producer->ConnectToReceiver(receiver_address);
    if(err) {
        std::cerr << "Failed to connect to receiver. ProducerError: " << err << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Successfully connected" << std::endl;

    if(!SendDummyData(producer.get(), number_of_byte, iterations)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

