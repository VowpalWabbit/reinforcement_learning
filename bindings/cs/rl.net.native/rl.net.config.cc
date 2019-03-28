#include "rl.net.config.h"

API reinforcement_learning::utility::configuration* CreateConfig()
{
    return new reinforcement_learning::utility::configuration();
}

API void DeleteConfig(reinforcement_learning::utility::configuration* config)
{
    delete config;
}

API int LoadConfigurationFromJson(const int byte_length, const char* utf8_json, reinforcement_learning::utility::configuration* config, reinforcement_learning::api_status* status)
{
    // This is a deep copy, so it is safe to push a pinned-managed string here.
	// Note that we are passing the byte_length (code units) here, not the number of code points. 
	// This should work for the most part, potentially breaking on non-careful use of regex.
	// See: "UTF-8 in std::string" at https://stackoverflow.com/questions/50403342/how-do-i-properly-use-stdstring-on-utf-8-in-c#50407375
    const std::string utf8_json_str(utf8_json, byte_length);

    return reinforcement_learning::utility::config::create_from_json(utf8_json_str, *config, /* i_trace */ nullptr, status);
}

API void ConfigurationSet(reinforcement_learning::utility::configuration* instance, const char* name, const char* value)
{
    instance->set(name, value);
}

API const char* ConfigurationGet(reinforcement_learning::utility::configuration* instance, const char* name, const char* defVal)
{
    return instance->get(name, defVal);
}