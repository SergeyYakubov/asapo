#include <fcntl.h>
#include <unistd.h>

#include <system_wrappers/system_io.h>

namespace hidra2 {

void ReadWholeFile(int fd, uint8_t* array, uint64_t fsize, IOErrors* err) {
    ssize_t totalbytes = 0;
    ssize_t readbytes = 0;
    do {
        readbytes = read(fd, array + totalbytes, fsize);
        totalbytes += readbytes;
    } while (readbytes > 0 && totalbytes < fsize);

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

    FileData data(fsize);

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