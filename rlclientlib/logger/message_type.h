#pragma once

#include <cstdint>

namespace reinforcement_learning { namespace logger {
  struct message_type {
    using const_int = const uint16_t;

    // WARNING !
    // Do not reuse message ids.  This can cause
    // incompatibility with services that exist in the system.
    //
    // Message ids should only be added to this list!

    // Flat buffer ranking event collection message
    static const_int UNKNOWN = 0;
    static const_int fb_ranking_event_collection = 1;                       // Deprecated. It is replaced by fb_ranking_learning_mode_event_collection = 9.
    static const_int fb_outcome_event_collection = 2;
    static const_int json_ranking_event_collection = 3;
    static const_int json_outcome_event_collection = 4;
    static const_int fb_outcome_event = 5;
    static const_int fb_interaction_event = 6;                              // Deprecated. It is replaced by fb_interaction_learning_mode_event = 10.
    static const_int fb_decision_event = 7;
    static const_int fb_decision_event_collection = 8;
    static const_int fb_ranking_learning_mode_event_collection = 9;
    static const_int fb_interaction_learning_mode_event = 10;
  };
}}
