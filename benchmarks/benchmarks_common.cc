#include "benchmarks_common.h"

#include "vw/common/random.h"

#include <set>

namespace
{
// Helper functions
std::string make_feature_vector(int count, uint32_t max_idx, prng& rand)
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

std::string make_actions_vector(int count, const std::vector<std::string>& actions_set, prng& rand)
{
  std::ostringstream str;
  std::set<size_t> added_actions;
  int added = 0;
  while (added < count)
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
  return str.str();
}
}  // namespace

prng::prng(uint64_t initial_seed) : val(VW::details::merand48(initial_seed)) {}

uint64_t prng::next_uint()
{
  VW::details::merand48(val);
  return val;
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
  {
    actions_set.push_back(make_feature_vector(action_features, action_features * 3, rand));
  }
}

std::string cb_decision_gen::gen_example()
{
  std::ostringstream str;
  str << "{";

  str << R"("shared":)";
  str << make_feature_vector(shared_features, shared_features * 3, rand);
  str << ",";

  str << R"("_multi":[)";
  str << make_actions_vector(actions_per_decision, actions_set, rand);
  str << "]";

  if (passthrough)
  {
    str << R"(, "_p":[)";
    str << (1 / actions_per_decision);
    for (size_t i = 1; i < actions_per_decision; i++) { str << "," << (1 / actions_per_decision); }
    str << "]";
  }

  str << "}";
  return str.str();
}

ccb_decision_gen::ccb_decision_gen(int shared_features_size, int shared_features_count, int action_features_size,
    int action_features_count, int actions_per_example, int slots_per_example, int total_actions, int initial_seed)
    : shared_features_size(shared_features_size)
    , shared_features_count(shared_features_count)
    , action_features_size(action_features_size)
    , action_features_count(action_features_count)
    , actions_per_example(actions_per_example)
    , slots_per_example(slots_per_example)
    , rand(initial_seed)
{
  for (int i = 0; i < total_actions; ++i)
  {
    actions_set.push_back(make_feature_vector(action_features_count, action_features_size, rand));
  }
}

std::string ccb_decision_gen::gen_example()
{
  std::ostringstream str;
  str << "{";

  str << R"("shared":)";
  str << make_feature_vector(shared_features_count, shared_features_size, rand);
  str << ",";

  str << R"("_multi":[)";
  str << make_actions_vector(actions_per_example, actions_set, rand);
  str << "],";

  str << R"("_slots":[)";
  for (int i = 0; i < slots_per_example; i++)
  {
    if (i > 0) { str << ","; }
    str << make_feature_vector(action_features_count, action_features_size, rand);
  }
  str << "]";

  str << "}";
  return str.str();
}
