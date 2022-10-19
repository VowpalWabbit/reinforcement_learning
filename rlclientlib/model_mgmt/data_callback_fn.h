#pragma once
#include "model_mgmt.h"

#include <functional>

namespace reinforcement_learning
{
class i_trace;
namespace model_management
{
class data_callback_fn
{
public:
  using data_fn = std::function<void(const model_data&)>;

  int report_data(const model_data& data, i_trace* trace, api_status* status = nullptr);

  // Typed constructor
  template <typename DataCntxt>
  using data_fn_ptr = void (*)(const model_data&, DataCntxt*);

  template <typename DataCntxt>
  explicit data_callback_fn(data_fn_ptr<DataCntxt> fn, DataCntxt* context)
  {
    if (fn != nullptr) { _fn = std::bind(fn, std::placeholders::_1, context); }
  }

  ~data_callback_fn() = default;

  data_callback_fn(const data_callback_fn&) = delete;
  data_callback_fn(data_callback_fn&&) = delete;
  data_callback_fn& operator=(const data_callback_fn&) = delete;
  data_callback_fn& operator=(data_callback_fn&&) = delete;

private:
  data_fn _fn;
};

}  // namespace model_management
}  // namespace reinforcement_learning
