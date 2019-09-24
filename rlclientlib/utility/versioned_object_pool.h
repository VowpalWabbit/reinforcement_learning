#pragma once
#include <algorithm>
#include <mutex>
#include <vector>

namespace reinforcement_learning { namespace utility {
  template<typename TObject>
  class pooled_object {
  private:
    TObject * _val;

  public:
    pooled_object(TObject* obj, int pversion)
      : _val(obj), version(pversion)
    { }

    pooled_object(const pooled_object&) = delete;
    pooled_object& operator=(const pooled_object& other) = delete;
    pooled_object(pooled_object&& other) = delete;

    ~pooled_object()
    {
      delete _val;
      _val = nullptr;
    }

    inline TObject* val() { return _val; }

    const int version;
  };

  template<typename TObject, typename TFactory>
  class versioned_object_pool;

  // RAII guard to handle exception case properly
  template<typename TObject, typename TFactory>
  class pooled_object_guard {
    versioned_object_pool<TObject, TFactory>* _pool;
    pooled_object<TObject>* _obj;

  public:
    pooled_object_guard(versioned_object_pool<TObject, TFactory>& pool, pooled_object<TObject>* obj)
      : _pool(&pool), _obj(obj)
    { }

    pooled_object_guard(const pooled_object_guard&) = delete;
    pooled_object_guard& operator=(const pooled_object_guard& other) = delete;
    pooled_object_guard(pooled_object_guard&& other) = delete;

    ~pooled_object_guard() {
      _pool->return_to_pool(_obj);
    }

    TObject* operator->() { return _obj->val(); }
    TObject* get() { return _obj->val(); }
  };

  template<typename TObject, typename TFactory>
  class versioned_object_pool_unsafe {
    int _version;
    std::vector<pooled_object<TObject>*> _pool;
    TFactory* _factory;
    int _used_objects;
    int _objects_count;

  public:
    versioned_object_pool_unsafe(TFactory* factory, int objects_count, int version)
      : _version(version)
      , _factory(factory)
      , _used_objects(0)
      , _objects_count(objects_count)
    {
      for (int i = 0; i < _objects_count; ++i) {
        _pool.emplace_back(new pooled_object<TObject>((*_factory)(), _version));
      }
    }

    versioned_object_pool_unsafe(TFactory* factory)
      : versioned_object_pool_unsafe(factory, 0, 0)
    { }

    versioned_object_pool_unsafe(const versioned_object_pool_unsafe&) = delete;
    versioned_object_pool_unsafe& operator=(const versioned_object_pool_unsafe& other) = delete;
    versioned_object_pool_unsafe(versioned_object_pool_unsafe&& other) = delete;

    ~versioned_object_pool_unsafe() {
      // delete factory
      delete _factory;
      _factory = nullptr;

      // delete each pool object
      for (auto&& obj : _pool)
        delete obj;
      _pool.clear();
    }

    pooled_object<TObject>* get_or_create() {
      if (_pool.size() == 0) {
        _used_objects++;
        _objects_count++;
        return new pooled_object<TObject>((*_factory)(), _version);
      }

      auto back = _pool.back();
      _pool.pop_back();

      return back;
    }

    void return_to_pool(pooled_object<TObject>* obj) {
      if (_version == obj->version) {
        _pool.emplace_back(obj);
        return;
      }

      delete obj;
    }

    int size() const {
      return _objects_count;
    }

    int version() const {
      return _version;
    }
  };

  template<typename TObject, typename TFactory>
  class versioned_object_pool {
    std::mutex _mutex;
    using impl_type = versioned_object_pool_unsafe<TObject, TFactory>;
    std::unique_ptr<impl_type> _impl;

  public:
    versioned_object_pool(TFactory* factory)
    : _impl(new impl_type(factory))
    { }

    versioned_object_pool(const versioned_object_pool&) = delete;
    versioned_object_pool& operator=(const versioned_object_pool& other) = delete;
    versioned_object_pool(versioned_object_pool&& other) = delete;

    ~versioned_object_pool() {
      std::lock_guard<std::mutex> lock(_mutex);
      _impl.reset();
    }

    pooled_object<TObject>* get_or_create() {
      std::lock_guard<std::mutex> lock(_mutex);
      return _impl->get_or_create();
    }

    void return_to_pool(pooled_object<TObject>* obj) {
      std::lock_guard<std::mutex> lock(_mutex);
      _impl->return_to_pool(obj);
    }

    // takes owner-ship of factory (and will free using delete) - !!!!THREAD-UNSAFE!!!!
    void update_factory(TFactory* new_factory) {
      int objects_count = 0;
      int version = 0;
      {
        std::lock_guard<std::mutex> lock(_mutex);
        objects_count = (std::max)(_impl->size(), _init_size);
        version = _impl->version() + 1;
      }
      std::unique_ptr<impl_type> new_impl(new impl_type(new_factory, objects_count, version));
      std::lock_guard<std::mutex> lock(_mutex);
      _impl.swap(new_impl);
    }
  };
}}
