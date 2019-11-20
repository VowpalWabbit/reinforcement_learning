#include "invariant_helpers.h"
#include <cstring>
namespace reinforcement_learning {
  namespace utility {
    //helper: check if at least one of the arguments is null or empty
    int check_null_or_empty(const char* arg1, const char* arg2, api_status* status) {
      if (!arg1 || !arg2 || strlen(arg1) == 0 || strlen(arg2) == 0) {
        api_status::try_update(status, error_code::invalid_argument,
          "one of the arguments passed to the ds is null or empty");
        return error_code::invalid_argument;
      }
      return error_code::success;
    }

    int check_null_or_empty(const char* arg1, api_status* status) {
      if (!arg1 || strlen(arg1) == 0) {
        api_status::try_update(status, error_code::invalid_argument,
          "one of the arguments passed to the ds is null or empty");
        return error_code::invalid_argument;
      }
      return error_code::success;
    }
  }
}
