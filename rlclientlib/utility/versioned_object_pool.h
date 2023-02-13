#pragma once
#include "str_util.h"
#include "trace_logger.h"

#include <functional>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

namespace reinforcement_learning
{
namespace utility
{
template <typename TObject>
class versioned_object_pool_unsafe
{
  using TFactory = std::function<TObject*(void)>;

  int _version;
  TFactory _factory;
  int _objects_count;
  std::vector<std::unique_ptr<TObject>> _pool;

public:
  // Construct object pool given a factory function that allocates new objects when called
  // Optionally, pre-populate the pool with a given count of objects
  versioned_object_pool_unsafe(TFactory factory, int objects_count = 0, int version = 0)
      : _version(version), _factory(std::move(factory)), _objects_count(objects_count)
  {
    _pool.reserve(_objects_count);
    for (int i = 0; i < _objects_count; ++i) { _pool.emplace_back(_factory()); }
  }

  ~versioned_object_pool_unsafe() = default;

  versioned_object_pool_unsafe(const versioned_object_pool_unsafe&) = delete;
  versioned_object_pool_unsafe& operator=(const versioned_object_pool_unsafe& other) = delete;
  versioned_object_pool_unsafe(versioned_object_pool_unsafe&& other) = delete;

  // Retreive an object in the pool, or nullptr if pool is empty
  // The object must be returned to the pool along with its version, or else allocated memory will not be freed
  TObject* get()
  {
    if (_pool.empty()) { return nullptr; }

    auto obj = std::move(_pool.back());
    _pool.pop_back();
    return obj.release();
  }

  // Create a new object with the pool's factory function
  // The object must be returned to the pool along with its version, or else allocated memory will not be freed
  TObject* create()
  {
    _objects_count++;
    return _factory();
  }

  // Put the object into the pool if version matches
  // Otherwise, we deallocate it
  void return_to_pool(TObject* obj, int obj_version)
  {
    std::unique_ptr<TObject> pobj(obj);
    if (_version == obj_version) { _pool.push_back(std::move(pobj)); }
    // else unique_ptr will delete obj
  }

  int size() const { return _objects_count; }
  int version() const { return _version; }

  // Get a reference to the internal factory std::function
  const TFactory& get_factory_function() const { return _factory; }
};

template <typename TObject>
class versioned_object_pool
{
  using TFactory = std::function<TObject*(void)>;
  using TObjectDeleter = std::function<void(TObject*)>;
  using impl_type = versioned_object_pool_unsafe<TObject>;
  std::mutex _mutex;
  std::unique_ptr<impl_type> _impl;
  i_trace* _trace_logger = nullptr;

public:
  // Construct object pool given a factory function that allocates new objects when called
  // Optionally, pre-populate the pool with a given count of objects
  versioned_object_pool(TFactory factory, int init_size = 0, i_trace* trace_logger = nullptr)
      : _impl(new impl_type(std::move(factory), init_size, 0)), _trace_logger(trace_logger)
  {
  }

  versioned_object_pool(const versioned_object_pool&) = delete;
  versioned_object_pool& operator=(const versioned_object_pool& other) = delete;
  versioned_object_pool(versioned_object_pool&& other) = delete;

  ~versioned_object_pool()
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _impl.reset();
  }

  // Retrieve an object from the pool, creating a new one if the pool is empty
  // The object is returned to pool when its std::unique_ptr is destroyed
  std::unique_ptr<TObject, TObjectDeleter> get_or_create()
  {
    std::lock_guard<std::mutex> lock(_mutex);
    // in the deleter function, capture a copy of the version at time of object creation
    int current_version = _impl->version();

    // try to get an existing object
    TObject* pool_obj = _impl->get();
    if (pool_obj != nullptr)
    {
      TRACE_INFO(_trace_logger,
          utility::concat(
              "versioned_object_pool::get_or_create() called: existing object returned, total pool size is ",
              _impl->size()));
    }
    else
    {
      // pool was empty, create a new object
      pool_obj = _impl->create();
      TRACE_INFO(_trace_logger,
          utility::concat(
              "versioned_object_pool::get_or_create() called: new object created, total pool size is ", _impl->size()));
    }

#if __cplusplus >= 202002L
    // C++20 deprecates implicit "this" capture by reference in [=] lambda
    return std::unique_ptr<TObject, TObjectDeleter>(
        pool_obj, [=, this](TObject* obj) { this->return_to_pool(obj, current_version); });
#else
    return std::unique_ptr<TObject, TObjectDeleter>(
        pool_obj, [=](TObject* obj) { this->return_to_pool(obj, current_version); });
#endif
  }

  // Update the pool's factory function and increment version number
  void update_factory(TFactory new_factory)
  {
    int objects_count = 0;
    int new_version = 0;

    std::lock_guard<std::mutex> lock(_mutex);
    objects_count = _impl->size();
    new_version = _impl->version() + 1;

    TRACE_INFO(
        _trace_logger, utility::concat("versioned_object_pool::update_factory() called: pool size is ", objects_count));

    std::unique_ptr<impl_type> new_impl(new impl_type(std::move(new_factory), objects_count, new_version));
    _impl.swap(new_impl);
  }

  // Get a reference to the internal factory std::function
  const TFactory& get_factory_function() const { return _impl->get_factory_function(); }

private:
  void return_to_pool(TObject* obj, int obj_version)
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _impl->return_to_pool(obj, obj_version);
  }
};
}  // namespace utility
}  // namespace reinforcement_learning
