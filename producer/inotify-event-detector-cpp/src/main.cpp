#include <producer/producer.h>
#include <iostream>

int main (int argc, char* argv[]) {
    std::cout << "Running producer version: " << hidra2::Producer::VERSION << std::endl;

    hidra2::Producer* producer = hidra2::Producer::CreateProducer("127.0.0.1");
    if(!producer) {
        std::cerr << "Fail to create producer" << std::endl;
        return 1;
    }

    std::cout << "Successfully create producer " << std::hex << producer << std::endl;

    return 0;
}
