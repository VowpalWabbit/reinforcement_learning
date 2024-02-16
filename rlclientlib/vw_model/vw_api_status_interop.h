#pragma once

#include "vw/core/api_status.h"
#include "vw/core/error_constants.h"

#ifdef RETURN_ERROR
#undef RETURN_ERROR
#endif

#ifdef RETURN_ERROR_ARG
#undef RETURN_ERROR_ARG
#endif

#ifdef RETURN_ERROR_LS
#undef RETURN_ERROR_LS
#endif

#ifdef RETURN_IF_FAIL
#undef RETURN_IF_FAIL
#endif