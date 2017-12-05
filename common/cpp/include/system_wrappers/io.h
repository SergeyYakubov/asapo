#ifndef HIDRA2_SYSTEM_WRAPPERS__IO_H
#define HIDRA2_SYSTEM_WRAPPERS__IO_H

#include <string>
#include <vector>


namespace hidra2 {

enum class IOErrors {
    NO_ERROR,
    FOLDER_NOT_FOUND,
    PERMISSIONS_DENIED,
    UNKWOWN_ERROR
};

class IO {
  public:
    virtual int open(const char *__file, int __oflag) = 0;
    virtual int close(int __fd) = 0;
    virtual ssize_t read(int __fd, void *buf, size_t count) = 0;
    virtual ssize_t write(int __fd, const void *__buf, size_t __n) = 0;

// this is not standard function - to be implemented differently in windows and linux
    virtual std::vector<std::string> FilesInFolder(std::string folder,IOErrors* err) = 0;
};

}

#endif //HIDRA2_SYSTEM_WRAPPERS__IO_H
