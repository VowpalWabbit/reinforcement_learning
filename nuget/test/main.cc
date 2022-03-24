#include "ranking_response.h"
#include "explore.h"
#include <assert.h>

const int NUM_ACTIONS = 10;
namespace e = exploration;
namespace r = reinforcement_learning;

int main()
{
  const float epsilon = 0.2f;
  const auto top_action_id = 0;
  float pdf[NUM_ACTIONS];
  auto scode = e::generate_epsilon_greedy(epsilon, top_action_id, pdf, pdf + NUM_ACTIONS);
  assert(scode == 0);
  uint32_t chosen_index;
  scode = e::sample_after_normalizing(7791, pdf, pdf + NUM_ACTIONS, chosen_index);
  assert(scode == 0);
}