
#include "onnx_input.h"

#include <string>
#include <vector>

namespace reinforcement_learning { namespace onnx {

// This implements a very restricted parser/serializer for a JSON-like dialect to
// describe input to the ONNX Runtime
//
// The format is:
// <TENSORS> := "{" <TENSOR-LIST> "}"
// <TENSOR-LIST> := <TENSOR> ["," [<TENSOR-LIST>]]
// <TENSOR> := "\"" <INPUT-NAME> "\":\"" <TENSOR-DATA> "\""
// <TENSOR-DATA> := <DIMS-BASE64> ";" <VALUES-BASE64>
// <DIMS-BASE64> := { base64 encoding of int64[] representing dimensions of the tensor }
// <VALUES-BASE64> := { base64 encoding of float[] representing values of the tensor }
//
// Any other JSON concepts are not allowed. The reason for any relation to JSON is
// the current setup for RLClientLib to log the context as JSON.
// Ideally, we would use protobuf definitions from ONNX to represent the IOContext

namespace tensor_parser
{
  struct parser_context
  {
  public:
    const std::string line() const
    {
      return _line;
    }

    parser_context(const std::string line, onnx_input_builder& input_builder) : _line(line), _parsed(false), _input_builder(input_builder)
    {
      _reading_head = _line.c_str();
    }

    inline const std::vector<std::string> errors() const
    {
      return _errors;
    }

    inline size_t position() const
    {
      return std::distance(_line.c_str(), _reading_head);
    }

    inline const onnx_input_builder& input_builder() const
    {
        return _input_builder;
    }

  private:
    bool _parsed;
    const char* _reading_head;
    const std::string _line;

    std::vector<std::string> _errors;
    onnx_input_builder& _input_builder;

    friend bool parse(parser_context& context);
  };

  bool parse(parser_context& context);
}
}}