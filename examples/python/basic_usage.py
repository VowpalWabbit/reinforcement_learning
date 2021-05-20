import uuid
import rl_client


def on_error(error_code, error_message):
    print("Background error:")
    print(error_code)
    print(error_message)

def load_config_from_json(file_name):
    with open(file_name, 'r') as config_file:
        return rl_client.create_config_from_json(config_file.read())

def main():
    config = load_config_from_json("client.json")
    model = rl_client.live_model(config, on_error)
    model.init()

    event_id = str(uuid.uuid4())
    context = '{"User":{"id":"a","major":"eng","hobby":"hiking"},"_multi":[{"N1":{"F1":"V1"},"N2":{"F2":"V2"}},{"N3":{"F1":"V3"}}]}'

    response = model.choose_rank(context, event_id=event_id)

    print("event_id: " + response.event_id)
    print("model_id: " + response.model_id)
    print("chosen action id: " + str(response.chosen_action_id))
    print("all action probabilities " + str(response.actions_probabilities))

    response = model.choose_rank(context)
    print("event_id: " + response.event_id)
    print("model_id: " + response.model_id)
    print("chosen action id: " + str(response.chosen_action_id))
    print("all action probabilities " + str(response.actions_probabilities))

    event_id = str(uuid.uuid4())
    response = model.choose_rank(context, event_id=event_id)
    print("event_id: " + response.event_id)
    print("model_id: " + response.model_id)
    print("chosen action id: " + str(response.chosen_action_id))
    print("all action probabilities " + str(response.actions_probabilities))

    outcome = 1.0
    model.report_outcome(event_id, outcome)

if __name__ == "__main__":
   main()
