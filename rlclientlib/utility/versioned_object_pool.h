#pragma once
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

namespace reinforcement_learning
{
namespace utility
{
template <typename TObject, typename TFactory>
class versioned_object_pool_unsafe
{
  int _version;
  std::vector<TObject*> _pool;
  TFactory* _factory;
  int _used_objects;
  int _objects_count;

public:
  versioned_object_pool_unsafe(TFactory* factory, int objects_count, int version)
      : _version(version), _factory(factory), _used_objects(0), _objects_count(objects_count)
  {
    if (factory != nullptr)
    {
      for (int i = 0; i < _objects_count; ++i) { 
        _pool.emplace_back((*_factory)());
      }
    }
  }

  versioned_object_pool_unsafe(TFactory* factory) : versioned_object_pool_unsafe(factory, 0, 0) {}

  versioned_object_pool_unsafe(const versioned_object_pool_unsafe&) = delete;
  versioned_object_pool_unsafe& operator=(const versioned_object_pool_unsafe& other) = delete;
  versioned_object_pool_unsafe(versioned_object_pool_unsafe&& other) = delete;

  ~versioned_object_pool_unsafe()
  {
    // delete factory
    delete _factory;
    _factory = nullptr;

    // delete each pool object
    for (auto&& obj : _pool) {
      delete obj;
      obj = nullptr;
    }

    // clear the pool vector itself
    _pool.clear();
  }

  // Retreive an object in the pool, creating a new one if the pool is empty
  // The object must be returned to the pool, or else allocated memory will not be freed
  TObject* get_or_create()
  {
    if (_pool.size() == 0)
    {
      _used_objects++;
      _objects_count++;
      return (*_factory)();
    }

    auto back = _pool.back();
    _pool.pop_back();

    return back;
  }

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
};

template <typename TObject, typename TFactory>
class versioned_object_pool
{
  using impl_type = versioned_object_pool_unsafe<TObject, TFactory>;
  std::mutex _mutex;
  std::unique_ptr<impl_type> _impl;

public:
  versioned_object_pool(TFactory* factory, int init_size = 0) : _impl(new impl_type(factory, init_size, 0)) {}

  versioned_object_pool(const versioned_object_pool&) = delete;
  versioned_object_pool& operator=(const versioned_object_pool& other) = delete;
  versioned_object_pool(versioned_object_pool&& other) = delete;

  ~versioned_object_pool()
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _impl.reset();
  }

  std::unique_ptr<TObject, std::function<void(TObject*)>> get_or_create()
  {
    std::lock_guard<std::mutex> lock(_mutex);
    // in the deleter function, capture a copy of the version at time of object creation
    int current_version = _impl->version();
    return std::unique_ptr<TObject, std::function<void(TObject*)>>(_impl->get_or_create(), [=](TObject* obj) {
      this->return_to_pool(obj, current_version);
    });
  }

  // takes owner-ship of factory (and will free using delete) - !!!!THREAD-UNSAFE!!!!
  void update_factory(TFactory* new_factory)
  {
    int objects_count = 0;
    int version = 0;
    {
      std::lock_guard<std::mutex> lock(_mutex);
      objects_count = _impl->size();
      version = _impl->version() + 1;
    }
    std::unique_ptr<impl_type> new_impl(new impl_type(new_factory, objects_count, version));
    std::lock_guard<std::mutex> lock(_mutex);
    _impl.swap(new_impl);
  }

private:
  void return_to_pool(TObject* obj, int obj_version)
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _impl->return_to_pool(obj, obj_version);
  }
};
}  // namespace utility
}  // namespace reinforcement_learning
