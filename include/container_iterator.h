#pragma once

#include <iterator>
#include <vector>
#include <type_traits>

namespace reinforcement_learning {
  /**
   * @brief Forward iterator class used to access the TElem collection
   */
  template<typename TElem, typename TColl = std::vector<TElem>>
  class container_iterator : public std::iterator<
    std::forward_iterator_tag,
    TElem> {
  public:
    //! Construct an (action, probability) collection iterator using the ranking_response implementation
    container_iterator(TColl& coll)
      : _coll(coll)
      , _idx(0) {
      static_assert(std::is_same<TElem, TColl::value_type>::value, "TColl must be a collection of TElem");
    }

    //! Construct an (action, probability) collection iterator using the ranking_response implementation and size
    container_iterator(TColl& coll, size_t idx)
      : _coll(coll)
      , _idx(idx) {
      static_assert(std::is_same<TElem, TColl::value_type>::value, "TColl must be a collection of TElem");
    }
    //! Move the iterator to the next element
    iterator& operator++() {
      ++_idx;
      return *this;
    }
    //! Inequality comparison for the iterator
    bool operator!=(const container_iterator& other) const {
      return _idx != other._idx;
    }
    //! Dereferencing operator to get the (action, probability) pair
    TElem& operator*() {
      return _coll[_idx];
    }
    //! Allow comparison of iterators
    bool operator<(const container_iterator& rhs) const {
      return _idx < rhs._idx;
    }
    //! Allow distance measurement
    int64_t operator-(const container_iterator& rhs) const {
      return _idx - rhs._idx;
    }
    //! Increment the index
    container_iterator operator+(const uint32_t idx) const {
      return { _coll, _idx + idx };
    }

  private:
    TColl& _coll;
    size_t _idx;
  };

  /**
  * @brief Forward const iterator class used to access the TElem collection
  */
  template<typename TElem, typename TColl = std::vector<TElem>>
  class const_container_iterator : public std::iterator<
    std::forward_iterator_tag,
    TElem> {
  public:
    //! Construct an (action, probability) collection iterator using the ranking_response implementation
    const_container_iterator(const TColl& coll)
      : _coll(coll)
      , _idx(0) {
      static_assert(std::is_same<TElem, TColl::value_type>::value, "TColl must be a collection of TElem");
    }
    //! Construct an (action, probability) collection iterator using the ranking_response implementation and size
    const_container_iterator(const TColl& coll, size_t idx)
      : _coll(coll)
      , _idx(idx) {
      static_assert(std::is_same<TElem, TColl::value_type>::value, "TColl must be a collection of TElem");
    }
    //! Move the iterator to the next element
    const_container_iterator& operator++() {
      ++_idx;
      return *this;
    }
    //! Inequality comparison for the iterator
    bool operator!=(const const_container_iterator& other) const {
      return _idx != other._idx;
    }
    //! Dereferencing operator to get the (action, probability) pair
    const TElem& operator*() const {
      return _coll[_idx];
    }
    //! Allow comparison of iterators
    bool operator<(const const_container_iterator& rhs) const {
      return _idx < rhs._idx;
    }
    //! Allow distance measurement
    int64_t operator-(const const_container_iterator& rhs) const {
      return _idx - rhs._idx;
    }
    //! Increment the index
    const_container_iterator operator+(const uint32_t idx) const {
      return { _p_resp,_idx + idx };
    }

  private:
    const TColl& _coll;
    size_t _idx;
  };
}