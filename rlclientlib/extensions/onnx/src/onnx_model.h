#pragma once
#include <string>

#include "model_mgmt.h"

#include <core/session/onnxruntime_cxx_api.h>

namespace reinforcement_learning {
  class i_trace;
}

namespace reinforcement_learning { namespace onnx {
  class onnx_model : public model_management::i_model {
  public:
    onnx_model(onnx_model&&) = delete;
    onnx_model& operator=(onnx_model&&) = delete;

  public:
    onnx_model(i_trace* trace_logger, const char* app_id, const char* output_name, bool use_unstructured_input);
    int update(const model_management::model_data& data, bool& model_ready, api_status* status = nullptr) override;
    int choose_rank(uint64_t rnd_seed, const char* features, std::vector<int>& action_ids, std::vector<float>& action_pdf, std::string& model_version, api_status* status = nullptr) override;

    int choose_continuous_action(const char* features, float& action, float& pdf_value, std::string& model_version, api_status* status = nullptr) override
    {
      return error_code::not_supported;
    }

    int request_decision(const std::vector<const char*>& event_ids, const char* features, std::vector<std::vector<uint32_t>>& actions_ids, std::vector<std::vector<float>>& action_pdfs, std::string& model_version, api_status* status = nullptr) override
    {
      return error_code::not_supported;
    }

    int request_multi_slot_decision(const char* event_id, const std::vector<std::string>& slot_ids, const char* features, std::vector<std::vector<uint32_t>>& actions_ids, std::vector<std::vector<float>>& action_pdfs, std::string& model_version, api_status* status = nullptr) override
    {
      return error_code::not_supported;
    }

    model_management::model_type_t model_type() const { return model_management::model_type_t::CB; }
      
  private:
    i_trace* _trace_logger;
    std::string _output_name;
    size_t _output_index;
    const bool _use_unstructured_input;

    Ort::Env _env;
    Ort::SessionOptions _session_options;

    std::shared_ptr<Ort::Session> _master_session;
  };
}}
