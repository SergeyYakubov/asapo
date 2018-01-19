#include <fcntl.h>
#include <system_wrappers/system_io.h>
#include <assert.h>

namespace hidra2 {

/*******************************************************************************
 *                              system_io.cpp                                  *
 * THIS FILE HOLDS GENERAL FUNCTIONS THAT CAN BE USED ON WINDOWS AND ON LINUX  *
 *******************************************************************************/

std::thread* hidra2::SystemIO::NewThread(std::function<void()> function) {
    return new std::thread(function);
}

void hidra2::SystemIO::Skip(hidra2::FileDescriptor socket_fd, size_t length, hidra2::IOErrors* err) {
    static const size_t kSkipBufferSize = 1024;

    //TODO need to find a better way to skip bytes
    *err = IOErrors::kNoError;
    std::unique_ptr<uint8_t[]> buffer;
    try {
        buffer.reset(new uint8_t[kSkipBufferSize]);
    } catch(std::exception& e) {
        assert(false);
        *err = IOErrors::kUnknownError;
        return;
    }
    size_t already_skipped = 0;
    while(already_skipped < length) {
        size_t need_to_skip = length - already_skipped;
        if(need_to_skip > kSkipBufferSize)
            need_to_skip = kSkipBufferSize;
        size_t skipped_amount = Receive(socket_fd, buffer.get(), need_to_skip, err);
        if(*err != IOErrors::kNoError) {
            return;
        }
        already_skipped += skipped_amount;
    }
}

hidra2::FileDescriptor hidra2::SystemIO::CreateAndConnectIPTCPSocket(const std::string& address,
        hidra2::IOErrors* err) {
    *err = hidra2::IOErrors::kNoError;

    FileDescriptor fd = CreateSocket(AddressFamilies::INET, SocketTypes::STREAM, SocketProtocols::IP, err);
    if(*err != IOErrors::kNoError) {
        return -1;
    }
    InetConnect(fd, address, err);
    if(*err != IOErrors::kNoError) {
        Close(fd, nullptr);
        return -1;
    }

    return fd;
}

}
