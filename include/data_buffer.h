#pragma once
#include <string>
#include <vector>
#include "vw/common/vwvis.h"

namespace reinforcement_learning { namespace utility {

/*
 * Data buffer used for serialized messages.  Data buffer
 * consists of 2 parts.  1) preamble 2) body
 *
 * body: can start somewhere in the middle and end before the
 * buffer ends.
 * preamble: is n bytes immediately before the body.
 * preamble is usually 8 bytes and sometimes zero if there is no preamble
 */
class VW_DLL_PUBLIC data_buffer {
public:
  using value_type = unsigned char;

  explicit data_buffer(size_t body_size = 1024);

  // Get a pointer to beginning of preamble
  value_type *preamble_begin();

  size_t preamble_size() const;

  // Return pointer to beginning of buffer, starting at offset from the real beginning.
  value_type *body_begin();

  // Body size (does not include the preamble)
  size_t body_filled_size() const;

  // How large can we make the Body? (does not include the preamble.)
  size_t body_capacity() const;

  // Offset for beginning of the body region from the beginning of the buffer
  size_t get_body_beginoffset() const;

  // Offset for beginning of the body data from the beginning of the buffer
  int set_body_beginoffset(size_t beginoffset);

  // Offset for end of the body data from the beginning of the buffer
  size_t get_body_endoffset() const;

  // Offset for end of the body data from the beginning of the buffer
  int set_body_endoffset(size_t endoffset);

  // Size of the entire filled buffer (body + preamble)
  size_t buffer_filled_size() const;

  // Will resize entire buffer
  void resize_body_region(size_t size);

  // Clear the contents of the buffer
  void reset();

  // Get the beginning of the raw buffer
  value_type *raw_begin();

private:
  std::vector<value_type> _buffer;
  // Offset for beginning of the body data from the beginning of the buffer
  size_t _body_beginoffset;
  // Offset for end of the body data from beginning of the buffer
  size_t _body_endoffset;
  // Size in bytes of the preamble region
  const size_t _preamble_size;
};
} // namespace utility
} // namespace reinforcement_learning
