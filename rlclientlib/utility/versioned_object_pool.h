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
  std::vector<TObject*> _pool;
  TFactory _factory;
  int _used_objects;
  int _objects_count;

public:
  // Construct object pool given a factory function that allocates new objects when called
  // Optionally, pre-populate the pool with a given count of objects
  versioned_object_pool_unsafe(TFactory factory, int objects_count = 0, int version = 0)
      : _version(version), _factory(std::move(factory)), _used_objects(0), _objects_count(objects_count)
  {
    for (int i = 0; i < _objects_count; ++i) { 
      _pool.emplace_back(_factory());
    }
  }

  versioned_object_pool_unsafe(const versioned_object_pool_unsafe&) = delete;
  versioned_object_pool_unsafe& operator=(const versioned_object_pool_unsafe& other) = delete;
  versioned_object_pool_unsafe(versioned_object_pool_unsafe&& other) = delete;

  ~versioned_object_pool_unsafe()
  {
    // delete each pool object
    for (auto&& obj : _pool) {
      delete obj;
      obj = nullptr;
    }

    // clear the pool vector itself
    _pool.clear();
  }

  // Retreive an object in the pool, creating a new one if the pool is empty
  // The object must be returned to the pool along with its version, or else allocated memory will not be freed
  TObject* get_or_create()
  {
    if (_pool.size() == 0)
    {
      _used_objects++;
      _objects_count++;
      return _factory();
    }

    auto back = _pool.back();
    _pool.pop_back();

    return back;
  }

  // Put the object into the pool if version matches
  // Otherwise, we deallocate it
  void return_to_pool(TObject* obj, int obj_version)
  {
    if (_version == obj_version)
    {
      _pool.emplace_back(obj);
    }
    else
    {
      delete obj;
      obj = nullptr;
    }
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
    : _impl(new impl_type(std::move(factory), init_size, 0)), _trace_logger(trace_logger) { }

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
    auto pool_obj = _impl->get_or_create();
    int objects_count = _impl->size();

    TRACE_DEBUG(_trace_logger, utility::concat("versioned_object_pool::get_or_create() called: pool size is ", objects_count));

    return std::unique_ptr<TObject, TObjectDeleter>(pool_obj, [=](TObject* obj) {
      this->return_to_pool(obj, current_version);
    });
  }

  // Update the pool's factory function and increment version number
  void update_factory(TFactory new_factory)
  {
    int objects_count = 0;
    int new_version = 0;

    std::lock_guard<std::mutex> lock(_mutex);
    objects_count = _impl->size();
    new_version = _impl->version() + 1;

    TRACE_DEBUG(_trace_logger, utility::concat("versioned_object_pool::update_factory() called: pool size is ", objects_count));

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
