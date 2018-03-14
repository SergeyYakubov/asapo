#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <unittests/MockIO.h>
#include "../src/network_producer_peer.h"

using ::testing::Ne;

namespace {

TEST(CreateNetworkProducerPeer, PointerIsNotNullptr) {
    std::unique_ptr<hidra2::NetworkProducerPeer> producer = hidra2::NetworkProducerPeer::CreateNetworkProducerPeer(1, "");
    ASSERT_THAT(producer.get(), Ne(nullptr));
}

}
