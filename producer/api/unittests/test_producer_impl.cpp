#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../src/producer_impl.h"

namespace hidra2 {

class MockProducer : public Producer {
  public:
    MOCK_CONST_METHOD0(get_version,
                       uint64_t());
    MOCK_CONST_METHOD0(get_status,
                       ProducerStatus());
    MOCK_METHOD1(connect_to_receiver,
                 ProducerError(const std::string& receiver_address));
    MOCK_METHOD3(send,
                 ProducerError(std::string filename, void* data, uint64_t file_size));
};


TEST(get_version, VersionAboveZero) {
    hidra2::ProducerImpl producer;
    EXPECT_GE(producer.get_version(), 0);
}


TEST(connect_to_receivera, blatodo) {

    MockProducer mockIO;
    hidra2::ProducerImpl producer;

    //producer.__set_io(&mockIO);

    //EXPECT_GE(producer.connect_to_receiver, 0);
}

}
