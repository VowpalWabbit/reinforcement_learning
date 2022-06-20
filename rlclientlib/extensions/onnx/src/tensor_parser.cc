#include "tensor_parser.h"

#include <algorithm>
#include <sstream>

namespace reinforcement_learning
{
namespace onnx
{
namespace tensor_parser
{
enum CONSUME_TYPE : bool
{
  EXCLUSIVE = false,
  INCLUSIVE = true
};
enum KNOWN_CHARS : char
{
  END = '\0',
  DOUBLE_QUOTE = '\"',
  SEMICOLON = ';',
  COMMA = ',',
  OPEN_BRACE = '{',
  CLOSE_BRACE = '}',
  BACKSLASH = '\\',
  COLON = ':'
};

namespace errors
{
struct error_context
{
private:
  const std::string _prefix;
  std::vector<std::string>& _errors;
  tensor_parser::parser_context& _parser_context;

public:
  inline error_context(
      std::vector<std::string>& target, const std::string& prefix, tensor_parser::parser_context& parser_context)
      : _errors(target), _prefix(prefix), _parser_context(parser_context)
  {
  }

  inline error_context with_prefix(const std::string& prefix)
  {
    return error_context(_errors, prefix, _parser_context);
  }

  inline bool append_error(const std::string& detail)
  {
    std::stringstream error_builder;
    error_builder << "Parse failure at position " << _parser_context.position();

    if (_prefix.size() > 0) { error_builder << " " << _prefix; }

    error_builder << ": " << detail;

    _errors.push_back(error_builder.str());

    // This is intended to be used as an || append during the consume/consume_exact API pattern
    return false;
  }
};
}  // namespace errors

namespace primitives
{
struct none
{
};

template <typename parse_context_t, const char ch>
struct check_exact
{
  static const char _ch = ch;

  inline static bool invoke(const char c, parse_context_t&) { return invoke(c); }

  inline static bool invoke(const char c) { return (ch == c); }
};

template <typename parse_context_t, const char ch>
using until = check_exact<parse_context_t, ch>;

template <typename parse_context_t, bool returns = true>
struct no_op
{
  inline static bool invoke(const char, parse_context_t&) { return returns; }
  inline static bool end(parse_context_t&) { return returns; }
};

template <typename parse_context_t>
struct routing_action
{
  inline static bool invoke(const char c, parse_context_t& context) { return context.invoke(c); }

  inline static bool end(parse_context_t& context) { return context.end(); }
};

template <typename parse_context_t, typename until_t, typename action_t = no_op<parse_context_t, true>>
struct parse_frame
{
  inline static bool until(parse_context_t& context, const char c) { return until_t::invoke(c, context); }

  inline static bool action(parse_context_t& context, const char c) { return action_t::invoke(c, context); }

  inline static bool end(parse_context_t& context) { return action_t::end(context); }
};

template <bool inclusive, typename parse_frame_t, typename parse_context_t>
inline bool consume(const char*& reading_head, parse_context_t& context)
{
  while (reading_head != nullptr && *reading_head != END)
  {
    const char& curr_c = *reading_head;
    if (parse_frame_t::until(context, curr_c))
    {
      if (inclusive) reading_head++;

      return parse_frame_t::end(context);
    }

    if (!parse_frame_t::action(context, curr_c)) { return false; }

    reading_head++;
  }

  return false;
}

template <char ch>
inline static bool consume_exact(const char*& reading_head)
{
  if (reading_head == nullptr || *reading_head != ch) { return false; }

  reading_head++;
  return true;
}
}  // namespace primitives

namespace base64
{
struct parse_context
{
private:
  size_t _count;
  uint32_t _running;
  size_t _padding_count;
  std::vector<uint8_t>& _bytes;
  errors::error_context& _error_context;

public:
  inline parse_context(std::vector<uint8_t>& target, errors::error_context& errors_target)
      : _bytes(target), _error_context(errors_target), _count(0), _running(0), _padding_count(0)
  {
  }

  inline bool invoke(const char c)
  {
    _running = _running << 6;
    _count++;

    if (_padding_count || c == '=') { _padding_count++; }
    else if (!_padding_count)
    {
      if (c >= 'A' /* 65 */ && c <= 'Z' /* 90 */)
      {
        _running = _running | (c - 'A' + 0);  // 0 - 25
      }
      else if (c >= 'a' /* 97 */ && c <= 'z' /* 122 */)
      {
        _running = _running | (c - 'a' + 26);  // 26 - 51
      }
      else if (c >= '0' /* 48 */ && c <= '9' /* 57 */)
      {
        _running = _running | (c - '0' + 52);  // 52 - 61
      }
      else if (c == '+')
      {
        _running = _running | 62;
      }
      else if (c == '/')
      {
        _running = _running | 63;
      }
      else
      {
        // TODO: Bad character
        std::stringstream error_detail_builder;
        error_detail_builder << "Invalid base64 character: '" << c << "'.";

        return _error_context.append_error(std::string(error_detail_builder.str()));
      }

      if (_count % 4 == 0)
      {
        _bytes.push_back((_running >> 16) & 0xFF);
        _bytes.push_back((_running >> 8) & 0xFF);
        _bytes.push_back(_running & 0xFF);

        _running = 0;
      }
    }

    return true;
  }

  inline bool end()
  {
    if (_count % 4 != 0)
    {
      std::stringstream error_detail_builder;
      error_detail_builder << "Invalid number of base64 characters: '" << _count << "'.";

      return _error_context.append_error(std::string(error_detail_builder.str()));
    }

    switch (_padding_count)
    {
      case 0:
        break;
      case 1:
        _bytes.push_back((_running >> 16) & 0xFF);
        _bytes.push_back((_running >> 8) & 0xFF);
        break;
      case 2:
        _bytes.push_back((_running >> 16) & 0xFF);
        break;
      default:
        std::stringstream error_detail_builder;
        error_detail_builder << "Invalid number of base64 padding characters: '" << _padding_count << "'.";

        return _error_context.append_error(std::string(error_detail_builder.str()));
    }

    return true;
  }
};

using parse_action = primitives::routing_action<parse_context>;

template <typename until_t>
using frame = primitives::parse_frame<parse_context, until_t, parse_action>;

template <char ch>
using until = primitives::until<parse_context, ch>;

template <bool inclusive, typename until_t>
inline bool consume(const char*& c, parse_context& context)
{
  return primitives::consume<inclusive, frame<until_t>, parse_context>(c, context);
}
}  // namespace base64

template <char escape>
class escaped_string
{
public:
  struct parse_context
  {
    bool _in_escape;
    std::string& _value;

  public:
    parse_context(std::string& target) : _value(target), _in_escape(false) {}

    inline bool invoke(const char c)
    {
      if (c != escape || _in_escape)
      {
        _value.push_back(c);
        _in_escape = false;
      }
      else if (c == escape)
      {
        _in_escape = true;
      }

      return true;
    }

    inline bool end() { return true; }
  };

  using parse_action = primitives::routing_action<parse_context>;

  template <typename until_t>
  using frame = primitives::parse_frame<parse_context, until_t, parse_action>;

  template <char ch>
  using until = primitives::until<parse_context, ch>;

  template <bool inclusive, typename until_t>
  static inline bool consume(const char*& c, parse_context& context)
  {
    return primitives::consume<inclusive, frame<until_t>, parse_context>(c, context);
  }
};

using primitives::consume;
using primitives::consume_exact;
using primitives::until;

using escaped = escaped_string<BACKSLASH>;

bool parse_tensor_value(const char*& reading_head, std::vector<byte_t>& dims, std::vector<byte_t>& data,
    errors::error_context& error_target)
{
  errors::error_context error_context = error_target.with_prefix("while parsing tensor value");

  auto dims_context = base64::parse_context(dims, error_context);
  auto data_context = base64::parse_context(data, error_context);

  // " <base64 dimensions> ; <base64 data> "
  return consume_exact<DOUBLE_QUOTE>(reading_head) &&
      base64::consume<INCLUSIVE, base64::until<SEMICOLON>>(reading_head, dims_context) &&
      base64::consume<INCLUSIVE, base64::until<DOUBLE_QUOTE>>(reading_head, data_context);
}

bool parse_tensor_name_value(const char*& reading_head, std::string& name, bytes_t& shape_bytes, bytes_t& value_bytes,
    errors::error_context& error_target)
{
  auto name_context = escaped::parse_context(name);

  // " <escaped_name> " : <tensor_value>
  return consume_exact<DOUBLE_QUOTE>(reading_head) &&
      (escaped::consume<INCLUSIVE, escaped::until<DOUBLE_QUOTE>>(reading_head, name_context) ||  // on error:
          error_target.with_prefix("while parsing tensor name").append_error("Expected '\"'.")) &&
      consume_exact<COLON>(reading_head) && parse_tensor_value(reading_head, shape_bytes, value_bytes, error_target);
}

bool parse(parser_context& context)
{
  // PERF: Can the branchiness here be made nicer through computed jumps?

  if (context._parsed) { return false; }

  // Treat empty lines as empty examples, similar to VW
  if (context.line().empty()) { return true; }

  const char*& reading_head = context._reading_head;

  // '{' <tensor_name_value> [ ',' <tensor_name_value> ]*
  if (!consume_exact<OPEN_BRACE>(reading_head)) { return false; }

  // Empty objects are valid, empty examples (match VWJSON parser behaviour)
  if (consume_exact<CLOSE_BRACE>(reading_head)) { return true; }

  errors::error_context error_context(context._errors, "while parsing tensor notation", context);

  do {
    std::string name;
    bytes_t tensor_shape_bytes;
    bytes_t tensor_data_bytes;

    if (!parse_tensor_name_value(reading_head, name, tensor_shape_bytes, tensor_data_bytes, error_context))
    {
      return false;
    }

    context._input_builder.push_input(
        std::move(name), std::move(std::make_pair(tensor_shape_bytes, tensor_data_bytes)));

  } while (
      consume_exact<COMMA>(reading_head));  // consume's API is to move reading_head until after success or before first
                                            // failure. in the case of consume_exact, it can be used to switch based on
                                            // whether the character at the reading_head is what is expected, or no.

  return consume_exact<CLOSE_BRACE>(reading_head) ||  // the OR condition here only runs on error
      error_context.append_error("Expected '}'.");
}

}  // namespace tensor_parser
}  // namespace onnx
}  // namespace reinforcement_learning