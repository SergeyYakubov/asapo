#include <gtest/gtest.h>
#include "producer/producer.h"

namespace
{

    TEST(create, PointerIsNotNullptr)
    {
        std::unique_ptr<hidra2::Producer> p = hidra2::Producer::create();
        EXPECT_NE(p, nullptr);
    }

    TEST(get_version, VersionAboveZero)
    {
        std::unique_ptr<hidra2::Producer> p = hidra2::Producer::create();
        EXPECT_GE(p->get_version(), 0);
    }

    TEST(get_status, StatusDisconnectedBeforeConnect)
    {
        std::unique_ptr<hidra2::Producer> p = hidra2::Producer::create();
        EXPECT_GE(p->get_status(), hidra2::ProducerStatus::PRODUCER_STATUS__DISCONNECTED);
    }
}
