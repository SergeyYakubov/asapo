#include <fcntl.h>
#include <unistd.h>
#include <chrono>
#include <iostream>

#include <system_wrappers/system_io.h>

namespace hidra2 {

void ReadWholeFile(int fd, uint8_t* array, uint64_t fsize, IOErrors* err) {

    auto t1 = std::chrono::high_resolution_clock::now();

    ssize_t totalbytes = 0;
    ssize_t readbytes = 0;
    do {
        readbytes = read(fd, array + totalbytes, fsize);
        totalbytes += readbytes;
    } while (readbytes > 0 && totalbytes < fsize);

    auto t2 = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();
    std::cout << "Elapsed ReadWholeFile : " << duration << "ms" << std::endl;


    if (totalbytes != fsize) {
        *err = IOErrors::READ_ERROR;
    }
}

FileData SystemIO::GetDataFromFile(const std::string& fname, uint64_t fsize, IOErrors* err) {
    int fd = open(fname.c_str(), O_RDONLY);
    *err = IOErrorFromErrno();
    if (*err != IOErrors::NO_ERROR) {
        return {};
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    FileData data(fsize);

    auto t2 = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    std::cout << "Elapsed CreateVector : " << duration << "ms" << std::endl;



    ReadWholeFile(fd, &data[0], fsize, err);
    if (*err != IOErrors::NO_ERROR) {
        close(fd);
        return {};
    }

    close(fd);
    *err = IOErrorFromErrno();
    if (*err != IOErrors::NO_ERROR) {
        return {};
    }

    return data;
}


int SystemIO::open(const char* __file, int __oflag) {
    return ::open(__file, __oflag);
}

int SystemIO::close(int __fd) {
    return ::close(__fd);
}

ssize_t SystemIO::read(int __fd, void* buf, size_t count) {
    return ::read(__fd, buf, count);
}

ssize_t SystemIO::write(int __fd, const void* __buf, size_t __n) {
    return ::write(__fd, __buf, __n);
}

}