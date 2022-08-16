#include "model_mgmt.h"

#include <cstring>
#include <new>

namespace reinforcement_learning
{
namespace model_management
{
model_data::model_data() = default;

// copy constructor: allocate new memory and copy over other's data
model_data::model_data(model_data const& other)
    : _data(new char[other._data_sz]), _data_sz(other._data_sz), _refresh_count(other._refresh_count)
{
  if (_data_sz > 0) std::memcpy(_data, other._data, _data_sz);
}

// move constructor: take other's data pointer and set original pointer to null
model_data::model_data(model_data&& other) noexcept
    : _data(other._data), _data_sz(other._data_sz), _refresh_count(other._refresh_count)
{
  other._data = nullptr;
}

// pass-by-value assignment operator
model_data& model_data::operator=(model_data other) noexcept
{
  std::swap(_data, other._data);
  std::swap(_data_sz, other._data_sz);
  std::swap(_refresh_count, other._refresh_count);
  return *this;
}

model_data::~model_data() { free(); }

char* model_data::data() const { return _data; }

void model_data::increment_refresh_count() { ++_refresh_count; }

size_t model_data::data_sz() const { return _data_sz; }

uint32_t model_data::refresh_count() const { return _refresh_count; }

void model_data::data_sz(const size_t fillsz) { _data_sz = fillsz; }

char* model_data::alloc(const size_t desired)
{
  // wrap the allocation in a unique_ptr for exception safety
  std::unique_ptr<char> data_new(new char[desired]);
  char* data_new_ptr = data_new.get();
  std::swap(_data, data_new_ptr);
  data_new.release();
  _data_sz = desired;

  // after swap, data_new_ptr now holds the original _data ptr
  if (data_new_ptr != nullptr)
  {
    delete[] data_new_ptr;
    data_new_ptr = nullptr;
  }

  return _data;
}

void model_data::free()
{
  if (_data != nullptr)
  {
    delete[] _data;
    _data = nullptr;
  }
  _data_sz = 0;
}

}  // namespace model_management
}  // namespace reinforcement_learning
