#include "data_buffer.h"

#include "err_constants.h"
#include "logger/preamble.h"

#include <cassert>
namespace reinforcement_learning
{
namespace utility
{
data_buffer::data_buffer(size_t body_size)
    : _buffer(body_size + logger::preamble::size())
    , _body_beginoffset{logger::preamble::size()}
    , _body_endoffset{logger::preamble::size()}
    , _preamble_size{logger::preamble::size()}
{
  assert(body_size != 0);
}

void data_buffer::reset()
{
  _buffer.resize(_preamble_size + 1);
  _body_beginoffset = _preamble_size;
  _body_endoffset = _preamble_size;
}

data_buffer::value_type* data_buffer::raw_begin() { return _buffer.data(); }

size_t data_buffer::get_body_beginoffset() const { return _body_beginoffset; }

int data_buffer::set_body_beginoffset(size_t beginoffset)
{
  if (beginoffset > _buffer.size() || beginoffset < _preamble_size) { return error_code::invalid_argument; }

  _body_beginoffset = beginoffset;
  return error_code::success;
}

size_t data_buffer::get_body_endoffset() const { return _body_endoffset; }

int data_buffer::set_body_endoffset(size_t endoffset)
{
  if (endoffset > _buffer.size() || endoffset < preamble_size() || endoffset < _body_beginoffset)
  {
    return error_code::invalid_argument;
  }

  _body_endoffset = endoffset;
  return error_code::success;
}

size_t data_buffer::buffer_filled_size() const { return preamble_size() + body_filled_size(); }

data_buffer::value_type* data_buffer::body_begin() { return _buffer.data() + _body_beginoffset; }

size_t data_buffer::body_filled_size() const { return _body_endoffset - _body_beginoffset; }

size_t data_buffer::body_capacity() const { return _buffer.size() - _preamble_size; }

void data_buffer::resize_body_region(size_t size)
{
  assert(size != 0);

  _buffer.resize(size + _preamble_size);

  // If body became smaller, change body
  // region settings to sane values
  if (_body_endoffset >= (_buffer.size())) { _body_endoffset = _buffer.size() - 1; }
  if (_body_beginoffset >= (_buffer.size())) { _body_beginoffset = _buffer.size() - 1; }
}

data_buffer::value_type* data_buffer::preamble_begin() { return _buffer.data() + _body_beginoffset - _preamble_size; }

size_t data_buffer::preamble_size() const { return _preamble_size; }
}  // namespace utility
}  // namespace reinforcement_learning
