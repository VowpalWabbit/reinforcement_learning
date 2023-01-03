#pragma once
#include "data_buffer.h"

#include <memory>
#include <mutex>
#include <vector>

namespace reinforcement_learning
{
namespace utility
{
template <typename Object>
class object_pool : public std::enable_shared_from_this<object_pool<Object>>
{
public:
  // Factory function to create the object pool
  static std::shared_ptr<object_pool<Object>> create(size_t initial_size = 0)
  {
    return std::shared_ptr<object_pool<Object>>(new object_pool<Object>(initial_size));
  }

  // Get an object from the pool, or allocate a new object if pool is empty
  // The shared_ptr will have a custom deleter that returns the object back into pool
  std::shared_ptr<Object> acquire();

private:
  // Private constructor because std::enable_shared_from_this requires objects to be inside shared_ptr
  // Use the factory function to create object_pool
  object_pool(size_t initial_size) : _pool(initial_size) {}

  void return_to_pool(std::unique_ptr<Object>);

  std::vector<std::unique_ptr<Object>> _pool;
  std::mutex _mutex;

  struct return_to_pool_deleter
  {
    return_to_pool_deleter(std::weak_ptr<object_pool<Object>> pool) : _pool(pool) {}

    void operator()(Object* pobject)
    {
      // wrap in unique_ptr so it's destroyed upon exception
      std::unique_ptr<Object> obj(pobject);

      // if pool is still valid, return the object to pool
      if (auto pool = _pool.lock())
      {
        try
        {
          pool->return_to_pool(std::move(obj));
        }
        catch (...)
        {
        }
      }
      // else pool has been deleted, obj will be destroyed by unique_ptr
    }

    std::weak_ptr<object_pool<Object>> _pool;
  };
};

template <typename Object>
std::shared_ptr<Object> object_pool<Object>::acquire()
{
  std::lock_guard<std::mutex> lock(_mutex);

  if (_pool.empty()) { return std::shared_ptr<Object>(new Object(), return_to_pool_deleter(this->shared_from_this())); }

  std::unique_ptr<Object> obj = std::move(_pool.back());
  _pool.pop_back();
  obj->reset();
  return std::shared_ptr<Object>(obj.release(), return_to_pool_deleter(this->shared_from_this()));
}

template <typename Object>
void object_pool<Object>::return_to_pool(std::unique_ptr<Object> obj)
{
  std::lock_guard<std::mutex> lock(_mutex);
  _pool.push_back(std::move(obj));
}

}  // namespace utility
}  // namespace reinforcement_learning