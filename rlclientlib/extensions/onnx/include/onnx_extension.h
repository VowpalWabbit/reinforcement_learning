#pragma once

namespace reinforcement_learning
{
namespace onnx
{
void register_onnx_factory();
}
}  // namespace reinforcement_learning

// Constants

namespace reinforcement_learning
{
namespace name
{
// TODO: Explore and expose useful configuration settings here
const char* const ONNX_OUTPUT_NAME = "onnx.output_name";

const char* const TENSOR_NOTATION_INPUT_SERIALIZATION = "ONNX/TENSOR_NOTATION";

// Unstructured input === TENSOR_NOTATION_INPUT_SERIALIZATION
const char* const ONNX_USE_UNSTRUCTURED_INPUT = "onnx.use_unstructured_input";

}  // namespace name
}  // namespace reinforcement_learning

namespace reinforcement_learning
{
namespace value
{
const char* const ONNXRUNTIME_MODEL = "ONNXRUNTIME";
}
}  // namespace reinforcement_learning