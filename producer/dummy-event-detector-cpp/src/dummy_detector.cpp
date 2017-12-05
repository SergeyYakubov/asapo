#include "dummy_detector.h"
#include <iostream>

int DummyDetector::main(int argc, char **argv) {

    std::unique_ptr<hidra2::Producer> producer = hidra2::Producer::create();
    producer->connect_to_receiver("127.0.0.1");
    /*
        const size_t size = 1024 * 20;
        void *buffer = malloc(size);

        hidra2::ProducerError error;
        error = producer->send("testfile", size, buffer);

        if(error) {
            std::cerr << "File was not successfully send, ErrorCode: " << error << std::endl;
        } else {
            std::cout << "File was successfully send." << std::endl;
        }

        free(buffer);
    */
    return 0;
}
