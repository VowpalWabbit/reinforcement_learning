#include "slot_ranking.h"

#include "api_status.h"
#include "err_constants.h"

namespace reinforcement_learning
{
slot_ranking::slot_ranking(char const* id) : _id{id}, _chosen_action_id{0} {}

char const* slot_ranking::get_id() const { return _id.c_str(); }

void slot_ranking::set_id(char const* id) { _id = id; }

int slot_ranking::get_chosen_action_id(size_t& action_id, api_status* status) const
{
  if (_ranking.empty()) { RETURN_ERROR_LS(nullptr, status, action_not_found); }
  action_id = _chosen_action_id;
  return error_code::success;
}

int slot_ranking::set_chosen_action_id(size_t action_id, api_status* status)
{
  _chosen_action_id = action_id;
  return error_code::success;
}

int slot_ranking::set_chosen_action_id_unchecked(size_t action_id, api_status* /*unused*/)
{
  _chosen_action_id = action_id;
  return error_code::success;
}

void slot_ranking::push_back(const size_t action_id, const float prob) { _ranking.emplace_back(action_id, prob); }

size_t slot_ranking::size() const { return _ranking.size(); }

void slot_ranking::clear()
{
  _id.clear();
  _chosen_action_id = 0;
  _ranking.clear();
}

slot_ranking::slot_ranking(slot_ranking&& tmp) noexcept
    : _id(std::move(tmp._id)), _chosen_action_id(tmp._chosen_action_id), _ranking(std::move(tmp._ranking))
{
}

slot_ranking& slot_ranking::operator=(slot_ranking&& tmp) noexcept
{
  std::swap(_id, tmp._id);
  std::swap(_chosen_action_id, tmp._chosen_action_id);
  std::swap(_ranking, tmp._ranking);
  return *this;
}

using iterator = slot_ranking::iterator;
using const_iterator = slot_ranking::const_iterator;

const_iterator slot_ranking::begin() const { return {_ranking}; }

iterator slot_ranking::begin() { return {_ranking}; }

const_iterator slot_ranking::end() const { return {_ranking, _ranking.size()}; }

iterator slot_ranking::end() { return {_ranking, _ranking.size()}; }
}  // namespace reinforcement_learning
