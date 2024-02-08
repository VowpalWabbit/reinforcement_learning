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

int model_data::set_data(const char* vw_model, size_t len)
{
  if (vw_model == nullptr || len == 0)
  {
    return reinforcement_learning::error_code::static_model_load_error;
  }

  char* buffer = this->alloc(len);
  if (buffer == nullptr) { return reinforcement_learning::error_code::static_model_load_error; }

  memcpy(buffer, vw_model, len);
  this->data_sz(len);
}

}  // namespace model_management
}  // namespace reinforcement_learning
