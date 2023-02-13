#include "data_callback_fn.h"

#include "err_constants.h"

#include <object_factory.h>

#include <exception>

namespace reinforcement_learning
{
namespace model_management
{
int model_management::data_callback_fn::report_data(const model_data& data, i_trace* trace, api_status* status)
{
  if (!_fn) { RETURN_ERROR_LS(trace, status, data_callback_not_set); }

  // Need not be thread safe since this is only called from one thread.
  try
  {
    _fn(data);
    return error_code::success;
  }
  catch (const std::exception& ex)
  {
    RETURN_ERROR_LS(trace, status, data_callback_exception) << ex.what();
  }
  catch (...)
  {
    RETURN_ERROR_LS(trace, status, data_callback_exception) << "Unknown exception";
  }
}

}  // namespace model_management
}  // namespace reinforcement_learning
