#include "dummy_detector.h"
#include "dummy_yieldable.h"
#include <iostream>

int DummyDetector::main(int argc, char** argv)
{

    std::unique_ptr<hidra2::Producer> producer = hidra2::Producer::create();
    producer->connectToReceiver("127.0.0.1");

    const size_t size = 1024*20;
    void* buffer = malloc(size);
    auto dummy_yieldable = new DummyYieldable(buffer, size);

    hidra2::ProducerError error;
    producer->send("test", size, dummy_yieldable,
    [&dummy_yieldable, &buffer](hidra2::FileChunk fileChunk) {
        if(dummy_yieldable->is_done()) {
            free(buffer);
            delete dummy_yieldable;
        }
    }, [this](hidra2::FileReferenceId reference_id, hidra2::ProducerError error) {
        handle_file_done(reference_id, error);
    },
    error);

    return 0;
}

void DummyDetector::handle_file_done(hidra2::FileReferenceId reference_id, hidra2::ProducerError error)
{
    if(!error) {
        std::cout << "File " << reference_id << " was successfully send." << std::endl;
        return;
    }
    std::cout << "An error occurred while sending file " << error << std::endl;
}
