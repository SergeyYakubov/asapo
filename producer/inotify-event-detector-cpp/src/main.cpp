#include <producer/producer.h>
#include <iostream>

int main (int argc, char* argv[]) {
    std::cout << "Running producer version: " << HIDRA2::Producer::VERSION << std::endl;

    HIDRA2::Producer* producer = HIDRA2::Producer::CreateProducer("127.0.0.1");
    if(!producer) {
        std::cerr << "Fail to create producer" << std::endl;
        return 1;
    }

    std::cout << "Successfully create producer " << std::hex << producer << std::endl;

    return 0;
}
