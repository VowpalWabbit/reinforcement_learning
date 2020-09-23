#pragma once

#include <cstddef>
#include <iterator>
#include <vector>
#include <string>

namespace reinforcement_learning {
  class api_status;

  class continuous_action_response {
  public:
    continuous_action_response() = default;
    ~continuous_action_response() = default;

    float get_chosen_action() const;
    void set_chosen_action(float action);

    float get_chosen_action_pdf_value() const;
    void set_chosen_action_pdf_value(float pdf_value);

    void set_event_id(const char* event_id);
    void set_event_id(std::string&& event_id);
    const char* get_event_id() const;

    void set_model_id(const char* model_id);
    void set_model_id(std::string&& model_id);
    const char* get_model_id() const;

    void clear();

    continuous_action_response(continuous_action_response&&) noexcept;
    continuous_action_response& operator=(continuous_action_response&&) noexcept;
    continuous_action_response(const continuous_action_response&) = default;
    continuous_action_response& operator =(const continuous_action_response&) = default;
  private:
    float _chosen_action;
    float _chosen_action_pdf_value;
    std::string _model_id;
    std::string _event_id;
  };
}
