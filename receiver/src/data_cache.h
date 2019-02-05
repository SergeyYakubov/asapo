#ifndef ASAPO_DATA_CACHE_H
#define ASAPO_DATA_CACHE_H

#include <stdint.h>
#include <memory>
#include <mutex>
#include <deque>

namespace asapo {

struct CacheMeta {
  uint64_t id;
  void* addr;
  uint64_t size;
  bool locked;
};

class DataCache {
 public:
  explicit DataCache(uint64_t cache_size,float keepunlocked_ratio);
  void* GetFreeSlot(uint64_t size,uint64_t* id);
  void* GetSlotToReadAndLock(uint64_t id,uint64_t* size);
 private:
  uint64_t cache_size_;
  float keepunlocked_ratio_;
  uint64_t next_id_ = 1;
  uint64_t cur_pointer_ = 0;
  std::unique_ptr<uint8_t[]> cache_;
  std::mutex mutex_;
  std::deque<CacheMeta> meta_;
  bool SlotTooCloseToCurrentPointer(const CacheMeta& meta);
  bool CleanOldSlots();
};

}

#endif //ASAPO_DATA_CACHE_H
