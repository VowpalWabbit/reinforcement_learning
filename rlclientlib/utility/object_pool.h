#pragma once
#include "data_buffer.h"

#include <mutex>

namespace reinforcement_learning
{
namespace utility
{

template <typename Object>
class object_pool
{
public:
  std::shared_ptr<Object> acquire();
  void release(Object*);
  ~object_pool();

private:
  bool _pool_invalid = false;
  std::vector<Object*> _pool;
  std::mutex _mutex;
};

template <typename Object>
std::shared_ptr<Object> object_pool<Object>::acquire()
{
  std::lock_guard<std::mutex> lock(_mutex);
  Object* ptr;

  if (_pool.empty()) { ptr = new Object(); }
  else
  {
    ptr = _pool.back();
    _pool.pop_back();
    ptr->reset();
  }
  return std::shared_ptr<Object>(ptr, [this](Object* pobject) { this->release(pobject); });
}

template <typename Object>
void object_pool<Object>::release(Object* pobj)
{
  // In some test scenarios the pool is destroyed
  // before the object is released
  if (_pool_invalid) return;

  std::lock_guard<std::mutex> lock(_mutex);
  _pool.emplace_back(pobj);
}

template <typename Object>
object_pool<Object>::~object_pool()
{
  for (auto ptr : _pool) { delete ptr; }
  _pool_invalid = true;
}

}  // namespace utility
}  // namespace reinforcement_learning