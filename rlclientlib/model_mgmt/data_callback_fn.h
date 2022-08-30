#pragma once
#include "model_mgmt.h"
namespace reinforcement_learning
{
class i_trace;
namespace model_management
{
class data_callback_fn
{
public:
  using data_fn = void (*)(const model_data& data, void*);

  // This function calls a C-style function pointer with argument cast to void*
  // Disable UBSan here to prevent generating an error
#ifdef RL_USE_UBSAN
  __attribute__((no_sanitize("undefined")))
#endif
  int report_data(const model_data& data, i_trace* trace, api_status* status = nullptr);

  // Typed constructor
  template <typename DataCntxt>
  using data_fn_t = void (*)(const model_data&, DataCntxt*);

  template <typename DataCntxt>
  explicit data_callback_fn(data_fn_t<DataCntxt>, DataCntxt*);

  ~data_callback_fn() = default;

  data_callback_fn(const data_callback_fn&) = delete;
  data_callback_fn(data_callback_fn&&) = delete;
  data_callback_fn& operator=(const data_callback_fn&) = delete;
  data_callback_fn& operator=(data_callback_fn&&) = delete;

private:
  data_callback_fn(data_fn, void*);
  data_fn _fn;
  void* _context;
};

template <typename DataCntxt>
data_callback_fn::data_callback_fn(data_fn_t<DataCntxt> fn, DataCntxt* ctxt)
    : data_callback_fn((data_fn)fn, (void*)ctxt)
{
}
}  // namespace model_management
}  // namespace reinforcement_learning
