#include "dummy_detector.h"
#include <iostream>

int DummyDetector::main(int argc, char **argv) {

    std::unique_ptr<hidra2::Producer> producer = hidra2::Producer::create();
    producer->connect_to_receiver("127.0.0.1");
    const size_t size = 255;
    void *buffer = malloc(size);

    for(char i = 0; i < 255; i++) {
        static_cast<char*>(buffer)[i] = i;
    }

    hidra2::ProducerError error;
    error = producer->send("testfile", buffer, size);

    if(error) {
        std::cerr << "File was not successfully send, ErrorCode: " << error << std::endl;
    } else {
        std::cout << "File was successfully send." << std::endl;
    }

    free(buffer);
    /*
    */
    return 0;
}
