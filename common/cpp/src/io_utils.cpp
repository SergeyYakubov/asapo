#include <fcntl.h>
#include <cerrno>

#include "system_wrappers/io_utils.h"


hidra2::IOUtils::IOUtils(hidra2::IO** io) {
    io_ = io;
}

bool hidra2::IOUtils::is_valid_fd(int fd) {
    return fcntl(fd, F_GETFD) != -1 || errno != EBADF;
}

ssize_t hidra2::IOUtils::send_in_steps(int __fd, const void* __buf, size_t __n, int __flags) {
    size_t already_send = 0;
    while(already_send < __n) {
        ssize_t send_amount = (*io_)->send(__fd, __buf + already_send, __n - already_send, __flags);
        if(send_amount <= 0) {
            return send_amount;
        }
        already_send += send_amount;
    }
    return already_send;
}

ssize_t hidra2::IOUtils::recv_in_steps(int __fd, void* __buf, size_t __n, int __flags) {
    size_t already_received = 0;

    while(already_received < __n) {
        ssize_t recv_amount = (*io_)->recv(__fd, __buf + already_received, __n - already_received, __flags);
        if(recv_amount <= 0) {
            return recv_amount;
        }
        already_received += recv_amount;
    }

    return already_received;
}
