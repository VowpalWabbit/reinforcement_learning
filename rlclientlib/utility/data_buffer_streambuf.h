#pragma once
#include <streambuf>
namespace reinforcement_learning
{
namespace utility
{
class data_buffer;
/**
 * \brief A streambuf class that is backed by data_buffer.  streambuf can be passed to
 * std::ostream and used to serialize using << and >> operators.
 * This is used while serializing json into data_buffer.
 *
 * See https://en.cppreference.com/w/cpp/io/basic_streambuf for additional details on streambuf
 */
class data_buffer_streambuf : public std::streambuf
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
}  // namespace utility
}  // namespace reinforcement_learning
