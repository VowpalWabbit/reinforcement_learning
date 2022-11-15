#include "request_builders.h"

#include "live_model_impl.h"

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <string>
#include <vector>

namespace reinforcement_learning
{

rank_builder::rank_builder(live_model_impl* live_model) : model(live_model) {}

int rank_builder::rank(ranking_response& resp, api_status* status)
{
  const auto uuid = boost::uuids::to_string(boost::uuids::random_generator()());
  if (_event_id == nullptr) { set_event_id(uuid.c_str()); }

  auto err_resp = model->choose_rank(_event_id, _context_json, _flags, resp, status);
  clear();
  return err_resp;
}

rank_builder& rank_builder::clear() { return basic_builder<rank_builder>::clear(); }

}  // namespace reinforcement_learning