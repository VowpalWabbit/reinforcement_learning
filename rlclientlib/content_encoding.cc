#include "content_encoding.h"
#include "constants.h"
#include <cstring>

#ifndef _WIN32
#define _stricmp strcasecmp
#endif

namespace reinforcement_learning {
  content_encoding_enum to_content_encoding_enum(const char* content_encoding) {
    if (_stricmp(content_encoding, value::CONTENT_ENCODING_ZSTD_AND_DEDUP) == 0) {
      return content_encoding_enum::ZSTD_AND_DEDUP;
    }
    else {
      return content_encoding_enum::IDENTITY;
    }
  }

  const char* to_content_encoding_string(content_encoding_enum content_encoding)
  {
    if (content_encoding == content_encoding_enum::ZSTD_AND_DEDUP) {
      return value::CONTENT_ENCODING_ZSTD_AND_DEDUP;
    }
    else {
      return value::CONTENT_ENCODING_IDENTITY;
    }
  }
}