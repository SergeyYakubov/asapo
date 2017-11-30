#ifndef HIDRA2_DUMMYEVENTDETECTOR_DUMMYYIELDABLE_H
#define HIDRA2_DUMMYEVENTDETECTOR_DUMMYYIELDABLE_H

#include <producer/producer.h>

class DummyYieldable : public HIDRA2::Yieldable<HIDRA2::FileChunk>
{
private:
    static const size_t kChunkSize;

    void* address;
    size_t size;
    size_t pos;
public:
    DummyYieldable(void* address, size_t size)
    {
        this->address = address;
        this->size = size;
        this->pos = 0;
    };

    HIDRA2::FileChunk next() override;

    bool is_done() const override;
};


#endif //HIDRA2_DUMMYEVENTDETECTOR_DUMMYYIELDABLE_H
