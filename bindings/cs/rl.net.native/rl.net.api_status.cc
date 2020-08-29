#include "rl.net.api_status.h"

API reinforcement_learning::api_status* CreateApiStatus()
{
    return new reinforcement_learning::api_status();
}

API void DeleteApiStatus(reinforcement_learning::api_status* status)
{
    delete status;
}

API const char* GetApiStatusErrorMessage(reinforcement_learning::api_status* status)
{
    return status->get_error_msg();
}

API int GetApiStatusErrorCode(reinforcement_learning::api_status* status)
{
    return status->get_error_code();
}

API void UpdateApiStatusSafe(reinforcement_learning::api_status* status, int error_code, const char* message)
{
    // api_status takes a copy of the message string coming in, since it has no way to enforce that its callers
    // do not deallocate the buffer after calling try_update.
    reinforcement_learning::api_status::try_update(status, error_code, message);
}

API void ClearApiStatusSafe(reinforcement_learning::api_status* status)
{
    reinforcement_learning::api_status::try_clear(status);
}
