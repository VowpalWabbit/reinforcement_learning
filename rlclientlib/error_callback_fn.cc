#include "error_callback_fn.h"

using namespace std;

namespace reinforcement_learning
{
void error_callback_fn::report_error(const api_status& s)
{
  if (!_fn) { return; }

  lock_guard<mutex> lock(_mutex);
  if (_fn)
  {
    try
    {
      _fn(s);
    }
    catch (...)
    {
      // Error handler is throwing so can't call it again
    }
  }
}

}  // namespace reinforcement_learning
