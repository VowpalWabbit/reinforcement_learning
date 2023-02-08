#include <sstream>
#include <string>
#include <vector>

class prng
{
  uint64_t val;

public:
  prng(uint64_t initial_seed);
  uint64_t next_uint();
};

class cb_decision_gen
{
  int shared_features, action_features, actions_per_decision;
  std::vector<std::string> actions_set;
  prng rand;
  bool passthrough;

public:
  cb_decision_gen(int shared_features, int action_features, int actions_per_decision, int total_actions,
      int initial_seed, bool passthrough);

  std::string gen_example();
};

class ccb_decision_gen
{
  int shared_features_size;   // size of set of possible features to choose from
  int shared_features_count;  // actual number of features per example
  int action_features_size;
  int action_features_count;
  int actions_per_example;
  int slots_per_example;

  std::vector<std::string> actions_set;
  prng rand;

public:
  ccb_decision_gen(int shared_features_size, int shared_features_count, int action_features_size,
      int action_features_count, int actions_per_example, int slots_per_example, int total_actions, int initial_seed);

  std::string gen_example();
};
