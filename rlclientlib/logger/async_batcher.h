#pragma once

#include "event_queue.h"
#include "api_status.h"
#include "constants.h"
#include "error_callback_fn.h"
#include "err_constants.h"
#include "data_buffer.h"
#include "utility/periodic_background_proc.h"

#include "serialization/fb_serializer.h"
#include "serialization/json_serializer.h"
#include "message_sender.h"
#include "utility/config_helper.h"
#include "utility/object_pool.h"

// float comparisons
#include "vw_math.h"

namespace reinforcement_learning {
  class error_callback_fn;
};

namespace reinforcement_learning { namespace logger {
  template<typename TEvent>
  class i_async_batcher {
  public:
    virtual ~i_async_batcher() = default;

    virtual int init(api_status* status) = 0;

    virtual int append(TEvent&& evt, api_status* status = nullptr) = 0;
    virtual int append(TEvent& evt, api_status* status = nullptr) = 0;

    virtual int run_iteration(api_status* status) = 0;
  };

  // This class takes uses a queue and a background thread to accumulate events, and send them by batch asynchronously.
  // A batch is shipped with TSender::send(data)
  template<typename TEvent, template<typename> class TSerializer = json_collection_serializer>
  class async_batcher: public i_async_batcher<TEvent> {
  public:
    using shared_state_t = typename TSerializer<TEvent>::shared_state_t;

    int init(api_status* status) override;

    int append(TEvent&& evt, api_status* status = nullptr) override;
    int append(TEvent& evt, api_status* status = nullptr) override;

    int run_iteration(api_status* status) override;

  private:
    int fill_buffer(std::shared_ptr<utility::data_buffer>& retbuffer,
      size_t& remaining,
      api_status* status,
      unsigned int* original_count);

    void flush(); //flush all batches
    void increment_counter();
    unsigned int reset_counter_get_count();

  public:
    async_batcher(i_message_sender* sender,
                  utility::watchdog& watchdog,
                  shared_state_t& shared_state,
                  error_callback_fn* perror_cb,
                  const utility::async_batcher_config& config);
    ~async_batcher();

  private:
    std::unique_ptr<i_message_sender> _sender;

    event_queue<TEvent> _queue;       // A queue to accumulate batch of events.
    size_t _send_high_water_mark;
    error_callback_fn* _perror_cb;
    shared_state_t& _shared_state;

    utility::periodic_background_proc<async_batcher> _periodic_background_proc;
    float _pass_prob;
    queue_mode_enum _queue_mode;
    std::condition_variable _cv;
    std::mutex _m;
    std::mutex _counter_mutex;
    utility::object_pool<utility::data_buffer> _buffer_pool;
    const char* _batch_content_encoding;
    float _subsample_rate;
    events_counter_status _events_counter_status;
    unsigned int _number_of_events = 0;
    unsigned int _buffer_end_event_number = 0;
  };

  template<typename TEvent, template<typename> class TSerializer>
  int async_batcher<TEvent, TSerializer>::init(api_status* status) {
    RETURN_IF_FAIL(_periodic_background_proc.init(this, status));
    bool subsample_lte_zero = _subsample_rate < 0.f || VW::math::are_same(_subsample_rate, 0.f);
    bool subsample_gt_one = _subsample_rate > 1.f && !VW::math::are_same(_subsample_rate, 1.f);
    if(subsample_lte_zero || subsample_gt_one) {
      // invalid subsample rate
      RETURN_ERROR_ARG(nullptr, status, invalid_argument, "subsampling rate must be within (0, 1]");
    }
    return error_code::success;
  }

  template<typename TEvent, template<typename> class TSerializer>
  int async_batcher<TEvent, TSerializer>::append(TEvent&& evt, api_status* status) {
    increment_counter();
    // If subsampling rate is < 1, then run subsampling logic
    if(_subsample_rate < 1) {
      if(evt.try_drop(_subsample_rate, constants::SUBSAMPLE_RATE_DROP_PASS)) {
        // If the event is dropped, just get out of here
        return error_code::success;
      }
    }
    
    if (_events_counter_status == events_counter_status::ENABLE) { evt.set_number_of_events(_number_of_events); }
    _queue.push(std::move(evt), TSerializer<TEvent>::serializer_t::size_estimate(evt));

    //block or drop events if the queue if full
    if (_queue.is_full()) {
      if (queue_mode_enum::BLOCK == _queue_mode) {
        std::unique_lock<std::mutex> lk(_m);
        _cv.wait(lk, [this] { return !_queue.is_full(); });
      }
      else if (queue_mode_enum::DROP == _queue_mode) {
        _queue.prune(_pass_prob);
      }
    }

    return error_code::success;
  }

  template<typename TEvent, template<typename> class TSerializer>
  int async_batcher<TEvent, TSerializer>::append(TEvent& evt, api_status* status) {
    return append(std::move(evt), status);
  }

  template<typename TEvent, template<typename> class TSerializer>
  int async_batcher<TEvent, TSerializer>::run_iteration(api_status* status) {
    flush();
    return error_code::success;
  }

  template<typename TEvent, template<typename> class TSerializer>
  int async_batcher<TEvent, TSerializer>::fill_buffer(
                                                      std::shared_ptr<utility::data_buffer>& buffer,
                                                      size_t& remaining,
                                                      api_status* status,
                                                      unsigned int* original_count)
  {
    TEvent evt;
    TSerializer<TEvent> collection_serializer(*buffer.get(), _batch_content_encoding, _shared_state);
    
    while (remaining > 0 && collection_serializer.size() < _send_high_water_mark) {
      if (_queue.pop(&evt)) {
        if (queue_mode_enum::BLOCK == _queue_mode) {
          _cv.notify_one();
        }
        RETURN_IF_FAIL(collection_serializer.add(evt, status));
        --remaining;
      }
    }

    RETURN_IF_FAIL(collection_serializer.finalize(status));

    if (_events_counter_status == events_counter_status::ENABLE) {
      unsigned int buffer_start_event_number;
      buffer_start_event_number = _buffer_end_event_number;
      _buffer_end_event_number = evt.get_number_of_events();
      *original_count = (_buffer_end_event_number - buffer_start_event_number);
      if (_queue.size() == 0) {
        //reset counter values if queue is empty to prevent overflow incase of too many events
        *original_count += reset_counter_get_count() - _buffer_end_event_number;
        buffer_start_event_number = 0;
        _buffer_end_event_number = 0;
      }
    }

    return error_code::success;
  }

  template<typename TEvent, template<typename> class TSerializer>
  void async_batcher<TEvent, TSerializer>::increment_counter()  {
    std::unique_lock<std::mutex> mlock(_counter_mutex);
    if (_events_counter_status == events_counter_status::ENABLE) { ++_number_of_events; }
  }

  template<typename TEvent, template<typename> class TSerializer>
  unsigned int async_batcher<TEvent, TSerializer>::reset_counter_get_count() {
    std::unique_lock<std::mutex> mlock(_counter_mutex);
    if (_events_counter_status == events_counter_status::ENABLE) { 
      unsigned int current_events_count = _number_of_events;
      _number_of_events = 0;
      return current_events_count;
    }
    return 0;
  }

  template<typename TEvent, template<typename> class TSerializer>
  void async_batcher<TEvent, TSerializer>::flush() {
    const auto queue_size = _queue.size();

    // Early exit if queue is empty.
    if (queue_size == 0) {
      return;
    }

    auto remaining = queue_size;
    // Handle batching
    while (remaining > 0) {
      api_status status;

      auto buffer = _buffer_pool.acquire();
      unsigned int original_count = 0;

      if (fill_buffer(buffer, remaining, &status, &original_count) != error_code::success) {
        ERROR_CALLBACK(_perror_cb, status);
      }
      if (original_count > 0 && _events_counter_status == events_counter_status::ENABLE) {
        if (_sender->send(TSerializer<TEvent>::message_id(), buffer, original_count, &status) != error_code::success) {
          ERROR_CALLBACK(_perror_cb, status);
        }
      }
      else {
        if (_sender->send(TSerializer<TEvent>::message_id(), buffer, &status) != error_code::success) {
          ERROR_CALLBACK(_perror_cb, status);
        }
      }
    }
  }

  template<typename TEvent, template<typename> class TSerializer>
  async_batcher<TEvent, TSerializer>::async_batcher(
    i_message_sender* sender,
    utility::watchdog& watchdog,
    typename TSerializer<TEvent>::shared_state_t& shared_state,
    error_callback_fn* perror_cb,
    const utility::async_batcher_config& config)
    : _sender(sender)
    , _queue(config.send_queue_max_capacity)
    , _send_high_water_mark(config.send_high_water_mark)
    , _perror_cb(perror_cb)
    , _shared_state(shared_state)
    , _periodic_background_proc(static_cast<int>(config.send_batch_interval_ms), watchdog, "Async batcher thread", perror_cb)
    , _pass_prob(0.5)
    , _queue_mode(config.queue_mode)
    , _batch_content_encoding(config.batch_content_encoding)
    , _subsample_rate(config.subsample_rate)
    , _events_counter_status(config.event_counter_status)
  {}

  template<typename TEvent, template<typename> class TSerializer>
  async_batcher<TEvent, TSerializer>::~async_batcher() {
    // Stop the background procedure the queue before exiting
    _periodic_background_proc.stop();
    if (_queue.size() > 0) {
      flush();
    }
  }
}}
