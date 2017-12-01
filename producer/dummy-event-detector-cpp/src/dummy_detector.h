#ifndef HIDRA2_DUMMYEVENTDETECTOR__DUMMYDETECTOR_H
#define HIDRA2_DUMMYEVENTDETECTOR__DUMMYDETECTOR_H

#include <common/networking.h>
#include <producer/producer.h>

class DummyDetector
{

public:
    int main(int argc, char* argv[]);
    void handle_file_done(hidra2::FileReferenceId reference_id, hidra2::ProducerError producer_error);
};

#endif //HIDRA2_DUMMYEVENTDETECTOR__DUMMYDETECTOR_H
