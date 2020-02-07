//#pragma once

#include "tensor_notation.h"
#include "err_constants.h"
#include "api_status.h"

#include <memory>
#include <assert.h>

// for base64 decoding
#include <cpprest/http_client.h>

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

namespace tokens {
  const char NULL_TERMINATOR = '\0';
  const char ESCAPE = '\\';
  const char DQUOTE = '\"';
  const char SEMICOLON = ';';
  const char COLON = ':';
  const char COMMA = ',';
  const char OPEN_CBRACKET = '{';
  const char CLOSE_CBRACKET = '}';
}

using bytes = std::vector<unsigned char>;

#define NO_ERROR_OR_RETURN(val) \
  if (_has_error) { \
    return val; \
  }
//!define# PARSER_HEADER

#define NO_ERROR_OR_RETURN_DEFAULT(val) \
  if (_has_error) { \
    return val; \
  }
//!define# PARSER_HEADER

class TensorParser
{
public:
  // warnings/errors?
  TensorParser(const char* tensor_notation, OnnxRtInputContext& input_context);

  inline bool succeeded()
  {
    return !_has_error;
  }

  inline std::string error_message()
  {
    return _error_message;
  }

private:
  inline void skip_whitespace()
  {
    NO_ERROR_OR_RETURN();

    while (*_reading_head == ' ' 
        || *_reading_head == '\t' 
        || *_reading_head == '\r'
        || *_reading_head == '\n')
    {
      // This is robust to end-of-string without an explicit check
      // because \0 would kick out of the loop above. Technically,
      // the optimizer should take care of it, though. 
      _reading_head++; 
    }
  }

  inline std::stringstream error_builder()
  {
    std::stringstream result("Error parsing TensorNotation at position ");

    // TODO: Error reporting here should be line/col based, rather than position-based.
    result << (_reading_head - _parse_buffer) << ": ";

    return result;
  }

  inline void set_error(std::string error_message)
  {
    _has_error = true;
    _error_message = error_message;
  }

  template <char c>
  inline void read_character()
  {
    NO_ERROR_OR_RETURN();

    if (*_reading_head == c)
    {
      _reading_head++;
    }
    else
    {
      auto err_stream = error_builder();
      err_stream << "Expecting '" << c << "'; actual '" << *_reading_head << "'.";
      set_error(err_stream.str());
    }
  }

  inline bool is_base64(const char c) const
  {
    NO_ERROR_OR_RETURN_DEFAULT(false);

    // I really wish ASCII was a nicer encoding (i.e. alphanumeric = contiguous range) - it 
    // would make base64 much easier to parse without doing a lot of superfluous operations.
    return (c == '+' /* 43 */ 
        || c == '/' /* 47 */ 
        || (c >= '0' /* 48 */ && c <= '9' /* 57 */)
        || (c >= 'A' /* 65 */ && c <= 'Z' /* 90 */)
        || (c >= 'a' /* 97 */  && c <= 'z' /* 122 */)
        || c == '=' /* padding character */);
  }

  inline bytes read_base64()
  {
    NO_ERROR_OR_RETURN_DEFAULT(bytes());

    _token_start = _reading_head;

    // scan BASE64 sequence
    while (is_base64(*_reading_head))
    { 
      _reading_head++;
    }

    // this performs a copy
    std::string base64string(_token_start, _reading_head - _token_start);

    if (base64string.length() % 4 != 0)
    {
      auto err_stream = error_builder();
      err_stream << "Base64 string \"" << base64string << "\" length is not divisible by 4: " << base64string.length() << ".";
      set_error(err_stream.str());
    }

    bytes conversion = ::utility::conversions::from_base64(base64string);
    return conversion;
  }

  inline std::string read_tensor_name()
  {
    NO_ERROR_OR_RETURN_DEFAULT(std::string(""));

    // Consume \'
    read_character<tokens::DQUOTE>();
    
    _token_start = _reading_head;

    // Scan for the name string
    bool in_escape = false;
    while (true)
    {
      switch (*_reading_head)
      {
        case tokens::DQUOTE:
          if (in_escape) break;
          // If not in an escape sequence, terminate scanning for the name
        case tokens::NULL_TERMINATOR:
          goto end_name_scan;
        case tokens::ESCAPE:
          in_escape = !in_escape;
      }

      _reading_head++;
    }
    end_name_scan:

    const char* token_end = _reading_head;

    // Consume \'
    read_character<tokens::DQUOTE>();

    return std::string(_token_start, token_end - _token_start);
  }

  inline tensor_data read_tensor_data()
  {
    NO_ERROR_OR_RETURN_DEFAULT(tensor_data(bytes(), bytes()))

    // Consume \'
    read_character<tokens::DQUOTE>();

    // TODO: Support reading type information for the tensor (and later map/sequence)
    // See value_t definition in tensor_notation.h
    // <TYPE_INFO> := '<' VALUE_TYPE '>'
    // <VALUE_TYPE> := "float"
    // read_value_type();
    // read_character<';'>();
    
    // Read base_64(int_64t[]) until ';'
    bytes dimensions_base64 = read_base64();

    // Consume ';' - also another bit of validation that dimensions were parsed 
    // meaningfully
    read_character<tokens::SEMICOLON>();

    // Read base_64(float[]) until '\''
    bytes values_base64 = read_base64();

    // Consume \'
    read_character<tokens::DQUOTE>();

    // Should we do the base64 parse here?
    return std::make_pair(dimensions_base64, values_base64);
  }

  inline void read_tensor()
  {
    NO_ERROR_OR_RETURN();

    std::string name = read_tensor_name();

    skip_whitespace();

    read_character<tokens::COLON>();

    skip_whitespace();

    tensor_data value = read_tensor_data();

    _parse_context.push_input(name, value);
  }

  inline void read_tensor_list()
  {
    NO_ERROR_OR_RETURN();

    skip_whitespace();

    read_character<tokens::OPEN_CBRACKET>();
    
    skip_whitespace();

    while (*_reading_head != tokens::CLOSE_CBRACKET 
        && *_reading_head != tokens::NULL_TERMINATOR)
    {
      NO_ERROR_OR_RETURN();

      read_tensor();
      skip_whitespace();
      if (*_reading_head == tokens::COMMA)
      {
        read_character<tokens::COMMA>();
        skip_whitespace();
      }
    }

    read_character<tokens::CLOSE_CBRACKET>();
  }

private:
  const char* const _parse_buffer;
  
  const char* _token_start;
  const char* _reading_head;

  OnnxRtInputContext& _parse_context;

  bool _has_error;
  std::string _error_message;
};

TensorParser::TensorParser(const char* tensor_notation, OnnxRtInputContext& input_context) 
: _parse_buffer{tensor_notation}, 
  _reading_head{tensor_notation}, 
  _token_start{tensor_notation}, 
  _parse_context{input_context},
  _has_error{false},
  _error_message{""}
{
  if (_parse_buffer && *_parse_buffer != tokens::NULL_TERMINATOR)
  {
    read_tensor_list();
  }
}

int read_tensor_notation(const char* tensor_notation, OnnxRtInputContext* input_context, api_status* status)
{
  TensorParser parser(tensor_notation, *input_context);

  if (!parser.succeeded())
  {
    RETURN_ERROR_LS(nullptr, status, extension_error)
      << "OnnxExtension: Failed to deserialize tensor: "
      << parser.error_message();
  };

  return error_code::success;
}

}}