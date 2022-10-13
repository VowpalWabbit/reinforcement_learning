#pragma once

#include <iterator>
#include <type_traits>
#include <vector>

namespace reinforcement_learning
{
/**
 * @brief Forward iterator class used to access the TElem collection
 */
template <typename TElem, typename TColl = std::vector<TElem>>
class container_iterator : public std::iterator<std::random_access_iterator_tag, TElem>
{
public:
  //! Construct an iterator using container implementation
  container_iterator(TColl& coll) : _coll(coll), _idx(0)
  {
    static_assert(std::is_same<TElem, typename TColl::value_type>::value, "TColl must be a collection of TElem");
  }

  //! Construct an iterator using container implementation and size
  container_iterator(TColl& coll, size_t idx) : _coll(coll), _idx(idx)
  {
    static_assert(std::is_same<TElem, typename TColl::value_type>::value, "TColl must be a collection of TElem");
  }
  //! Move the iterator to the previous element
  container_iterator& operator--()
  {
    --_idx;
    return *this;
  }
  //! Move the iterator to the next element
  container_iterator& operator++()
  {
    ++_idx;
    return *this;
  }
  //! Inequality comparison for the iterator
  bool operator!=(const container_iterator& other) const { return _idx != other._idx; }
  // Equality comparison for the iterator
  bool operator==(const container_iterator& other) const { return _idx == other._idx; }
  //! Dereferencing operator to get the TElem
  TElem& operator*() { return _coll[_idx]; }
  TElem& operator*() const { return _coll[_idx]; }

  //! Allow comparison of iterators
  bool operator<(const container_iterator& rhs) const { return _idx < rhs._idx; }

  //! Allow comparison of iterators
  bool operator<=(const container_iterator& rhs) const { return _idx <= rhs._idx; }

  //! Allow comparison of iterators
  bool operator>(const container_iterator& rhs) const { return _idx > rhs._idx; }

  //! Allow comparison of iterators
  bool operator>=(const container_iterator& rhs) const { return _idx >= rhs._idx; }
  //! Allow distance measurement
  size_t operator-(const container_iterator& rhs) const
  {
    return static_cast<size_t>(_idx) - static_cast<size_t>(rhs._idx);
  }

  //! Allow distance measurement
  container_iterator operator-(const size_t idx) const { return {_coll, _idx - idx}; }

  //! Increment the index
  container_iterator operator+(const size_t idx) const { return {_coll, _idx + idx}; }

  //! add and assign the index
  container_iterator& operator+=(const size_t idx)
  {
    this->_idx += idx;
    return *this;
  }
  //! subtract and assign operator
  container_iterator& operator-=(const size_t idx)
  {
    this->_idx -= idx;
    return *this;
  }

  //! Assign the values for the iterator
  container_iterator& operator=(const container_iterator& other)
  {
    _coll = other._coll;
    _idx = other._idx;
    return *this;
  }

private:
  TColl& _coll;
  size_t _idx;
};

/**
 * @brief Forward const iterator class used to access the TElem collection
 */
template <typename TElem, typename TColl = std::vector<TElem>>
class const_container_iterator : public std::iterator<std::random_access_iterator_tag, TElem>
{
public:
  //! Construct an iterator using container implementation
  const_container_iterator(const TColl& coll) : _coll(coll), _idx(0)
  {
    static_assert(std::is_same<TElem, typename TColl::value_type>::value, "TColl must be a collection of TElem");
  }
  //! Construct an iterator using container implementation and size
  const_container_iterator(const TColl& coll, size_t idx) : _coll(coll), _idx(idx)
  {
    static_assert(std::is_same<TElem, typename TColl::value_type>::value, "TColl must be a collection of TElem");
  }
  //! Move the iterator to the next element
  const_container_iterator& operator++()
  {
    ++_idx;
    return *this;
  }
  //! Inequality comparison for the iterator
  bool operator!=(const const_container_iterator& other) const { return _idx != other._idx; }
  //! Dereferencing operator to get the TElem
  const TElem& operator*() const { return _coll[_idx]; }
  //! Allow comparison of iterators
  bool operator<(const const_container_iterator& rhs) const { return _idx < rhs._idx; }

  //! Allow distance measurement
  int64_t operator-(const const_container_iterator& rhs) const { return _idx - rhs._idx; }
  //! Increment the index
  const_container_iterator operator+(const uint32_t idx) const { return {_coll, _idx + idx}; }

private:
  const TColl& _coll;
  size_t _idx;
};
}  // namespace reinforcement_learning
