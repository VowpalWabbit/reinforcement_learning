#include "tensor_parser.h"

#include <algorithm>
#include <sstream>

namespace reinforcement_learning { namespace onnx {

namespace tensor_parser
{
  enum CONSUME_TYPE : bool { EXCLUSIVE = false, INCLUSIVE = true };
  enum KNOWN_CHARS : char { END = '\0', DOUBLE_QUOTE = '\"', SEMICOLON = ';', COMMA = ',', OPEN_BRACE = '{', CLOSE_BRACE = '}', BACKSLASH = '\\', COLON = ':' };

  namespace errors
  {
    struct error_context
    {
    private:
      std::vector<std::string>& errors;
      const std::string prefix;
      tensor_parser::parser_context& parser_context;

    public:
      inline error_context(std::vector<std::string>& target, const std::string prefix, tensor_parser::parser_context& parser_context)
        : errors(target), prefix(prefix), parser_context(parser_context)
      {
      }

      inline error_context with_prefix(const std::string prefix)
      {
        return error_context(errors, prefix, parser_context);
      }

      inline bool append_error(const std::string& detail)
      {
        std::stringstream error_builder;
        error_builder << "Parse failure at position " << parser_context.position();

        if (prefix.size() > 0)
        {
          error_builder << " " << prefix;
        }
          
        error_builder << ": " << detail;

        errors.push_back(error_builder.str());

        // This is intended to be used as an || append during the consume/consume_exact API pattern
        return false;
      }
    };
  }

  namespace primitives 
  {
    struct none {};

    template <typename parse_context_t, const char ch>
    struct check_exact
    {
      static const char _ch = ch;

      inline static bool invoke(const char c, parse_context_t&)
      {
        return invoke(c);
      }

      inline static bool invoke(const char c)
      {
        return (ch == c);
      }
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
      inline static bool invoke(const char c, parse_context_t& context)
      {
        return context.invoke(c);
      }

      inline static bool end(parse_context_t& context)
      {
        return context.end();
      }
    };

    template <typename parse_context_t, typename until_t, typename action_t = no_op<parse_context_t, true>>
    struct parse_frame
    {
      inline static bool until(parse_context_t& context, const char c)
      {
        return until_t::invoke(c, context);
      }

      inline static bool action(parse_context_t& context, const char c)
      {
        return action_t::invoke(c, context);
      }

      inline static bool end(parse_context_t& context)
      {
        return action_t::end(context);
      }
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

        if (!parse_frame_t::action(context, curr_c))
        {
          return false;
        }

        reading_head++;
      }

      return false;
    }

    template <char ch>
    inline static bool consume_exact(const char*& reading_head)
    {
      if (reading_head == nullptr || *reading_head != ch)
      {
        return false;
      }

      reading_head++;
      return true;
    }
  }

  namespace base64
  {
    struct parse_context
    {
    private:
      size_t padding_count;
      size_t count;
      uint32_t running;
      std::vector<uint8_t>& bytes;
      errors::error_context& error_context;

    public:

      inline parse_context(std::vector<uint8_t>& target, errors::error_context& errors_target) : bytes(target), error_context(errors_target), count(0), running(0), padding_count(0)
      {
      }

      inline bool invoke(const char c)
      {
        running = running << 6;
        count++;

        if (padding_count || c == '=')
        {
          padding_count++;
        }
        else if (!padding_count)
        {
          if (c >= 'A' /* 65 */ && c <= 'Z' /* 90 */)
          {
            running = running | (c - 'A' + 0); // 0 - 25
          }
          else if (c >= 'a' /* 97 */ && c <= 'z' /* 122 */)
          {
            running = running | (c - 'a' + 26); // 26 - 51
          }
          else if (c >= '0' /* 48 */ && c <= '9' /* 57 */)
          {
            running = running | (c - '0' + 52); // 52 - 61
          }
          else if (c == '+')
          {
            running = running | 62;
          }
          else if (c == '/')
          {
            running = running | 63;
          }
          else
          {
            // TODO: Bad character
            std::stringstream error_detail_builder;
            error_detail_builder << "Invalid base64 character: '" << c << "'.";

            return error_context.append_error(std::string(error_detail_builder.str()));
          }

          if (count % 4 == 0)
          {
            bytes.push_back((running >> 16) & 0xFF);
            bytes.push_back((running >> 8) & 0xFF);
            bytes.push_back(running & 0xFF);

            running = 0;
          }
        }

        return true;
      }

      inline bool end()
      {
        if (count % 4 != 0)
        {
          std::stringstream error_detail_builder;
          error_detail_builder << "Invalid number of base64 characters: '" << count << "'.";

          return error_context.append_error(std::string(error_detail_builder.str()));
        }

        switch (padding_count)
        {
        case 0:
          break;
        case 1:
          bytes.push_back((running >> 16) & 0xFF);
          bytes.push_back((running >> 8) & 0xFF);
          break;
        case 2:
          bytes.push_back((running >> 16) & 0xFF);
          break;
        default:
          std::stringstream error_detail_builder;
          error_detail_builder << "Invalid number of base64 padding characters: '" << padding_count << "'.";

          return error_context.append_error(std::string(error_detail_builder.str()));
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
  }

  template <char escape>
  class escaped_string 
  {
  public:
    struct parse_context
    {
      bool in_escape;
      std::string& value;

    public:
      parse_context(std::string& target) : value(target), in_escape(false)
      {
      }

      inline bool invoke(const char c)
      {
        if (c != escape || in_escape)
        {
          value.push_back(c);
          in_escape = false;
        }
        else if (c == escape)
        {
          in_escape = true;
        }

        return true;
      }

      inline bool end()
      {
        return true;
      }
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

  using primitives::consume_exact;
  using primitives::consume;
  using primitives::until;

  using escaped = escaped_string<BACKSLASH>;

  bool parse_tensor_value(const char*& reading_head, std::vector<byte_t>& dims, std::vector<byte_t>& data, errors::error_context& error_target)
  {
    errors::error_context error_context = error_target.with_prefix("while parsing tensor value");

    auto dims_context = base64::parse_context(dims, error_context);
    auto data_context = base64::parse_context(data, error_context);

    // " <base64 dimensions> ; <base64 data> "
    return consume_exact<DOUBLE_QUOTE>(reading_head) &&
      base64::consume<INCLUSIVE, base64::until<SEMICOLON>>(reading_head, dims_context) &&
      base64::consume<INCLUSIVE, base64::until<DOUBLE_QUOTE>>(reading_head, data_context);
  }

  bool parse_tensor_name_value(const char*& reading_head, std::string& name, bytes_t& shape_bytes, bytes_t& value_bytes, errors::error_context& error_target)
  {
    auto name_context = escaped::parse_context(name);

    // " <escaped_name> " : <tensor_value>
    return 
      consume_exact<DOUBLE_QUOTE>(reading_head) &&
      (escaped::consume<INCLUSIVE, escaped::until<DOUBLE_QUOTE>>(reading_head, name_context) 
        || // on error:
           error_target.with_prefix("while parsing tensor name").append_error("Expected '\"'.")) &&
      consume_exact<COLON>(reading_head) &&
      parse_tensor_value(reading_head, shape_bytes, value_bytes, error_target);
  }

  bool parse(parser_context& context)
  {
    // PERF: Can the branchiness here be made nicer through computed jumps?

    if (context._parsed)
    {
      return false;
    }

    // Treat empty lines as empty examples, similar to VW
    if (context.line().empty())
    {
      return true;
    }

    const char*& reading_head = context._reading_head;

    // '{' <tensor_name_value> [ ',' <tensor_name_value> ]* 
    if (!consume_exact<OPEN_BRACE>(reading_head))
    {
      return false;
    }

    // Empty objects are valid, empty examples (match VWJSON parser behaviour)
    if (consume_exact<CLOSE_BRACE>(reading_head))
    {
      return true;
    }

    errors::error_context error_context(context._errors, "while parsing tensor notation", context);

    do
    {
      std::string name;
      bytes_t tensor_shape_bytes;
      bytes_t tensor_data_bytes;

      if (!parse_tensor_name_value(reading_head, name, tensor_shape_bytes, tensor_data_bytes, error_context))
      {
        return false;
      }

      context._input_builder.push_input(
        std::move(name), 
        std::move(std::make_pair(tensor_shape_bytes, tensor_data_bytes)));

    } while (consume_exact<COMMA>(reading_head)); // consume's API is to move reading_head until after success or before first failure.
                                                  // in the case of consume_exact, it can be used to switch based on whether the 
                                                  // character at the reading_head is what is expected, or no.

    return 
      consume_exact<CLOSE_BRACE>(reading_head) 
      || // on error:
         error_context.append_error("Expected '}'.");
  }

}
}}