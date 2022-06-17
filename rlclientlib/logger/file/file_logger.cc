#include "file_logger.h"

#include "api_status.h"
#include "err_constants.h"

#include <sys/stat.h>

#include <fstream>
namespace reinforcement_learning
{
namespace logger
{
namespace file
{

file_logger::file_logger(const std::string& file_name, i_trace* trace) : _file_name(file_name), _trace(trace) {}

int file_logger::init(const utility::configuration& config, api_status* status)
{
  _file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  try
  {
    _file.open(_file_name, std::ios::binary);
  }
  catch (const std::ios_base::failure& e)
  {
    RETURN_ERROR_LS(_trace, status, file_open_error) << " File:" << _file_name << " Error:" << e.what();
  }
  return error_code::success;
}

int file_logger::v_send(const buffer& data, api_status* status)
{
  try
  {
    _file.write(reinterpret_cast<char*>(data->preamble_begin()), data->buffer_filled_size());
    _file.flush();
  }
  catch (const std::ios_base::failure& e)
  {
    RETURN_ERROR_LS(_trace, status, file_open_error) << " File:" << _file_name << " Error:" << e.what();
  }
  return error_code::success;
}
}  // namespace file
}  // namespace logger
}  // namespace reinforcement_learning
