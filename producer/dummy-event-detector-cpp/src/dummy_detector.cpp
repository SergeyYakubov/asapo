#include "dummy_detector.h"
#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <cmath>
#include <unistd.h>

int DummyDetector::main(int argc, char** argv) {
    auto producer = hidra2::Producer::create();

    hidra2::ProducerError err = producer->ConnectToReceiver("127.0.0.1:8099");
    if(err) {
        std::cerr << "Fail to connect to receiver. ProducerError: " << err << std::endl;
        return 1;
    }

    /*
    const size_t size = 255;
    void *buffer = malloc(size);

    for(unsigned char i = 0; i < 255; i++) {
        static_cast<unsigned char*>(buffer)[i] = i;
    }
     */

    hidra2::FileOpenMode open_flags = hidra2::OPEN_MODE_RW;
    hidra2::IOError io_err;
    int fd = io->Open("/tmp/bigfile", open_flags, &io_err);
    if(io_err != hidra2::IOError::NO_ERROR) {
        std::cerr << "Fail to open file" << std::endl;
        return 1;
    }
    struct stat astat {};
    fstat(fd, &astat);

    size_t map_size = static_cast<size_t>(ceil(float(astat.st_size) / float(getpagesize())) * getpagesize());
    void* buffer = mmap(nullptr, map_size, PROT_READ, MAP_SHARED, fd, 0);

    madvise(buffer, map_size, MADV_SEQUENTIAL | MADV_WILLNEED);

    const size_t size = astat.st_size;

    hidra2::ProducerError error;
    error = producer->Send("testfile4", buffer, size);

    if(error) {
        std::cerr << "File was not successfully deprecated_send, ErrorCode: " << error << std::endl;
    } else {
        std::cout << "File was successfully send." << std::endl;
    }

    //munmap(buffer, map_size);

    return 0;
}
