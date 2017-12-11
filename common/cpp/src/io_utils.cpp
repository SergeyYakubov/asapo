#include <fcntl.h>
#include <cerrno>

#include "system_wrappers/io_utils.h"


hidra2::IOUtils::IOUtils(hidra2::IO** io) {
    io_ = io;
}

bool hidra2::IOUtils::is_valid_fd(int fd) {
    return fcntl(fd, F_GETFD) != -1 || errno != EBADF;
}
