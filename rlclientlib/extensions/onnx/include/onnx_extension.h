#pragma once

#include "future_compat.h"

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

// Unstructured input === TENSOR_NOTATION_INPUT_SERIALIZATION
RL_DEPRECATED("Use the INPUT_SERIALIZATION configuration mechanism with the TENSOR_NOTATION value instead")
const char* const ONNX_USE_UNSTRUCTURED_INPUT = "onnx.use_unstructured_input";

}  // namespace name
}  // namespace reinforcement_learning

namespace reinforcement_learning
{
namespace value
{
const char* const ONNXRUNTIME_MODEL = "ONNXRUNTIME";

const char* const TENSOR_NOTATION_INPUT_SERIALIZATION = "ONNX/TENSOR_NOTATION";
}
}  // namespace reinforcement_learning