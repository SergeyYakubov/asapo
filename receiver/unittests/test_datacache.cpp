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
using asapo::CacheMeta;


namespace {


class DataCacheTests : public Test {
  public:
    uint64_t expected_cache_size = 1024 * 1024;
    uint64_t expected_size = 10;
    uint64_t expected_val = 1;
    float expected_keepunlocked_ratio = 0.2f;
    CacheMeta* meta1;
    CacheMeta* meta2;
    DataCache cache{expected_cache_size, expected_keepunlocked_ratio};
    void SetUp() override {
    }
    void TearDown() override {
    }
};

TEST_F(DataCacheTests, GetFreeSlotFailsDueToSize) {
    auto addr = cache.GetFreeSlotAndLock(expected_cache_size + 1, &meta1);
    ASSERT_THAT(addr, Eq(nullptr));
}

void set_array(uint8_t* addr, uint64_t size, uint8_t val) {
    for (uint64_t i = 0; i < size; i++) {
        addr[i] = val;
    }
}

TEST_F(DataCacheTests, GetFreeSlotOK) {
    uint8_t* addr = (uint8_t*) cache.GetFreeSlotAndLock(1, &meta1);
    set_array(addr, 1, 2);
    ASSERT_THAT(addr[0], Eq(2));
    ASSERT_THAT(meta1->id, Gt(0));
}

TEST_F(DataCacheTests, GetFreeSlotStartsFromLastPointer) {
    uint8_t* ini_addr = (uint8_t*) cache.GetFreeSlotAndLock(1, &meta1);
    set_array(ini_addr, 1, 2);
    uint8_t* addr = (uint8_t*) cache.GetFreeSlotAndLock(1, &meta2);
    set_array(addr, 1, 1);
    ASSERT_THAT(ini_addr[0], Eq(2));
    ASSERT_THAT(ini_addr[1], Eq(1));
    ASSERT_THAT(meta1->id, Ne(meta2->id));
}

TEST_F(DataCacheTests, GetFreeSlotLocks) {
    uint8_t* ini_addr = (uint8_t*) cache.GetFreeSlotAndLock(1, &meta1);
    uint8_t* addr = (uint8_t*) cache.GetFreeSlotAndLock(expected_cache_size, &meta2);
    ASSERT_THAT(ini_addr, Ne(nullptr));
    ASSERT_THAT(addr, Eq(nullptr));
    ASSERT_THAT(meta1, Ne(nullptr));
    ASSERT_THAT(meta2, Eq(nullptr));
}


TEST_F(DataCacheTests, GetFreeSlotStartsFromBeginIfNotFit) {
    uint8_t* ini_addr = (uint8_t*) cache.GetFreeSlotAndLock(1, &meta1);
    auto id = meta1->id;
    set_array(ini_addr, 1, 2);
    cache.UnlockSlot(meta1);
    uint8_t* addr = (uint8_t*) cache.GetFreeSlotAndLock(expected_cache_size, &meta2);
    set_array(addr, expected_cache_size, 1);
    ASSERT_THAT(ini_addr[0], Eq(1));
    ASSERT_THAT(id, Ne(meta2->id));
}

TEST_F(DataCacheTests, GetFreeSlotCannotWriteIfAlreadyWriting) {
    cache.GetFreeSlotAndLock(1, &meta1);
    uint8_t* addr = (uint8_t*) cache.GetFreeSlotAndLock(expected_cache_size, &meta2);
    ASSERT_THAT(addr, Eq(nullptr));
    ASSERT_THAT(meta2, Eq(nullptr));

}

TEST_F(DataCacheTests, PrepareToReadIdNotFound) {
    uint64_t id;
    id = 0;
    uint8_t* addr = (uint8_t*) cache.GetSlotToReadAndLock(id, expected_size, &meta1);
    ASSERT_THAT(addr, Eq(nullptr));
    ASSERT_THAT(meta1, Eq(nullptr));
}

TEST_F(DataCacheTests, PrepareToReadOk) {
    auto data_size = static_cast<uint64_t>(expected_cache_size * 0.7);
    uint8_t* ini_addr = (uint8_t*) cache.GetFreeSlotAndLock(data_size, &meta1);

    uint8_t* addr = (uint8_t*) cache.GetSlotToReadAndLock(meta1->id, data_size, &meta2);
    ASSERT_THAT(addr, Eq(ini_addr));
    ASSERT_THAT(meta1, Eq(meta2));
    ASSERT_THAT(meta2->size, Eq(data_size));
}


TEST_F(DataCacheTests, PrepareToReadFailsIfTooCloseToCurrentPointer) {
    auto data_size = static_cast<uint64_t>(expected_cache_size * 0.9);
    cache.GetFreeSlotAndLock(data_size, &meta1);

    uint8_t* addr = (uint8_t*) cache.GetSlotToReadAndLock(meta1->id, data_size, &meta2);
    ASSERT_THAT(addr, Eq(nullptr));

}

TEST_F(DataCacheTests, GetFreeSlotRemovesOldMetadataRecords) {
    DataCache cache{expected_cache_size, 0};
    CacheMeta* meta3, *meta4, *meta5;
    CacheMeta* meta;
    cache.GetFreeSlotAndLock(10, &meta1);
    cache.GetFreeSlotAndLock(10, &meta2);
    cache.GetFreeSlotAndLock(expected_cache_size - 30, &meta3);
    auto id1 = meta1->id;
    auto id2 = meta2->id;
    auto id3 = meta3->id;

    cache.UnlockSlot(meta1);
    cache.UnlockSlot(meta2);
    cache.UnlockSlot(meta3);

    cache.GetFreeSlotAndLock(10, &meta4);
    auto id4 = meta4->id;

    cache.UnlockSlot(meta4);
    cache.GetFreeSlotAndLock(30, &meta5);
    uint8_t* addr1 = (uint8_t*) cache.GetSlotToReadAndLock(id1, 10, &meta);
    uint8_t* addr2 = (uint8_t*) cache.GetSlotToReadAndLock(id2, 10, &meta);
    uint8_t* addr3 = (uint8_t*) cache.GetSlotToReadAndLock(id3, expected_cache_size - 30, &meta);
    uint8_t* addr4 = (uint8_t*) cache.GetSlotToReadAndLock(id4, 10, &meta);

    ASSERT_THAT(addr1, Eq(nullptr));
    ASSERT_THAT(addr2, Eq(nullptr));
    ASSERT_THAT(addr3, Eq(nullptr));
    ASSERT_THAT(addr4, Ne(nullptr));
    ASSERT_THAT(meta->size, Eq(10));
}

TEST_F(DataCacheTests, GetFreeSlotRemovesOldWhenCrossTheBoundary) {
    DataCache cache{expected_cache_size, 0};
    CacheMeta* meta1, *meta2, *meta3;
    CacheMeta* meta4, *meta5, *meta;
    auto addr1_alloc = cache.GetFreeSlotAndLock(expected_cache_size / 3 - 1, &meta1);
    auto addr2_alloc = cache.GetFreeSlotAndLock(expected_cache_size / 3 - 1, &meta2);
    auto addr3_alloc = cache.GetFreeSlotAndLock(expected_cache_size / 3 - 1, &meta3);
    auto id1 = meta1->id;
    auto id2 = meta2->id;
    auto id3 = meta3->id;
    cache.UnlockSlot(meta1);
    cache.UnlockSlot(meta2);
    cache.UnlockSlot(meta3);
    auto addr4_alloc = cache.GetFreeSlotAndLock(expected_cache_size / 2 + 5, &meta4);
    auto id4 = meta4->id;
    cache.UnlockSlot(meta4);
    auto addr5_alloc = cache.GetFreeSlotAndLock(expected_cache_size / 2 + 5, &meta5);
    auto id5 = meta5->id;

    uint8_t* addr1 = (uint8_t*) cache.GetSlotToReadAndLock(id1, expected_cache_size / 3 - 1, &meta);
    uint8_t* addr2 = (uint8_t*) cache.GetSlotToReadAndLock(id2, expected_cache_size / 3 - 1, &meta);
    uint8_t* addr3 = (uint8_t*) cache.GetSlotToReadAndLock(id3, expected_cache_size / 3 - 1, &meta);
    uint8_t* addr4 = (uint8_t*) cache.GetSlotToReadAndLock(id4, expected_cache_size / 2 + 5, &meta);
    uint8_t* addr5 = (uint8_t*) cache.GetSlotToReadAndLock(id5, expected_cache_size / 2 + 5, &meta);

    ASSERT_THAT(addr1_alloc, Ne(nullptr));
    ASSERT_THAT(addr2_alloc, Ne(nullptr));
    ASSERT_THAT(addr3_alloc, Ne(nullptr));
    ASSERT_THAT(addr4_alloc, Ne(nullptr));
    ASSERT_THAT(addr5_alloc, Ne(nullptr));

    ASSERT_THAT(addr1, Eq(nullptr));
    ASSERT_THAT(addr2, Eq(nullptr));
    ASSERT_THAT(addr3, Eq(nullptr));
    ASSERT_THAT(addr4, Eq(nullptr));
    ASSERT_THAT(addr5, Ne(nullptr));


    ASSERT_THAT(meta->size, Eq(expected_cache_size / 2 + 5));
}



TEST_F(DataCacheTests, GetSlotToReadSizeOk) {
    CacheMeta* meta;
    cache.GetFreeSlotAndLock(expected_size, &meta1);

    uint8_t* addr1 = (uint8_t*) cache.GetSlotToReadAndLock(meta1->id, expected_size, &meta);

    ASSERT_THAT(addr1, Ne(nullptr));
    ASSERT_THAT(meta->size, Eq(expected_size));
}

TEST_F(DataCacheTests, GetSlotToReadWrongSize) {
    CacheMeta* meta;
    cache.GetFreeSlotAndLock(expected_size, &meta1);

    uint8_t* addr1 = (uint8_t*) cache.GetSlotToReadAndLock(meta1->id, expected_size + 1, &meta);

    ASSERT_THAT(addr1, Eq(nullptr));
}



TEST_F(DataCacheTests, CannotGetFreeSlotIfNeedCleanOnebeingReaded) {
    CacheMeta* meta;

    uint8_t* ini_addr = (uint8_t*) cache.GetFreeSlotAndLock(10, &meta1);
    auto res = cache.GetSlotToReadAndLock(meta1->id, 10, &meta);
    uint8_t* addr = (uint8_t*) cache.GetFreeSlotAndLock(expected_cache_size, &meta2);

    ASSERT_THAT(ini_addr, Ne(nullptr));
    ASSERT_THAT(res, Eq(ini_addr));
    ASSERT_THAT(addr, Eq(nullptr));
}


TEST_F(DataCacheTests, CanGetFreeSlotIfWasUnlocked) {
    CacheMeta* meta;
    cache.GetFreeSlotAndLock(10, &meta1);
    cache.UnlockSlot(meta1);
    cache.GetSlotToReadAndLock(meta1->id, 10, &meta);
    cache.UnlockSlot(meta);
    auto addr = cache.GetFreeSlotAndLock(expected_cache_size, &meta2);

    ASSERT_THAT(addr, Ne(nullptr));
}

TEST_F(DataCacheTests, IncreasLockForEveryRead) {
    CacheMeta* meta;
    cache.GetFreeSlotAndLock(10, &meta1);
    cache.GetSlotToReadAndLock(meta1->id, 10, &meta);
    cache.GetSlotToReadAndLock(meta1->id, 10, &meta);
    cache.UnlockSlot(meta);
    auto addr = cache.GetFreeSlotAndLock(expected_cache_size, &meta2);

    ASSERT_THAT(addr, Eq(nullptr));
}

TEST_F(DataCacheTests, DecreasLockForEveryUnlock) {
    CacheMeta* meta;
    cache.GetFreeSlotAndLock(10, &meta1);
    cache.UnlockSlot(meta1);

    cache.GetSlotToReadAndLock(meta1->id, 10, &meta);
    cache.GetSlotToReadAndLock(meta1->id, 10, &meta);
    cache.UnlockSlot(meta);
    cache.UnlockSlot(meta);
    auto addr = cache.GetFreeSlotAndLock(expected_cache_size, &meta2);

    ASSERT_THAT(addr, Ne(nullptr));
}


TEST_F(DataCacheTests, GetFreeSlotCreatesCorrectIds) {
    CacheMeta* meta3, *meta4;
    cache.GetFreeSlotAndLock(10, &meta1);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    cache.GetFreeSlotAndLock(10, &meta2);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    cache.GetFreeSlotAndLock(10, &meta3);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    cache.GetFreeSlotAndLock(10, &meta4);

    auto c1 = static_cast<uint32_t>(meta1->id);
    auto c2 = static_cast<uint32_t>(meta2->id);
    auto c3 = static_cast<uint32_t>(meta3->id);
    auto c4 = static_cast<uint32_t>(meta4->id);

    auto t1 = meta1->id >> 32;
    auto t2 = meta2->id >> 32;
    auto t3 = meta3->id >> 32;
    auto t4 = meta4->id >> 32;

    ASSERT_THAT(c2, Eq(c1 + 1));
    ASSERT_THAT(c3, Eq(c2 + 1));
    ASSERT_THAT(c4, Eq(c3 + 1));

    ASSERT_THAT(t2, Ne(t1));
    ASSERT_THAT(t2, Ne(t3));
    ASSERT_THAT(t4, Ne(t3));
    ASSERT_THAT(t1, Ne(t3));
    ASSERT_THAT(t4, Ne(t2));
    ASSERT_THAT(t4, Ne(t1));

}

}
