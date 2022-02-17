#pragma once

#include "ranking_event.h"

#include <list>
#include <queue>
#include <mutex>
#include <type_traits>
#include "utility/config_helper.h"
#include "constants.h"

namespace reinforcement_learning {

  //a moving concurrent queue with locks and mutex
  template <class T>
  class event_queue {
  private:
    using queue_t = std::list<std::pair<T,size_t>>;
    using iterator_t = typename queue_t::iterator;

    queue_t _queue;
    std::mutex _mutex;
    int _drop_pass{ 0 };
    size_t _capacity{ 0 };
    size_t _max_capacity{ 0 };
    uint64_t _event_index{ 0 };
    events_counter_status _event_counter_status{ events_counter_status::DISABLE };
    float _subsample_rate{ 1.0f };

  public:
    event_queue(size_t max_capacity, events_counter_status event_counter_status = events_counter_status::DISABLE, float subsample_rate = 1.0f)
      : _max_capacity(max_capacity), _event_counter_status(event_counter_status), _subsample_rate(subsample_rate) {
    }

    bool pop(T* item)
    {
      std::unique_lock<std::mutex> mlock(_mutex);
      if (!_queue.empty())
      {
        auto entry(std::move(_queue.front()));
        *item = std::move(entry.first);
        _capacity = (std::max)(0, static_cast<int>(_capacity) - static_cast<int>(entry.second));
        _queue.pop_front();
        return true;
      }
      return false;
    }

    bool push(T& item, size_t item_size) {
      return push(std::move(item), item_size);
    }

    bool push(T&& item, size_t item_size) {
      std::unique_lock<std::mutex> mlock(_mutex);
      if (_event_counter_status == events_counter_status::ENABLE) {
        ++_event_index;
        item.set_event_index(_event_index);
      }
      // If subsampling rate is < 1, then run subsampling logic
      if (_subsample_rate < 1) {
        if (item.try_drop(_subsample_rate, constants::SUBSAMPLE_RATE_DROP_PASS)) {
          // If the event is dropped, just get out of here
          return false;
        }
      }
      _capacity += item_size;
      _queue.push_back({ std::forward<T>(item),item_size });
      return true;
    }

    void prune(float pass_prob)
    {
      std::unique_lock<std::mutex> mlock(_mutex);
      if (!is_full()) return;
      for (auto it = _queue.begin(); it != _queue.end();) {
        it = it->first.try_drop(pass_prob, _drop_pass) ? erase(it) : (++it);
      }
      ++_drop_pass;
    }

    //approximate size
    size_t size()
    {
      std::unique_lock<std::mutex> mlock(_mutex);
      return _queue.size();
    }

    bool is_full() const {
      return capacity() >= _max_capacity;
    }

    size_t capacity() const 
    {
      return _capacity;
    }

  private:
    //thread-unsafe
    iterator_t erase(iterator_t it) {
      _capacity = (std::max)(0, static_cast<int>(_capacity) - static_cast<int>(it->second));
      return _queue.erase(it);
    }
  };
}

