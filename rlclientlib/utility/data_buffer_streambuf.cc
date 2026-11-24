#include "data_buffer_streambuf.h"

#include "data_buffer.h"

#include <cassert>

namespace reinforcement_learning
{
namespace utility
{
data_buffer_streambuf::data_buffer_streambuf(data_buffer* db) : _db(db)
{
  // Start the buffer with some minimum size
  _db->resize_body_region(GROW_BY);
  // Set buffer body to start at the beginning of body region
  _db->set_body_beginoffset(_db->preamble_size());
  // Set buffer body to end at the end of filled body which is
  // at the beginning of the buffer when we start out
  _db->set_body_endoffset(_db->preamble_size());
  // Sanity check body capacity
  assert(db->body_capacity() > 1);
  // Setup streambuf to write from beginning to end of body region
  // Reserve one byte for overflow
  setp(reinterpret_cast<char*>(_db->body_begin()),
      reinterpret_cast<char*>(_db->body_begin() + _db->body_capacity() - 1));
}

std::streambuf::int_type data_buffer_streambuf::overflow(int_type ch)
{
  // save the overflow character
  char* loc = pptr();
  *loc = ch;
  const auto old_body_size = _db->body_filled_size() + _db->body_capacity();
  _db->set_body_endoffset(_db->preamble_size() + old_body_size);

  // We are at the end of buffer, increase size
  try
  {
    _db->resize_body_region(old_body_size + GROW_BY);

    setp(reinterpret_cast<char*>(_db->body_begin() + old_body_size),
        reinterpret_cast<char*>(_db->body_begin() + old_body_size + GROW_BY));

    return ch;
  }
  catch (...)
  {
    return EOF;
  }
}

std::basic_streambuf<char>::int_type data_buffer_streambuf::sync()
{
  auto offset = _db->preamble_size() + (pptr() - pbase());
  _db->set_body_endoffset(offset);
  return 0;
}

void data_buffer_streambuf::finalize()
{
  if (!_finalized)
  {
    _finalized = true;
    // Null terminate but don't include that in the size
    *pptr() = '\0';
    const auto used_bytes = _db->preamble_size() + (pptr() - pbase());
    _db->set_body_endoffset(used_bytes);
  }
}

data_buffer_streambuf::~data_buffer_streambuf() { finalize(); }
}  // namespace utility
}  // namespace reinforcement_learning
