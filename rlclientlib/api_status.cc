#include "api_status.h"
#include "trace_logger.h"

namespace reinforcement_learning {

  inline std::string to_string(const std::vector<stack_frame_info> &call_stack) {
    std::stringstream ss;
    std::string delimiter;
    for (const auto &frame : call_stack) {
      ss << delimiter << frame.file_name << ":" << frame.line_number << " "
        << frame.function_name;
      delimiter = "\n";
    }
    return ss.str();
  }

  int api_status::get_error_code() const {
    return _error_code;
  }

  const char* api_status::get_error_msg() const {
    return _error_msg.c_str();
  }

  const std::vector<stack_frame_info> &api_status::get_call_stack() const {
    return _call_stack;
  }

  api_status::api_status()
    : _error_code(0), _error_msg("") {}

  //static helper: update the status if needed (i.e. if it is not null)
  void api_status::try_update(api_status* status, const int new_code, const char* new_msg) {
    if (status != nullptr) {
      status->_error_code = new_code;
      status->_error_msg = new_msg;
      status->_call_stack.clear();
    }
  }

  void api_status::try_push_stack_frame(api_status *status,
                                        const std::string &file_name,
                                        const std::string &function_name,
                                        size_t line_number) {
    if (status != nullptr) {
      status->_call_stack.push_back(
          stack_frame_info{file_name, function_name, line_number});
    }
  }

  void api_status::try_push_stack_frame(api_status *status,
                                        const stack_frame_info &frame) {
    if (status != nullptr) {
      status->_call_stack.push_back(frame);
    }
  }

  void api_status::try_clear(api_status* status) {
    if (status != nullptr) {
      status->_error_code = 0;
      status->_error_msg.clear();
      status->_call_stack.clear();
    }
  }

  status_builder::status_builder(i_trace* trace, api_status* status, const int code)
    : _code { code }, _status { status }, _trace {trace} {
    if ( enable_logging() )
      _os << "(ERR:" << _code << ")";
  }

  status_builder::status_builder(i_trace* trace, api_status* status, const int code, const std::string& file_name, const std::string& function_name, size_t line_number)
    : _code { code }, _status { status }, _trace {trace}, _stack_frame{file_name, function_name, line_number}, _stack_frame_provided{true} {
      if ( enable_logging() )
      _os << "(ERR:" << _code << ")";
  }

  status_builder::~status_builder() {
    if (_status != nullptr) {
      api_status::try_update(_status, _code, _os.str().c_str());
      if (_stack_frame_provided)
      {
        api_status::try_push_stack_frame(_status, _stack_frame);
      }
    }
    if (_trace != nullptr ) {
      _trace->log(0, _os.str());
    }
  }

  status_builder::operator int() const {
    return _code;
  }

  bool status_builder::enable_logging() const {
    return _status != nullptr || _trace != nullptr;
  }
}
