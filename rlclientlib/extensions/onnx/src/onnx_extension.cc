#include "onnx_extension.h"

#include "api_status.h"
#include "configuration.h"
#include "constants.h"
#include "err_constants.h"
#include "factory_resolver.h"
#include "future_compat.h"
#include "model_mgmt.h"
#include "onnx_model.h"

namespace m = reinforcement_learning::model_management;
namespace u = reinforcement_learning::utility;

namespace reinforcement_learning
{
namespace onnx
{
int create_onnx_model(
    std::unique_ptr<m::i_model>& retval, const u::configuration& config, i_trace* trace_logger, api_status* status)
{
  const char* app_id = config.get(name::APP_ID, "");
  const char* output_name = config.get(name::ONNX_OUTPUT_NAME, nullptr);
  if (output_name == nullptr)
  {
    RETURN_ERROR_LS(trace_logger, status, inference_configuration_error)
        << "Output name is not provided in the configuration.";
  }

  RL_IGNORE_DEPRECATED_USAGE_START
  bool use_unstructured_input = config.get_bool(name::ONNX_USE_UNSTRUCTURED_INPUT, false);
  RL_IGNORE_DEPRECATED_USAGE_END

  const char* input_serialization = config.get(name::INPUT_SERIALIZATION, value::TENSOR_NOTATION_INPUT_SERIALIZATION);

  // TODO: deprecate "USE_UNSUTRUCTURED_INPUT" in favour of "INPUT_SERIALIZATION" types
  if (use_unstructured_input && strcmp(input_serialization, value::TENSOR_NOTATION_INPUT_SERIALIZATION) != 0)
  {
    RETURN_ERROR_LS(trace_logger, status, inference_configuration_error)
        << "Unstructured input is only supported with TENSOR_NOTATION_INPUT_SERIALIZATION.";
  }
  else if (!use_unstructured_input && strcmp(input_serialization, value::TENSOR_NOTATION_INPUT_SERIALIZATION) == 0)
  {
    RETURN_ERROR_LS(trace_logger, status, inference_configuration_error)
        << "Structured input is not supported with TENSOR_NOTATION_INPUT_SERIALIZATION.";
  }

  if (strcmp(input_serialization, value::TENSOR_NOTATION_INPUT_SERIALIZATION))
  {
    RETURN_ERROR_LS(trace_logger, status, input_serialization_unsupported)
        << "Input serialization type is not supported: " << input_serialization << ". Supported types are: "
        << value::TENSOR_NOTATION_INPUT_SERIALIZATION << ".";
  }

  retval.reset(new onnx_model(trace_logger, app_id, output_name, use_unstructured_input));

  return error_code::success;
};

void register_onnx_factory() { model_factory.register_type(value::ONNXRUNTIME_MODEL, create_onnx_model); }
}  // namespace onnx
}  // namespace reinforcement_learning