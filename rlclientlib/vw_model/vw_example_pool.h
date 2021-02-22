#pragma once

#include "model_mgmt.h"
#include "vw.h"
#include <memory>
#include <vector>

namespace reinforcement_learning {

class vw_example_pool {
private:
  vw* _vw;
  std::vector<example *> _example_pool;

public:
  vw_example_pool();
  vw_example_pool(vw* vw);
  ~vw_example_pool();

  vw *get_vw() const;
  void set_vw(vw* vw);
  example *get_or_create_example();
  void return_example(example *);
};
} // namespace reinforcement_learning