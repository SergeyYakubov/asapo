#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>

#include "../src/data_cache.h"

using ::testing::Test;
using ::testing::Gt;
using ::testing::Ge;
using ::testing::Le;
using ::testing::Eq;
using ::testing::Ne;
using ::testing::Ref;
using ::testing::_;
using ::testing::ElementsAreArray;

using asapo::DataCache;


namespace {


class DataCacheTests : public Test {
  public:
    uint64_t expected_cache_size = 1024 * 1024;
    uint64_t expected_size = 10;
    uint64_t expected_val = 1;
    float expected_keepunlocked_ratio = 0.2;
    DataCache cache{expected_cache_size, expected_keepunlocked_ratio};
    void SetUp() override {
    }
    void TearDown() override {
    }
};

TEST_F(DataCacheTests, GetFreeSlotFailsDueToSize) {
    uint64_t id;
    auto addr = cache.GetFreeSlot(expected_cache_size + 1, &id);
    ASSERT_THAT(addr, Eq(nullptr));
}

void set_array(uint8_t* addr, uint64_t size, uint8_t val) {
    for (uint64_t i = 0; i < size; i++) {
        addr[i] = val;
    }
}

TEST_F(DataCacheTests, GetFreeSlotOK) {
    uint64_t id;
    uint8_t* addr = (uint8_t*) cache.GetFreeSlot(1, &id);
    set_array(addr, 1, 2);
    ASSERT_THAT(addr[0], Eq(2));
    ASSERT_THAT(id, Gt(0));
}

TEST_F(DataCacheTests, GetFreeSlotStartsFromLastPointer) {
    uint64_t id1, id2;
    uint8_t* ini_addr = (uint8_t*) cache.GetFreeSlot(1, &id1);
    set_array(ini_addr, 1, 2);
    uint8_t* addr = (uint8_t*) cache.GetFreeSlot(1, &id2);
    set_array(addr, 1, 1);
    ASSERT_THAT(ini_addr[0], Eq(2));
    ASSERT_THAT(ini_addr[1], Eq(1));
    ASSERT_THAT(id1, Ne(id2));
}


TEST_F(DataCacheTests, GetFreeSlotStartsFromBeginIfNotFit) {
    uint64_t id1, id2;
    uint8_t* ini_addr = (uint8_t*) cache.GetFreeSlot(1, &id1);
    set_array(ini_addr, 1, 2);
    uint8_t* addr = (uint8_t*) cache.GetFreeSlot(expected_cache_size, &id2);
    set_array(addr, expected_cache_size, 1);
    ASSERT_THAT(ini_addr[0], Eq(1));
    ASSERT_THAT(id1, Ne(id2));
}


TEST_F(DataCacheTests, PrepareToReadIdNotFound) {
    uint64_t size, id;
    id = 0;
    uint8_t* addr = (uint8_t*) cache.GetSlotToReadAndLock(id, &size);
    ASSERT_THAT(addr, Eq(nullptr));
}

TEST_F(DataCacheTests, PrepareToReadOk) {
    uint64_t size, id;
    uint64_t data_size = expected_cache_size * 0.7;
    uint8_t* ini_addr = (uint8_t*) cache.GetFreeSlot(data_size, &id);
    set_array(ini_addr, data_size, expected_val);

    uint8_t* addr = (uint8_t*) cache.GetSlotToReadAndLock(id, &size);
    ASSERT_THAT(addr, Eq(ini_addr));
    ASSERT_THAT(size, Eq(data_size));

}

TEST_F(DataCacheTests, PrepareToReadFailsIfTooCloseToCurrentPointer) {
    uint64_t size, id;
    auto data_size = expected_cache_size * 0.9;
    cache.GetFreeSlot(data_size, &id);

    uint8_t* addr = (uint8_t*) cache.GetSlotToReadAndLock(id, &size);
    ASSERT_THAT(addr, Eq(nullptr));

}

TEST_F(DataCacheTests, GetFreeSlotRemovesOldMetadataRecords) {
    uint64_t id1, id2, id3, id4, id5, size;
    cache.GetFreeSlot(10, &id1);
    cache.GetFreeSlot(10, &id2);
    cache.GetFreeSlot(expected_cache_size - 30, &id3);
    cache.GetFreeSlot(10, &id4);

    cache.GetFreeSlot(30, &id5);
    uint8_t* addr1 = (uint8_t*) cache.GetSlotToReadAndLock(id1, &size);
    uint8_t* addr2 = (uint8_t*) cache.GetSlotToReadAndLock(id2, &size);
    uint8_t* addr3 = (uint8_t*) cache.GetSlotToReadAndLock(id3, &size);
    uint8_t* addr4 = (uint8_t*) cache.GetSlotToReadAndLock(id4, &size);

    ASSERT_THAT(addr1, Eq(nullptr));
    ASSERT_THAT(addr2, Eq(nullptr));
    ASSERT_THAT(addr3, Eq(nullptr));
    ASSERT_THAT(addr4, Ne(nullptr));
    ASSERT_THAT(size, Eq(10));
}

TEST_F(DataCacheTests, CannotGetFreeSlotIfNeedCleanOnebeingReaded) {
    uint64_t id1, id2, size;

    uint8_t* ini_addr = (uint8_t*) cache.GetFreeSlot(10, &id1);


    auto res = cache.GetSlotToReadAndLock(id1, &size);


    uint8_t* addr = (uint8_t*) cache.GetFreeSlot(expected_cache_size, &id2);

    ASSERT_THAT(res, Ne(nullptr));
    ASSERT_THAT(ini_addr, Ne(nullptr));
    ASSERT_THAT(addr, Eq(nullptr));
}


TEST_F(DataCacheTests, GetFreeSlotCheckIds) {
    uint64_t id1, id2, id3, id4;
    cache.GetFreeSlot(10, &id1);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    cache.GetFreeSlot(10, &id2);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    cache.GetFreeSlot(10, &id3);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    cache.GetFreeSlot(10, &id4);

    auto c1 = static_cast<uint32_t>(id1);
    auto c2 = static_cast<uint32_t>(id2);
    auto c3 = static_cast<uint32_t>(id3);
    auto c4 = static_cast<uint32_t>(id4);

    auto t1 = id1 >> 32;
    auto t2 = id2 >> 32;
    auto t3 = id3 >> 32;
    auto t4 = id4 >> 32;

    ASSERT_THAT(c2, Eq(c1 + 1));
    ASSERT_THAT(c3, Eq(c2 + 1));
    ASSERT_THAT(c4, Eq(c3 + 1));

    ASSERT_THAT(t2 - t1, Ge(100));
    ASSERT_THAT(t2 - t1, Le(200));

    ASSERT_THAT(t3 - t2, Ge(10));
    ASSERT_THAT(t3 - t2, Le(20));

    ASSERT_THAT(t4 - t3, Ge(1));
}



}
