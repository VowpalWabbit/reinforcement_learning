#include "stl_container_adapter.h"

#include <cassert>
namespace reinforcement_learning
{
namespace utility
{
stl_container_adapter::stl_container_adapter(data_buffer* db) : _db(db) {}

size_t stl_container_adapter::size() const { return _db->buffer_filled_size(); }

const stl_container_adapter::value_type& stl_container_adapter::operator[](size_t idx) const
{
  assert(idx < size());
  return *(_db->preamble_begin() + idx);
}

void stl_container_adapter::resize(size_t /*unused*/)
{
  assert(false);  // Resize is not supported.
}

#ifdef _WIN32
stdext::checked_array_iterator<stl_container_adapter::value_type*> stl_container_adapter::begin() const
{
  return {_db->preamble_begin(), _db->buffer_filled_size()};
}
#else
stl_container_adapter::value_type* stl_container_adapter::begin() const { return _db->preamble_begin(); }
#endif
}  // namespace utility
}  // namespace reinforcement_learning
