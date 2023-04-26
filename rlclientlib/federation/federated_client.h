#pragma once

#include "api_status.h"
#include "future_compat.h"
#include "model_mgmt.h"

#include <cstdint>
#include <string>
#include <vector>

namespace reinforcement_learning
{
/**
 * @brief This interface represents the fetching of new models and reporting of
 * local changes required for a federated learning system. This interface is not
 * stable and may still change.
 */
struct i_federated_client
{
  virtual ~i_federated_client() = default;

  /**
   * @brief Try and get a new global model. There must be a matching call to
   * report_result for every successful call to try_get_model, and you cannot
   * call try_get_model again until report_result has been called.
   *
   * @param app_id App id to try and get the model for.
   * @param data This object will have its data field filled if a new model is
   *             available and refresh count incremented. Otherwise it is left unchanged.
   * @param model_received True, if a new model was available, otherwise false.
   * @param status Contains error information in the event of a failure.
   * @returns Status code
   */
  RL_ATTR(nodiscard)
  virtual int try_get_model(const std::string& app_id,
      /* inout */ model_management::model_data& data, /* out */ bool& model_received, api_status* status = nullptr) = 0;

  /**
   * @brief Report the final model based on local data and the *last* model
   * received from try_get_model. There must be a matching call to report_result
   * for every successful call to try_get_model, and you cannot call
   * try_get_model again until report_result has been called.
   *
   * @param payload payload represents the payload to aggregate. Must be a serialized VW model delta.
   * @param size payload represents the size of the payload to aggregate. Must be a serialized VW model delta.
   * @param status Contains error information in the event of a failure
   * @returns Status code
   */
  RL_ATTR(nodiscard) virtual int report_result(const uint8_t* payload, size_t size, api_status* status = nullptr) = 0;
};

}  // namespace reinforcement_learning
