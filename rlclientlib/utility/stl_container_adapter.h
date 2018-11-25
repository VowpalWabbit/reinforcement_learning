#pragma once
#include "data_buffer.h"
#include <iterator>

namespace reinforcement_learning {namespace utility {
  class data_buffer;
  class stl_container_adapter {
  public:
    using value_type = data_buffer::value_type;

    explicit stl_container_adapter(data_buffer* db);
    stl_container_adapter(const stl_container_adapter& rhs) = default;
    stl_container_adapter(stl_container_adapter&& rhs) noexcept = default;
    size_t size() const;
    value_type& operator[] (size_t idx);
    void resize(size_t);

    #ifdef _WIN32
    stdext::checked_array_iterator<value_type *> begin() const;
    #else
    value_type * begin() const;
    #endif

  protected:
    data_buffer* _db;
  };
}}
