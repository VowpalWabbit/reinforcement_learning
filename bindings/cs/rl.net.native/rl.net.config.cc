#include "rl.net.config.h"

API reinforcement_learning::utility::configuration* CreateConfig()
{
    return new reinforcement_learning::utility::configuration();
}

API void DeleteConfig(reinforcement_learning::utility::configuration* config)
{
    delete config;
}

API int LoadConfigurationFromJson(const int length, const char* json, reinforcement_learning::utility::configuration* config, reinforcement_learning::api_status* status)
{
    // This is a deep copy, so it is safe to push a pinned-managed string here.
    const std::string json_str(json, length);

    return reinforcement_learning::utility::config::create_from_json(json_str, *config, /* i_trace */ nullptr, status);
}

API void ConfigurationSet(reinforcement_learning::utility::configuration* instance, const char* name, const char* value)
{
    instance->set(name, value);
}

API const char* ConfigurationGet(reinforcement_learning::utility::configuration* instance, const char* name, const char* defVal)
{
    return instance->get(name, defVal);
}