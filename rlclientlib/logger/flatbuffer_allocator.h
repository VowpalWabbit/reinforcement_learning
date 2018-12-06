#pragma once
#include "data_buffer.h"
#include "flatbuffers/flatbuffers.h"

namespace reinforcement_learning {
  class flatbuffer_allocator : public flatbuffers::Allocator {

  public:
    flatbuffer_allocator(utility::data_buffer& data_buffer);

    uint8_t* allocate(size_t size) override;
    void deallocate(uint8_t *p, size_t size) override;
    uint8_t* reallocate_downward(uint8_t *old_p, size_t old_size, size_t new_size, size_t in_use_back, size_t in_use_front) override;

  private:
    utility::data_buffer& _buffer;
  };
}
