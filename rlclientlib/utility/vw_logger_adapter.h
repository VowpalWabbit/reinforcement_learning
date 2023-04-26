#include "trace_logger.h"
#include "vw/io/logger.h"

#include <string>

namespace reinforcement_learning
{
namespace utility
{
VW::io::logger make_vw_trace_logger(i_trace* trace_logger);
}
}  // namespace reinforcement_learning