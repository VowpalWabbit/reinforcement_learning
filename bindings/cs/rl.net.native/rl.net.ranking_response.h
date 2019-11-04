#pragma once

#include "rl.net.native.h"

// TODO: Make the underlying iterator more ammenable to P/Invoke projection
class ranking_enumerator_adapter;

// Global exports
extern "C" {
    // NOTE: THIS IS NOT POLYMORPHISM SAFE!
    API reinforcement_learning::ranking_response* CreateRankingResponse();
    API const reinforcement_learning::ranking_response* InitRankingResponse(int chosen_action, const char* event_id, int* actions, float* probs, size_t actions_count, const char* model_id);

    API void DeleteRankingResponse(reinforcement_learning::ranking_response* ranking);

    // TODO: We should think about how to avoid extra string copies; ideally, err constants
    // should be able to be shared between native/managed, but not clear if this is possible
    // right now.
    API const char* GetRankingEventId(reinforcement_learning::ranking_response* ranking);
    API const char* GetRankingModelId(reinforcement_learning::ranking_response* ranking);
    API size_t GetRankingActionCount(reinforcement_learning::ranking_response* ranking);
    API int GetRankingChosenAction(reinforcement_learning::ranking_response* ranking, size_t* action_id, reinforcement_learning::api_status* status = nullptr);
    
    API void SetRankingEventId(reinforcement_learning::ranking_response* ranking, const char*);
    API void SetRankingModelId(reinforcement_learning::ranking_response* ranking, const char*);
    API void SetRankingChosenAction(reinforcement_learning::ranking_response* ranking, int);
    API void PushActionProbability(reinforcement_learning::ranking_response* ranking, int action, float prob);

    API ranking_enumerator_adapter* CreateRankingEnumeratorAdapter(reinforcement_learning::ranking_response* ranking);
    API void DeleteRankingEnumeratorAdapter(ranking_enumerator_adapter* adapter);

    API int RankingEnumeratorInit(ranking_enumerator_adapter* adapter);
    API int RankingEnumeratorMoveNext(ranking_enumerator_adapter* adapter);
    API reinforcement_learning::action_prob_d GetRankingEnumeratorCurrent(ranking_enumerator_adapter* adapter);
}