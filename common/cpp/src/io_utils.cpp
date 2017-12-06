#include <fcntl.h>
#include <cerrno>

#include "system_wrappers/io_utils.h"

bool hidra2::IOUtils::is_valid_fd(int fd) {
    return fcntl(fd, F_GETFD) != -1 || errno != EBADF;
}
