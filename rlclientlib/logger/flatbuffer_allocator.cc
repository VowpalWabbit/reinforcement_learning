#include "flatbuffer_allocator.h"
namespace reinforcement_learning
{
flatbuffer_allocator::flatbuffer_allocator(utility::data_buffer& data_buffer) : _buffer(data_buffer) {}

uint8_t* flatbuffer_allocator::allocate(size_t size)
{
  _buffer.resize_body_region(size);
  return _buffer.body_begin();
}

void flatbuffer_allocator::deallocate(uint8_t* p, size_t size)
{
  // Nothing to do.  Buffer cleanup will happen in data_buffer
}

uint8_t* flatbuffer_allocator::reallocate_downward(
    uint8_t* old_p, size_t old_size, size_t new_size, size_t in_use_back, size_t in_use_front)
{
  assert(new_size > old_size);  // vector_downward only grows
  _buffer.resize_body_region(new_size);
  // implementation is almost identical with memcpy_downward from
  // https://github.com/google/flatbuffers/blob/master/include/flatbuffers/flatbuffers.h, but since we are staying at
  // the same chunk, we 1) cannot use memcpy which has undefined behavior on copying data between overlapping regions
  // (https://en.cppreference.com/w/cpp/string/byte/memcpy) 2) do not need to copy head
  memmove(_buffer.body_begin() + new_size - in_use_back, _buffer.body_begin() + old_size - in_use_back, in_use_back);
  return _buffer.body_begin();
}
}  // namespace reinforcement_learning
