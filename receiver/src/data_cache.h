#ifndef ASAPO_DATA_CACHE_H
#define ASAPO_DATA_CACHE_H

#include <stdint.h>
#include <memory>
#include <mutex>
#include <deque>

#include "asapo/preprocessor/definitions.h"


namespace asapo {

struct CacheMeta {
    uint64_t id;
    void* addr;
    uint64_t size;
    int lock;
};

class DataCache {
  public:
    explicit DataCache(uint64_t cache_size_gb, float keepunlocked_ratio);
    VIRTUAL void* GetFreeSlotAndLock(uint64_t size, CacheMeta** meta);
    VIRTUAL void* GetSlotToReadAndLock(uint64_t id, uint64_t data_size, CacheMeta** meta);
    VIRTUAL bool UnlockSlot(CacheMeta* meta);
    VIRTUAL ~DataCache() = default;
  private:
    uint64_t cache_size_;
    float keepunlocked_ratio_;
    uint32_t counter_;
    uint64_t cur_pointer_ = 0;
    std::unique_ptr<uint8_t[]> cache_;
    std::mutex mutex_;
    std::deque<std::unique_ptr<CacheMeta>> meta_;
    bool SlotTooCloseToCurrentPointer(const CacheMeta* meta);
    bool CleanOldSlots(uint64_t size);
    void* AllocateSlot(uint64_t size);
    bool CheckAllocationSize(uint64_t size);
    uint64_t GetNextId();
};

using SharedCache = std::shared_ptr<asapo::DataCache>;

}

#endif //ASAPO_DATA_CACHE_H
