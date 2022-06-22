#include "rl.net.continuous_action_response.h"

API reinforcement_learning::continuous_action_response* CreateContinuousActionResponse()
{
  return new reinforcement_learning::continuous_action_response();
}

API void DeleteContinuousActionResponse(reinforcement_learning::continuous_action_response* response)
{
  delete response;
}

API const char* GetContinuousActionEventId(reinforcement_learning::continuous_action_response* response)
{
  return response->get_event_id();
}

API const char* GetContinuousActionModelId(reinforcement_learning::continuous_action_response* response)
{
  return response->get_model_id();
}

API float GetContinuousActionChosenAction(reinforcement_learning::continuous_action_response* response)
{
  return response->get_chosen_action();
}

API float GetContinuousActionChosenActionPdfValue(reinforcement_learning::continuous_action_response* response)
{
  return response->get_chosen_action_pdf_value();
}