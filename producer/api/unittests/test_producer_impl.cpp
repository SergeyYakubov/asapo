#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <netinet/in.h>
#include "../src/producer_impl.h"

namespace hidra2 {

using ::testing::Return;

class MockIO : public IO {
 public:
    MOCK_METHOD2(GetDataFromFile,
                 FileData(const std::string &fname, IOErrors* err));
    MOCK_METHOD2(deprecated_open,
                 int(
                     const char* __file,
                     int __oflag));
    MOCK_METHOD1(deprecated_close,
                 int(int
                     __fd));
    MOCK_METHOD3(deprecated_read,
                 ssize_t(int
                     __fd, void * buf, size_t
                     count));
    MOCK_METHOD3(deprecated_write,
                 ssize_t(int
                     __fd,
                     const void* __buf, size_t
                     __n));
    MOCK_METHOD3(deprecated_socket,
                 int(int
                     __domain, int
                     __type, int
                     __protocol));
    MOCK_METHOD3(deprecated_bind,
                 int(int
                     __fd, __CONST_SOCKADDR_ARG
                     __addr, socklen_t
                     __len));
    MOCK_METHOD2(deprecated_listen,
                 int(int
                     __fd, int
                     __n));
    MOCK_METHOD3(deprecated_accept,
                 int(int
                     __fd, __SOCKADDR_ARG
                     __addr, socklen_t *
                     __restrict __addr_len));
    MOCK_METHOD3(deprecated_connect,
                 int(int
                     __fd, __CONST_SOCKADDR_ARG
                     __addr, socklen_t
                     __len));
    MOCK_METHOD4(deprecated_recv,
                 ssize_t(int
                     __fd, void * __buf, size_t
                     __n, int
                     __flags));
    MOCK_METHOD4(deprecated_send,
                 ssize_t(int
                     __fd,
                     const void* __buf, size_t
                     __n, int
                     __flags));
    MOCK_METHOD5(deprecated_select,
                 int(int
                     __nfds, fd_set * __readfds, fd_set * __writefds, fd_set * __exceptfds,
                     struct timeval* __timeout));
    MOCK_METHOD2(FilesInFolder,
                 std::vector<FileInfo>(const std::string& folder, IOErrors* err));
};


TEST(get_version, VersionAboveZero) {
    hidra2::ProducerImpl producer;
    EXPECT_GE(producer.get_version(), 0);
}


TEST(connect_to_receiver, blatodo) {
    MockIO mockIO;

    hidra2::ProducerImpl producer;
    producer.__set_io(&mockIO);

    EXPECT_CALL(mockIO, socket(AF_INET, 0, IPPROTO_IP)).WillOnce(Return(-1));

    producer.connect_to_receiver("127.0.0.1");
}
}
