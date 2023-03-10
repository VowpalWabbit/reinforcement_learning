#pragma once

#include "lru_dedup_cache.h"
#include "model_mgmt.h"
#include "vw/core/vw.h"

#include <memory>
#include <vector>

namespace reinforcement_learning
{
class safe_vw
{
  // we need to keep a reference to the master around, so it's still valid even if the factory is deleted
  std::shared_ptr<safe_vw> _master;
  VW::workspace* _vw;
  std::vector<VW::example*> _example_pool;

  VW::example* get_or_create_example();
  static VW::example& get_or_create_example_f(void* vw);

public:
  safe_vw(std::shared_ptr<safe_vw> master);
  safe_vw(const char* model_data, size_t len, const std::string& vw_commandline);
  safe_vw(const char* model_data, size_t len);
  safe_vw(const std::string& vw_commandline);

  ~safe_vw();

  void parse_context_with_pdf(string_view context, std::vector<int>& actions, std::vector<float>& scores);
  void load_action(uint64_t action_id, std::string action_str, lru_dedup_cache* action_cache);
  void rank(string_view context, std::vector<int>& actions, std::vector<float>& scores, lru_dedup_cache* action_cache = nullptr);
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

public:
  // model_data is copied and stored in the factory object.
  safe_vw_factory(std::string command_line);
  safe_vw_factory(const model_management::model_data& master_data);
  safe_vw_factory(const model_management::model_data&& master_data);
  safe_vw_factory(
      const model_management::model_data& master_data, std::string command_line);
  safe_vw_factory(
      const model_management::model_data&& master_data, std::string command_line);

  safe_vw* operator()();
};
}  // namespace reinforcement_learning
