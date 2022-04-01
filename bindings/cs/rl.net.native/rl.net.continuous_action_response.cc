#include "rl.net.continuous_action_response.h"

API reinforcement_learning::continuous_action_response* CreateContinuousActionResponse()
{
    return new reinforcement_learning::continuous_action_response();
}

API void DeleteContinuousActionResponse(reinforcement_learning::continuous_action_response* response)
{
    delete response;
}

API const char* GetContinuousActionEventId(reinforcement_learning::continuous_action_response* response, int& event_id_size)
{
    const auto event_id = response->get_event_id();
    event_id_size = static_cast<int>(event_id.size());
    return event_id.data();
}

API const char* GetContinuousActionModelId(reinforcement_learning::continuous_action_response* response, int& model_id_size)
{
    const auto model_id = response->get_model_id();
    model_id_size = static_cast<int>(model_id.size());
    return model_id.data();
}

API float GetContinuousActionChosenAction(reinforcement_learning::continuous_action_response* response)
{
    return response->get_chosen_action();
}

API float GetContinuousActionChosenActionPdfValue(reinforcement_learning::continuous_action_response* response)
{
    return response->get_chosen_action_pdf_value();
}