#pragma once
#include "data_buffer.h"

#include <memory>
#include <mutex>
#include <type_traits>
#include <vector>

namespace reinforcement_learning
{
namespace utility
{
// Object data type must have a .reset() member function that resets
// the state of the object as if it was newly constructed
template <typename Object>
class object_pool : public std::enable_shared_from_this<object_pool<Object>>
{
public:
  // Factory function to create the object pool
  static std::shared_ptr<object_pool<Object>> create(size_t initial_size = 0)
  {
    return std::shared_ptr<object_pool<Object>>(new object_pool<Object>(initial_size));
  }

  // Get object from pool
  // Deleter of shared_ptr will return the object back into the pool
  std::shared_ptr<Object> acquire();

private:
  // Private constructor because std::enable_shared_from_this requires objects to be inside shared_ptr
  // Use the factory function to create object_pool
  object_pool(size_t initial_size) : _pool(initial_size)
  {
    static_assert(std::is_member_function_pointer<decltype(&Object::reset)>::value,
        "Object type for object_pool must implement .reset() function");
  }

  void release(std::unique_ptr<Object>);

  std::vector<std::unique_ptr<Object>> _pool;
  std::mutex _mutex;
};

template <typename Object>
std::shared_ptr<Object> object_pool<Object>::acquire()
{
  std::lock_guard<std::mutex> lock(_mutex);
  std::unique_ptr<Object> ptr;

  // Get object from pool or create new object
  if (_pool.empty()) { ptr.reset(new Object()); }
  else
  {
    ptr = std::move(_pool.back());
    _pool.pop_back();
    ptr->reset();
  }

  // Wrap in shared_ptr with custom deleter
  std::weak_ptr<object_pool<Object>> pool_weak_ptr = this->shared_from_this();
  return std::shared_ptr<Object>(ptr.release(),
      [pool_weak_ptr](Object* pobject)
      {
        // Store raw pointer in unique_ptr for exception safety
        std::unique_ptr<Object> obj(pobject);
        if (auto pool_ptr = pool_weak_ptr.lock())
        {
          // pool is valid, we can return the object
          pool_ptr->release(std::move(obj));
        }
        // else couldn't lock weak_ptr because pool has been destroyed already
        // obj will be destroyed by unique_ptr
      });
}

template <typename Object>
void object_pool<Object>::release(std::unique_ptr<Object> obj)
{
  std::lock_guard<std::mutex> lock(_mutex);
  _pool.push_back(std::move(obj));
}

}  // namespace utility
}  // namespace reinforcement_learning
