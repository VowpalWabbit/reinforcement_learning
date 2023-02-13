#include "model_mgmt.h"

namespace reinforcement_learning
{
namespace model_management
{

char* model_data::data() { return _data.data(); }
const char* model_data::data() const { return _data.data(); }

void model_data::increment_refresh_count() { ++_refresh_count; }

size_t model_data::data_sz() const { return _data.size(); }

uint32_t model_data::refresh_count() const { return _refresh_count; }

void model_data::data_sz(const size_t fillsz) { _data.resize(fillsz); }

char* model_data::alloc(const size_t desired)
{
  _data.clear();
  _data.resize(desired);
  return _data.data();
}

void model_data::free() { _data.clear(); }

}  // namespace model_management
}  // namespace reinforcement_learning
