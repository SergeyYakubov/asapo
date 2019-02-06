#include "data_cache.h"

#include <iostream>

namespace asapo {

DataCache::DataCache(uint64_t cache_size, float keepunlocked_ratio) : cache_size_{cache_size},
                                                                      keepunlocked_ratio_{keepunlocked_ratio} {
    try {
        cache_.reset(new uint8_t[cache_size]);
    } catch (std::exception &e) {
        std::cout << "Cannot allocate data cache: " << e.what() << std::endl;
        exit(1);
    }

}

void* DataCache::AllocateSlot(uint64_t size) {
    auto tmp = cur_pointer_;

    if (cur_pointer_ + size > cache_size_) {
        cur_pointer_ = 0;
    }
    auto addr = cache_.get() + cur_pointer_;
    cur_pointer_ += size;

    if (!CleanOldSlots()) {
        cur_pointer_ = tmp;
        return nullptr;
    }
    return addr;
}

void* DataCache::GetFreeSlot(uint64_t size, uint64_t* id) {
    std::lock_guard<std::mutex> lock{mutex_};

    if (!CheckAllocationSize(size)) {
        return nullptr;
    }

    auto addr = AllocateSlot(size);
    if (!addr) {
        return nullptr;
    }

    *id = GetNextId();

    meta_.emplace_back(CacheMeta{*id, addr, size, false});

    return addr;
}

uint64_t DataCache::GetNextId() {
    return next_id_++;
}

bool DataCache::SlotTooCloseToCurrentPointer(const CacheMeta &meta) {
    uint64_t dist;
    uint64_t shift = (uint8_t*) meta.addr - cache_.get();
    if (shift > cur_pointer_) {
        dist = shift - cur_pointer_;
    } else {
        dist = cache_size_ - cur_pointer_ + shift;
    }
    return dist < cache_size_ * keepunlocked_ratio_;
}

void* DataCache::GetSlotToReadAndLock(uint64_t id, uint64_t* size) {
    std::lock_guard<std::mutex> lock{mutex_};
    for (auto& meta: meta_) {
        if (meta.id == id) {
            if (SlotTooCloseToCurrentPointer(meta)) {
                return nullptr;
            }
            *size = meta.size;
            meta.locked = true;
            return meta.addr;
        }
    }
    return nullptr;
}

bool DataCache::CleanOldSlots() {
    uint64_t last_ok = meta_.size();
    for (int64_t i = last_ok - 1; i >= 0; i--) {
        uint64_t start_position = (uint8_t*) meta_[i].addr - cache_.get();
        if (start_position > cur_pointer_) {
            last_ok = i;
        }
    }

    for (int64_t i = 0; i < last_ok; i++) {
        if (meta_[i].locked) return false;
    }

    if (last_ok != 0) {
        meta_.erase(meta_.begin(), meta_.begin() + last_ok);
    }
    return true;
}

bool DataCache::CheckAllocationSize(uint64_t size) {
    return size <= cache_size_;
}

}