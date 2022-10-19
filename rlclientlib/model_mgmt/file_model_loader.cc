#include "file_model_loader.h"
#include "err_constants.h"
#include "api_status.h"
#include <fstream>
#include <utility>

#include <sys/types.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#ifdef _WIN32
#define stat _stat
#endif

namespace reinforcement_learning { namespace model_management {

  int file_model_loader::get_file_modified_time(time_t& file_time, api_status* status) const {
    struct stat result {};
    if (stat(_file_name.c_str(), &result) == 0)
    {
      file_time = result.st_mtime;
      return error_code::success;
    }
    RETURN_ERROR_LS(_trace, status, file_stats_error) << " file_name = " << _file_name;
  }
  
  file_model_loader::file_model_loader(std::string file_name, bool file_must_exist, i_trace* trace_logger)
    : _file_name{ std::move(file_name) }, _file_must_exist{ file_must_exist }, _trace{ trace_logger }
  {}

  int file_model_loader::init(api_status* status) {
    if (_file_must_exist) {
      std::ifstream in_strm(_file_name.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
      if (!in_strm.good()) {
        RETURN_ERROR_LS(_trace, status, file_open_error) << " file_name = " << _file_name;
      }
    }
    return error_code::success;
  }

  int file_model_loader::get_data(model_data& data, api_status* status) {

    std::ifstream in_strm(_file_name.c_str(), std::ios::in|std::ios::binary|std::ios::ate);

    if(in_strm.good()) {
      
      // File exists read from it
      const auto curr_file_size = in_strm.tellg();

      time_t curr_last_modified;
      RETURN_IF_FAIL(get_file_modified_time(curr_last_modified, status));

      // If file has the same size and same timestamp, no need to reload
      if (curr_last_modified == _last_modified && (size_t)curr_file_size == _datasz)
        return error_code::success;

      in_strm.seekg(0, std::ios::beg);
      const auto buff = data.alloc(curr_file_size);
      if (!in_strm.read(buff, curr_file_size)){
        RETURN_ERROR_LS(_trace, status, file_read_error) << " file_name = " << _file_name;
      }
      data.data_sz(curr_file_size);
      data.increment_refresh_count();
      in_strm.close();
      _last_modified = curr_last_modified;
      _datasz = (size_t)curr_file_size;
    }
    else {
      // File does not exist or cannot open
      if(_file_must_exist) {
        RETURN_ERROR_LS(_trace, status, file_open_error) << " file_name = " << _file_name;
      }
    }
    return error_code::success;
  }

} }
