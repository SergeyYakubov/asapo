
#include <producer/producer.h>
#include <iostream>
#include <tuple>

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

int SendDummyData(const std::string& receiver_address, size_t number_of_byte, uint64_t iterations) {
    auto producer = hidra2::Producer::create();

    hidra2::ProducerError err = producer->ConnectToReceiver(receiver_address);
    if(err != hidra2::ProducerError::kNoError) {
        std::cerr << "Fail to connect to receiver. ProducerError: " /*<< err*/ << std::endl;//TODO
        return 1;
    }
    std::cout << "Successfully connected" << std::endl;

    auto buffer = std::unique_ptr<uint8_t>(new uint8_t[number_of_byte]);

    for(uint64_t i = 0; i < iterations; i++) {
        std::cout << "Send file " << i + 1 << "/" << iterations << std::endl;
        hidra2::ProducerError error;
        error = producer->Send(i, buffer.get(), number_of_byte);

        if (error != hidra2::ProducerError::kNoError) {
            std::cerr << "File was not successfully send, ErrorCode: " /*<< error*/ << std::endl;
            break;
        } else {
            std::cout << "File was successfully send." << std::endl;
        }
    }

    return 0;
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

    SendDummyData(receiver_address, number_of_byte, iterations);
    getchar();

}

