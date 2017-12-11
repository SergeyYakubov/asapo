#include "dummy_detector.h"
#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <cmath>
#include <unistd.h>

int DummyDetector::main(int argc, char **argv) {

    std::unique_ptr<hidra2::Producer> producer = hidra2::Producer::create();
    hidra2::ProducerError err = producer->connect_to_receiver("127.0.0.1:8099");
    if(err) {
        std::cerr << "Fail to connect to receiver. ProducerError: " << err << std::endl;
        return -1;
    }
    /*const size_t size = 255;
    void *buffer = malloc(size);

    for(unsigned char i = 0; i < 255; i++) {
        static_cast<unsigned char*>(buffer)[i] = i;
    }
     */

    int fd = open("/mnt/ramdisk/bigfile", O_RDONLY);
    struct stat astat {};
    fstat(fd, &astat);


    size_t map_size = static_cast<size_t>(ceil(float(astat.st_size)/float(getpagesize()))*getpagesize());
    void *buffer = mmap(nullptr, map_size, PROT_READ, MAP_SHARED, fd, 0);



    hidra2::ProducerError error;
    error = producer->send("testfile4", buffer, astat.st_size);

    if(error) {
        std::cerr << "File was not successfully deprecated_send, ErrorCode: " << error << std::endl;
    } else {
        std::cout << "File was successfully deprecated_send." << std::endl;
    }

    munmap(buffer, map_size);

    return 0;
}
