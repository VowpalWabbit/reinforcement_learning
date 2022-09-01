#pragma once
#include <functional>
#include <mutex>

namespace reinforcement_learning
{
#define ERROR_CALLBACK(fn, status)                   \
  do                                                 \
  {                                                  \
    if (fn != nullptr) { fn->report_error(status); } \
  } while (0)

class api_status;

class error_callback_fn
{
public:
  using callback_t = std::function<void(const api_status&)>;
  using error_fn = void (*)(const api_status&, void*);
  void set(error_fn fn, void* context)
  {
    std::lock_guard<std::mutex> lock(_mutex);
    if (fn != nullptr) { _fn = std::bind(fn, std::placeholders::_1, context); }
    else
    {
      _fn = nullptr;
    }
  }

  template <typename ContextT>
  void set(void (*fn)(const api_status&, ContextT*), ContextT* context)
  {
    std::lock_guard<std::mutex> lock(_mutex);
    if (fn != nullptr) { _fn = std::bind(fn, std::placeholders::_1, context); }
    else
    {
      _fn = nullptr;
    }
  }

  void report_error(api_status& s);

  error_callback_fn(error_fn fn, void* context) { set(fn, context); }

  template <typename ContextT>
  error_callback_fn(void (*fn)(const api_status&, ContextT*), ContextT* context)
  {
    set<ContextT>(fn, context);
  }

  ~error_callback_fn() = default;

  error_callback_fn(const error_callback_fn&) = delete;
  error_callback_fn(error_callback_fn&&) = delete;
  error_callback_fn& operator=(const error_callback_fn&) = delete;
  error_callback_fn& operator=(error_callback_fn&&) = delete;

private:
  std::mutex _mutex;
  callback_t _fn;
};
}  // namespace reinforcement_learning
