#include <gtest/gtest.h>
#include "producer/producer.h"

namespace
{
    TEST(VERSION, VersionAboveZero)
    {
        HIDRA2::Producer* p = HIDRA2::Producer::create();
        EXPECT_GE(p->get_version(), 0);
    }

    TEST(CreateProducer, PointerIsNotNullptr)
    {
        //HIDRA2::ProducerImpl* prod = HIDRA2::ProducerImpl::CreateProducer("127.0.0.1");
        //EXPECT_NE(1, nullptr);
    }
}
