#pragma once

#include "events_sequence_id.h"
#include "ranking_event.h"
#include "str_util.h"
#include "trace_logger.h"

#include <list>
#include <queue>
#include <mutex>
#include <type_traits>

namespace reinforcement_learning {

  //a moving concurrent queue with locks and mutex
  template <class T>
  class event_queue {
  private:
    using queue_t = std::list<std::pair<T,size_t>>;
    using iterator_t = typename queue_t::iterator;

    const std::string _name;
    queue_t _queue;
    i_trace* _trace;
    std::mutex _mutex;
    int _drop_pass{ 0 };
    size_t _capacity{ 0 };

  public:
    event_queue(const char* name, i_trace* trace)
      : _trace(trace)
      , _name(name) {
      static_assert(std::is_base_of<event, T>::value, "T must be a descendant of event");
    }

    void pop(T* item)
    {
      std::unique_lock<std::mutex> mlock(_mutex);
      if (!_queue.empty())
      {
        auto entry(std::move(_queue.front()));
        *item = std::move(entry.first);
        _capacity = (std::max)(0, static_cast<int>(_capacity) - static_cast<int>(entry.second));
        _queue.pop_front();
      }
    }

    void push(T& item, size_t item_size) {
      push(std::move(item), item_size);
    }

    void push(T&& item, size_t item_size)
    {
      std::unique_lock<std::mutex> mlock(_mutex);
      _capacity += item_size;
      _queue.push_back({std::forward<T>(item),item_size});
    }

    void prune(float pass_prob)
    {
      std::unique_lock<std::mutex> mlock(_mutex);
      if (_queue.empty()) {
        return;
      }
      const events_sequence_id sequence_id(_queue.front().first, _queue.back().first, _queue.size());
      size_t dropped = 0;
      for (auto it = _queue.begin(); it != _queue.end();) {
        it = it->first.try_drop(pass_prob, _drop_pass) ? erase(it, dropped) : (++it);
      }
      const std::string message = utility::concat("[DROP] [", _name, "] Messages range: ", sequence_id.get(), " Dropped: ", dropped);
      TRACE_DEBUG(_trace, message);
      ++_drop_pass;
    }

    //approximate size
    size_t size()
    {
      std::unique_lock<std::mutex> mlock(_mutex);
      return _queue.size();
    }

    size_t capacity() const 
    {
      return _capacity;
    }

  private:
    //thread-unsafe
    iterator_t erase(iterator_t it, size_t& counter) {
      _capacity = (std::max)(0, static_cast<int>(_capacity) - static_cast<int>(it->second));
      ++counter;
      return _queue.erase(it);
    }
  };
}

