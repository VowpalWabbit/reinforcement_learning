#pragma once

namespace reinforcement_learning { namespace onnx {
  void register_onnx_factory();
}}

// Constants

namespace reinforcement_learning { namespace name {
  //TODO: Explore and expose useful configuration settings here
  const char *const ONNX_USE_UNSTRUCTURED_INPUT = "onnx.use_unstructured_input";
  const char *const ONNX_OUTPUT_NAME          = "onnx.output_name";
}}

namespace reinforcement_learning { namespace value {
  const char *const ONNXRUNTIME_MODEL = "ONNXRUNTIME";
}}