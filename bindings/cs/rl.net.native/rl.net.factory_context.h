#pragma once

#include "rl.net.native.h"
#include "factory_resolver.h"

#include "binding_sender.h"

typedef struct factory_context {
  reinforcement_learning::trace_logger_factory_t* trace_logger_factory;
  reinforcement_learning::time_provider_factory_t* time_provider_factory;
  reinforcement_learning::sender_factory_t* sender_factory;
  reinforcement_learning::data_transport_factory_t* data_transport_factory;
  reinforcement_learning::model_factory_t* model_factory;
} factory_context_t;

extern "C" {
  API factory_context_t* CreateFactoryContext();
  API void DeleteFactoryContext(factory_context_t* context);

  API void SetFactoryContextBindingSenderFactory(
    factory_context_t* context,
    rl_net_native::sender_create_fn create_fn, 
    rl_net_native::sender_vtable_t vtable);
}
