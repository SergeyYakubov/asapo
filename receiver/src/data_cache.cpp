#include "data_cache.h"
#include "data_cache.h"

#include <iostream>
#include <chrono>
#include <algorithm>


namespace asapo {

DataCache::DataCache(uint64_t cache_size, float keepunlocked_ratio) : cache_size_{cache_size},
    keepunlocked_ratio_{keepunlocked_ratio} {
    try {
        cache_.reset(new uint8_t[cache_size]);
    } catch (std::exception& e) {
        std::cout << "Cannot allocate data cache: " << e.what() << std::endl;
        exit(1);
    }

    srand(time(NULL));
    counter_ = rand() % 100 + 1;
}

void* DataCache::AllocateSlot(uint64_t size) {
    auto tmp = cur_pointer_;

    if (cur_pointer_ + size > cache_size_) {
        cur_pointer_ = 0;
    }
    auto addr = cache_.get() + cur_pointer_;
    cur_pointer_ += size;

    if (!CleanOldSlots(size)) {
        cur_pointer_ = tmp;
        return nullptr;
    }
    return addr;
}

void* DataCache::GetFreeSlotAndLock(uint64_t size, CacheMeta** meta) {
    std::lock_guard<std::mutex> lock{mutex_};
    *meta = nullptr;
    if (!CheckAllocationSize(size)) {
        return nullptr;
    }

    auto addr = AllocateSlot(size);
    if (!addr) {
        return nullptr;
    }

    auto id = GetNextId();

    *meta = new CacheMeta{id, addr, size, 1};
    meta_.emplace_back(std::unique_ptr<CacheMeta> {*meta});

    return addr;
}

uint64_t DataCache::GetNextId() {
    counter_++;
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    uint32_t timeMillis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    return (uint64_t) timeMillis << 32 | counter_;
}

bool DataCache::SlotTooCloseToCurrentPointer(const CacheMeta* meta) {
    uint64_t dist;
    uint64_t shift = (uint8_t*) meta->addr - cache_.get();
    if (shift > cur_pointer_) {
        dist = shift - cur_pointer_;
    } else {
        dist = cache_size_ - cur_pointer_ + shift;
    }
    return dist < cache_size_ * keepunlocked_ratio_;
}

// we allow to read if it was already locked - if lock come from reading - no problems, from writing -should not happen!
void* DataCache::GetSlotToReadAndLock(uint64_t id, uint64_t data_size, CacheMeta** meta) {
    std::lock_guard<std::mutex> lock{mutex_};
    for (auto& meta_rec : meta_) {
        if (meta_rec->id == id) {
            if (data_size != meta_rec->size || SlotTooCloseToCurrentPointer(meta_rec.get())) {
                return nullptr;
            }
            meta_rec->lock++;
            *meta = meta_rec.get();
            return meta_rec->addr;
        }
    }

    *meta = nullptr;
    return nullptr;
}

bool Intersects(uint64_t left1, uint64_t right1, uint64_t left2, uint64_t right2) {
    return (left1 >= left2 && left1 < right2) || (right1 <= right2 && right1 > left2);
}

bool DataCache::CleanOldSlots(uint64_t size) {
    int64_t last_del = -1;
    bool was_intersecting = false;
    for (uint64_t i = 0; i < meta_.size(); i++) {
        uint64_t start_position = (uint8_t*) meta_[i]->addr - cache_.get();
        if (Intersects(start_position, start_position + meta_[i]->size, cur_pointer_ - size, cur_pointer_)) {
            last_del = i;
            was_intersecting = true;
        } else {
            if (cur_pointer_ - size > 0 || was_intersecting) {
                break; // if we (re)started from 0, the intersecting slot might be not number 0, so we don't break until intersection was found
            }
        }
    }

    for (int i = 0; i <= last_del; i++) {
        if (meta_[i]->lock > 0) return false;
    }

    if (last_del >=0) {
     meta_.erase(meta_.begin(), meta_.begin() + last_del + 1);
    }
    return true;
}

bool DataCache::CheckAllocationSize(uint64_t size) {
    return size <= cache_size_;
}

bool DataCache::UnlockSlot(CacheMeta* meta) {
    if (meta == nullptr) {
        return false;
    }
    std::lock_guard<std::mutex> lock{mutex_};
    meta->lock = std::max(0, meta->lock - 1);
    return true;
}

}
