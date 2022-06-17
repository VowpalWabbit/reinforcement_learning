#include "lru_dedup_cache.h"

void lru_dedup_cache::add(uint64_t dedup_id, example* ex)
{
  dedup_examples.emplace(dedup_id, ex);
  lru.push_front(dedup_id);
  lru_pos.emplace(dedup_id, lru.begin());
}

void lru_dedup_cache::update(uint64_t dedup_id)
{
  // existing move to front
  auto position = lru_pos[dedup_id];
  lru.erase(position);
  lru.push_front(dedup_id);
  lru_pos[dedup_id] = lru.begin();
}

void lru_dedup_cache::clear_after(uint64_t first_id, release_example_f release_example, void* context)
{
  // erase the rest
  auto iter = lru_pos[first_id];
  // point to the element right after
  iter++;
  auto first_pos = iter;
  while (iter != lru.end())
  {
    auto dedup_id = *iter;
    lru_pos.erase(dedup_id);
    release_example(context, dedup_examples[dedup_id]);
    dedup_examples.erase(dedup_id);
    iter++;
  }
  lru.erase(first_pos, lru.end());
}

void lru_dedup_cache::clear(release_example_f release_example, void* context)
{
  for (auto& dedup_item : dedup_examples) { release_example(context, dedup_item.second); }
  dedup_examples.clear();
  lru_pos.clear();
  lru.clear();
}

bool lru_dedup_cache::exists(uint64_t dedup_id) { return dedup_examples.find(dedup_id) != dedup_examples.end(); }