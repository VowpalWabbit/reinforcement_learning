#pragma once

#include "model_mgmt.h"
#include "configuration.h"
#include "vw/core/vw.h"
#include "trace_logger.h"
#include "api_status.h"

#include <memory>
#include <vector>

namespace reinforcement_learning
{

enum class input_serialization
{
  unknown,
  vwjson,
  dsjson,
  flatbuffer
};

class vw_input_type_configurator
{
public:
  static const vw_input_type_configurator& get_vw_model_input_adapter_factory();
  static const vw_input_type_configurator& get_pdf_model_input_adapter_factory();

public:
  input_serialization configure_input_serialization(const utility::configuration& config, i_trace* trace_logger, api_status* status) const;

private:
  template <size_t N>
  vw_input_type_configurator(const input_serialization(&allowed_formats)[N])
  {
    static_assert(N > 0, "Must have at least one allowed format");
    _allowed_format_count = N;
    _allowed_formats = new input_serialization[N];
    for (size_t i = 0; i < N; i++)
    {
      _allowed_formats[i] = allowed_formats[i];
    }
  }

  ~vw_input_type_configurator()
  {
    delete[] _allowed_formats;
  }

  size_t _allowed_format_count;
  input_serialization* _allowed_formats;
};

class safe_vw
{
  // we need to keep a reference to the master around, so it's still valid even if the factory is deleted
  std::shared_ptr<safe_vw> _master;
  VW::workspace* _vw;
  input_serialization _input_format;
  std::vector<VW::example*> _example_pool;

  VW::example* get_or_create_example();
  static VW::example& get_or_create_example_f(void* vw);
  void parse_context(string_view context, VW::multi_ex& examples);

public:
  safe_vw(std::shared_ptr<safe_vw> master);
  safe_vw(const char* model_data, size_t len, const std::string& vw_commandline, input_serialization input_format);
  safe_vw(const char* model_data, size_t len, input_serialization input_format);
  safe_vw(const std::string& vw_commandline, input_serialization input_format);

  ~safe_vw();

  void parse_context_with_pdf(string_view context, std::vector<int>& actions, std::vector<float>& scores);
  void rank(string_view context, std::vector<int>& actions, std::vector<float>& scores);
  void choose_continuous_action(string_view context, float& action, float& pdf_value);
  // Used for CCB
  void rank_decisions(const std::vector<const char*>& event_ids, string_view context,
      std::vector<std::vector<uint32_t>>& actions, std::vector<std::vector<float>>& scores);
  // Used for slates
  void rank_multi_slot_decisions(const char* event_id, const std::vector<std::string>& slot_ids, string_view context,
      std::vector<std::vector<uint32_t>>& actions, std::vector<std::vector<float>>& scores);

  const char* id() const;

  bool is_compatible(const std::string& args) const;
  bool is_CB_to_CCB_model_upgrade(const std::string& args) const;

  // audit data is not guaranteed after any subsequent safe_vw calls
  string_view get_audit_data() const;

  static model_management::model_type_t get_model_type(const std::string& args);
  static model_management::model_type_t get_model_type(const VW::config::options_i* args);

  friend class safe_vw_factory;

private:

  void init();
};

class safe_vw_factory
{
  model_management::model_data _master_data;
  std::string _command_line;
  input_serialization _input_format;

public:
  // model_data is copied and stored in the factory object.
  safe_vw_factory(std::string command_line, input_serialization input_format);
  safe_vw_factory(const model_management::model_data& master_data, input_serialization input_format);
  safe_vw_factory(const model_management::model_data&& master_data, input_serialization input_format);
  safe_vw_factory(const model_management::model_data& master_data, std::string command_line, input_serialization input_format);
  safe_vw_factory(const model_management::model_data&& master_data, std::string command_line, input_serialization input_format);

  safe_vw* operator()();
};
}  // namespace reinforcement_learning
