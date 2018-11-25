#pragma once
#include <streambuf>
namespace reinforcement_learning { namespace utility {
  class data_buffer;
  class data_buffer_streambuf :
      public std::streambuf
  {
  public:
    explicit data_buffer_streambuf(data_buffer*);
    int_type overflow(int_type) override;
    int_type sync() override;
    void finalize();
    ~data_buffer_streambuf();

  private:
    data_buffer* _db;
    const size_t GROW_BY = 2048;
    bool _finalized = false;
  };
}}

