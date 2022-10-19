#include "benchmarks_common.h"

#include "vw/core/rand48.h"

#include <set>

prng::prng(uint64_t initial_seed) : val(merand48(initial_seed)) {}

uint64_t prng::next_uint()
{
  merand48(val);
  return val;
}

std::string cb_decision_gen::mk_feature_vector(int count, uint32_t max_idx)
{
  std::ostringstream str;
  str << "{";
  std::set<uint32_t> added_idx;
  int added = 0;
  while (added < count)
  {
    auto idx = rand.next_uint() % max_idx;
    if (added_idx.find(idx) == added_idx.end())
    {
      if (added > 0) str << ",";
      str << "\"" << idx << "_f\":1";

      ++added;
      added_idx.insert(idx);
    }
  }
  str << "}";
  return str.str();
}

cb_decision_gen::cb_decision_gen(int shared_features, int action_features, int actions_per_decision, int total_actions,
    int initial_seed, bool passthrough)
    : shared_features(shared_features)
    , action_features(action_features)
    , actions_per_decision(actions_per_decision)
    , rand(initial_seed)
    , passthrough(passthrough)
{
  for (int i = 0; i < total_actions; ++i)
  { actions_set.push_back(mk_feature_vector(action_features, action_features * 3)); }
}

std::string cb_decision_gen::gen_example()
{
  std::ostringstream str;
  str << R"({"shared":)";
  str << mk_feature_vector(shared_features, shared_features * 3) << ",";
  str << R"("_multi":[)";
  std::set<size_t> added_actions;
  int added = 0;
  while (added < actions_per_decision)
  {
    auto idx = rand.next_uint() % actions_set.size();
    if (added_actions.find(idx) == added_actions.end())
    {
      if (added > 0) str << ",";
      added_actions.insert(idx);

      str << R"({"action":)";
      str << actions_set[idx];
      str << "}";
      ++added;
    }
  }

  if (passthrough)
  {
    str << R"(])";
    str << R"(, "_p":[)";
    str << (1 / actions_per_decision);
    for (size_t i = 1; i < actions_per_decision; i++) { str << "," << (1 / actions_per_decision); }
    str << R"(]})";
  }
  else
  {
    str << R"(]})";
  }
  temp_str = str.str();

  return temp_str;
}
