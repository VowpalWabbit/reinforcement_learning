#pragma once

#include "vw/core/example.h"

#include <list>
#include <unordered_map>

/*
LRU dedup cache
When a new dedup payload is deserialized, the examples that were not found in
the new dedup payload will have moved to the end of the lru list.
If we call clear_after with the first example of the new dedup payload we can
assume that anything after that can be evicted as it was not in the new dedup
payload. If two dedup payloads are identical then nothing will be evicted.

Assumption: dedup payloads are dictionaries and so they have unique items
*/
struct lru_dedup_cache {
  // from dictionary id to example object
  // right now holding one dedup dictionary at a time, could be exented to a
  // map of maps holding more than one dedup dictionaries at a time
  std::unordered_map<uint64_t, example *> dedup_examples;
  std::list<uint64_t> lru;
  using list_iterator = std::list<uint64_t>::iterator;
  std::unordered_map<uint64_t, list_iterator> lru_pos;

  using release_example_f = void (*)(void *, example *);
  static void noop_release_example_f(void *, example *) { return; }

public:
  void add(uint64_t dedup_id, example *ex);
  void update(uint64_t dedup_id);
  void clear_after(uint64_t dedup_id,
                   release_example_f release_example =
                       lru_dedup_cache::noop_release_example_f,
                   void *context = nullptr);
  bool exists(uint64_t dedup_id);
  void clear(release_example_f release_example =
                 lru_dedup_cache::noop_release_example_f,
             void *context = nullptr);

  lru_dedup_cache() = default;
  ~lru_dedup_cache() = default;
  lru_dedup_cache(const lru_dedup_cache &) = delete;
  lru_dedup_cache(lru_dedup_cache &&) = delete;
  lru_dedup_cache &operator=(const lru_dedup_cache &) = delete;
  lru_dedup_cache &operator=(lru_dedup_cache &&) = delete;
};