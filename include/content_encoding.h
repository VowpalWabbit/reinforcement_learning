#pragma once

namespace reinforcement_learning {
  enum class content_encoding_enum
  {
    IDENTITY,
    ZSTD_AND_DEDUP
  };

  content_encoding_enum to_content_encoding_enum(const char *content_encoding);
  const char* to_content_encoding_string(content_encoding_enum content_encoding);
}