#include <sstream>
#include <string>
#include <vector>

class prng {
  int val;

public:
  prng(int initial_seed);
  uint32_t next_uint();
};

class cb_decision_gen {
  int shared_features, action_features, actions_per_decision;
  std::vector<std::string> actions_set;
  prng rand;
  std::string temp_str;
  bool passthrough;

  std::string mk_feature_vector(int count, uint32_t max_idx);

public:
  cb_decision_gen(int shared_features, int action_features,
                  int actions_per_decision, int total_actions, int initial_seed,
                  bool passthrough);

  std::string gen_example();
};
