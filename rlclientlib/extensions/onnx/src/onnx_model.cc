#include "onnx_model.h"

#include "api_status.h"
#include "err_constants.h"
#include "factory_resolver.h"
#include "onnx_input.h"
#include "str_util.h"
#include "trace_logger.h"
#include "vw/core/scope_exit.h"

#include <memory>
#include <sstream>

namespace reinforcement_learning
{
namespace onnx
{
// This is used for statically introspecting the model. It will be used regardless of whether the actual
// inference is done on CPU/GPU/Accelerator.
static Ort::AllocatorWithDefaultOptions DefaultOnnxAllocator;
static const OrtApi& OnnxRuntimeCApi = Ort::GetApi();

inline void OrtLogCallback(void* param, OrtLoggingLevel severity, const char* category, const char* logid,
    const char* code_location, const char* message)
{
  i_trace* trace_logger = static_cast<i_trace*>(param);
  if ((trace_logger) == nullptr) { return; }

  int loglevel = LEVEL_ERROR;
  switch (severity)
  {
    case ORT_LOGGING_LEVEL_VERBOSE:
      loglevel = LEVEL_DEBUG;
      break;

    case ORT_LOGGING_LEVEL_INFO:
      loglevel = LEVEL_INFO;
      break;

    case ORT_LOGGING_LEVEL_WARNING:
      loglevel = LEVEL_WARN;
      break;

    case ORT_LOGGING_LEVEL_FATAL:  // TODO: Should this be a background error?
    case ORT_LOGGING_LEVEL_ERROR:
      loglevel = LEVEL_ERROR;
      break;
  }

  std::stringstream buf;
  buf << "[onnxruntime, modelid=" << logid << "]: " << message;

  TRACE_LOG(trace_logger, loglevel, buf.str());
}

onnx_model::onnx_model(i_trace* trace_logger, const char* app_id, const char* output_name, bool use_unstructured_input)
    : _trace_logger(trace_logger)
    , _output_name(output_name)
    , _use_unstructured_input(use_unstructured_input)
    , _env(Ort::Env(ORT_LOGGING_LEVEL_VERBOSE, app_id, OrtLogCallback, trace_logger))
{
  //_session_options.SetThreadPoolSize(thread_pool_size);

  // ORT_DISABLE_ALL -> To disable all optimizations
  // ORT_ENABLE_BASIC -> To enable basic optimizations (Such as redundant node removals)
  // ORT_ENABLE_EXTENDED -> To enable extended optimizations (Includes level 1 + more complex optimizations like node
  // fusions) ORT_ENABLE_ALL -> To Enable All possible opitmizations
  _session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);  // Make it FAST
}

int onnx_model::update(const model_management::model_data& data, bool& model_ready, api_status* status)
{
  try
  {
    TRACE_INFO(_trace_logger, utility::concat("Received new model data. With size ", data.data_sz()));

    if (data.data_sz() <= 0) { RETURN_ERROR_LS(_trace_logger, status, model_update_error) << "Empty model data."; }

    std::shared_ptr<Ort::Session> new_session =
        std::make_shared<Ort::Session>(_env, data.data(), data.data_sz(), _session_options);

    // Validate that the model makes sense
    // Rules:
    // 1. There are N inputs, which are all tensors of floats
    // 2. There is an output with the provided name, which is a tensor of floats

    size_t input_count = new_session->GetInputCount();
    for (size_t i = 0; i < input_count; i++)
    {
      // TODO: Support more input types (by making the input interface richer)
      Ort::TypeInfo input_type_info = new_session->GetInputTypeInfo(i);
      if (input_type_info.GetONNXType() != ONNX_TYPE_TENSOR ||
          input_type_info.GetTensorTypeAndShapeInfo().GetElementType() != ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT)
      {
        RETURN_ERROR_LS(_trace_logger, status, model_update_error) << "Invalid input type. Expected: tensor<float>.";
      }
    }

    bool found_output = false;
    size_t output_index = 0;
    size_t output_count = new_session->GetOutputCount();
    for (output_index = 0; output_index < output_count; output_index++)
    {
      char* output_name = new_session->GetOutputName(output_index, DefaultOnnxAllocator);

      if (_output_name == output_name) { found_output = true; }

      DefaultOnnxAllocator.Free(output_name);

      if (found_output) { break; }
    }

    if (!found_output)
    {
      RETURN_ERROR_LS(_trace_logger, status, model_update_error)
          << "Could not find output with name '" << _output_name << "' in model.";
    }

    // TODO: Support more output types
    Ort::TypeInfo output_type_info = new_session->GetOutputTypeInfo(output_index);
    if (output_type_info.GetONNXType() != ONNX_TYPE_TENSOR ||
        output_type_info.GetTensorTypeAndShapeInfo().GetElementType() != ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT)
    { RETURN_ERROR_LS(_trace_logger, status, model_update_error) << "Invalid output type. Expected: tensor<float>."; }

    // TODO: Should we add additional checks to make sure the next two sets are atomic?
    _output_index = output_index;

    _master_session = std::move(new_session);
  }
  catch (const std::exception& e)
  {
    RETURN_ERROR_LS(_trace_logger, status, model_update_error) << e.what();
  }
  catch (...)
  {
    RETURN_ERROR_LS(_trace_logger, status, model_update_error) << "Unknown error";
  }

  model_ready = true;
  return error_code::success;
}

int onnx_model::choose_rank(const char* event_id, uint64_t rnd_seed, string_view features, std::vector<int>& action_ids,
    std::vector<float>& action_pdf, std::string& model_version, api_status* status)
{
  std::shared_ptr<Ort::Session> local_session = _master_session;
  if (!local_session)
  {
    // Model is not ready
    RETURN_ERROR_LS(_trace_logger, status, model_rank_error) << "No model loaded.";
  }

  // TODO: Support GPU scoring - it is unfortunate that we cannot simply grab the appropriate allocator
  // based on what version of onnxruntime we are loading.
  Ort::MemoryInfo memory_info =
      Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);

  onnx_input_builder input_context(_trace_logger);
  if (_use_unstructured_input) { RETURN_IF_FAIL(read_tensor_notation(features, input_context, status)); }
  else
  {
    // TODO: This is a placeholder for implementing ExampleBuilder APIs. We put this here to ensure that we can make a
    // non-breaking-change in the future that makes structured input the default.
    RETURN_ERROR_LS(_trace_logger, status, model_rank_error)
        << "Structured input is not yet implemented. See onnx_model.cc.";
  }

  Ort::RunOptions run_options{nullptr};

  std::vector<const char*> input_names = input_context.input_names();
  std::vector<Ort::Value> inputs;

  RETURN_IF_FAIL(input_context.allocate_inputs(inputs, memory_info, status));

  // Use the C API to avoid an unneeded throw in the error case
  OrtValue* onnx_output = nullptr;

  // This cast-chain is taken from the OnnxRuntime code implementation of the C++ API of Ort::Session::Run().
  auto ort_input_values = reinterpret_cast<const OrtValue**>(const_cast<Ort::Value*>(inputs.data()));

  std::vector<const char*> output_node_names;
  output_node_names.push_back(_output_name.c_str());

  OrtStatus* run_status = OnnxRuntimeCApi.Run(
      local_session->operator OrtSession*(),  // Unwrap the underlying C reference to pass to the C API
      Ort::RunOptions{nullptr}, input_names.data(), ort_input_values,
      input_context.input_count(),                 // Inputs: Names, Values, Count
      output_node_names.data(), 1, &onnx_output);  // Outputs: Names, Count, Values; note the inconsistency

  if (run_status)
  {
    auto release_guard = VW::scope_exit([&run_status] { OnnxRuntimeCApi.ReleaseStatus(run_status); });
    RETURN_ERROR_LS(_trace_logger, status, extension_error) << OnnxRuntimeCApi.GetErrorMessage(run_status);
  }

  // Re-wrap in Ort::Value to ensure proper destruction (no point in using VW::scope_exit, since we allocate either way)
  Ort::Value target_output = Ort::Value(onnx_output);

  size_t num_elements = target_output.GetTensorTypeAndShapeInfo().GetElementCount();

  // TODO: Once we update to OnnxRuntime v1.5.1, we can change this to grab immutable data via GetTensorData<float>()
  float* floatarr = target_output.GetTensorMutableData<float>();

  for (size_t i = 0; i < num_elements; i++)
  {
    action_ids.push_back(i);
    action_pdf.push_back(floatarr[i]);
  }

  return error_code::success;
}
}  // namespace onnx
}  // namespace reinforcement_learning