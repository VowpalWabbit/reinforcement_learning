#include "learning_mode.h"
#include "constants.h"
#include <assert.h>

int main()
{
  const char* apprentice = "apprentice";
  auto mode = reinforcement_learning::learning::to_learning_mode(apprentice);
  assert(mode == reinforcement_learning::APPRENTICE);
}