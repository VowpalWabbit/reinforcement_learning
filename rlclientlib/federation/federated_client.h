#pragma once

#include "future_compat.h"
#include "model_mgmt.h"

#include <cstdint>
#include <string>
#include <vector>

namespace reinforcement_learning
{
/**
 * @brief Allows an implementation to be configurable based on what kinds of
 * aggregation payloads it supports.
 */
enum class federated_aggregation_payload_t : uint32_t
{
  /// A delta containing save_resume info in VW format
  vw_delta = 0,
  // A full model containing save_resume info in VW format
  vw_model,
  /// A delta containing predict info only in VW format
  vw_predict_only_delta,
  /// A full model containing predict info only in VW format
  vw_predict_only_model
};

/**
 * @brief This interface represents the fetching of new models and reporting of
 * local changes required for a federated learning system. This interface is not
 * stable and may still change.
 */
struct i_federated_client
{
  i_federated_client(federated_aggregation_payload_t federated_aggregation_payload)
      : _federated_aggregation_payload(federated_aggregation_payload)
  {
  }

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
  virtual RL_ATTR(nodiscard) int try_get_model(const std::string& app_id,
      /* inout */ model_management::model_data& data, /* out */ bool& model_received, api_status* status = nullptr) = 0;

  /**
   * @brief Report the final model based on local data and the *last* model
   * received from try_get_model. There must be a matching call to report_result
   * for every successful call to try_get_model, and you cannot call
   * try_get_model again until report_result has been called.
   *
   * @param payload payload represents the payload to aggregate. Will be
   *                interpreted based on the return value of
   *                get_aggregation_payload_type()
   * @param status Contains error information in the event of a failure
   * @returns Status code
   */
  virtual RL_ATTR(nodiscard) int report_result(const std::vector<uint8_t>& payload, api_status* status = nullptr) = 0;

  /// The supplied payload to report_result must match the expected type.
  RL_ATTR(nodiscard) federated_aggregation_payload_t get_expected_aggregation_payload_type() const noexcept
  {
    return _federated_aggregation_payload;
  }

private:
  federated_aggregation_payload_t _federated_aggregation_payload;
};

}  // namespace reinforcement_learning
