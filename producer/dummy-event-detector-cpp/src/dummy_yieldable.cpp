#include <iostream>
#include "dummy_yieldable.h"

const size_t DummyYieldable::kChunkSize = 10;

HIDRA2::FileChunk DummyYieldable::next()
{
    std::cout << "pos: " << pos << std::endl;
    auto chunk = HIDRA2::FileChunk {
        .ptr = address+this->pos,
        .start_byte = this->pos,
    };

    if(this->pos+kChunkSize > size) {
        chunk.chunk_size = size - this->pos;
    } else {
        chunk.chunk_size = (uint64_t) kChunkSize;
    }

    this->pos+=kChunkSize;
    return chunk;
}

bool DummyYieldable::is_done() const
{
    return pos >= size;
}
