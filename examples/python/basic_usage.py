import uuid
import rl_client


def on_error(error_code, error_message):
    print("Background error:")
    print(error_code)
    print(error_message)


def load_config_from_json(file_name):
    with open(file_name, "r") as config_file:
        return rl_client.create_config_from_json(config_file.read())


def basic_usage_cb():
    config = load_config_from_json("client.json")
    model = rl_client.LiveModel(config, on_error)

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


def basic_usage_multistep():
    config = load_config_from_json("client.json")

    model = rl_client.LiveModel(config)

    episode1 = rl_client.EpisodeState("episode1")
    episode2 = rl_client.EpisodeState("episode2")

    # episode1, event1
    context1 = '{"shared":{"F1": 1.0}, "_multi": [{"AF1": 2.0}, {"AF1": 3.0}]}'
    response1 = model.request_episodic_decision("event1", None, context1, episode1)
    print("episode id:", episode1.episode_id)
    print("event id:", response1.event_id)
    print("chosen action:", response1.chosen_action_id)

    # episode2, event1
    context1 = '{"shared":{"F2": 1.0}, "_multi": [{"AF2": 2.0}, {"AF2": 3.0}]}'
    response1 = model.request_episodic_decision("event1", None, context1, episode2)
    print("episode id:", episode2.episode_id)
    print("event id:", response1.event_id)
    print("chosen action:", response1.chosen_action_id)

    # episode1, event2
    context2 = '{"shared":{"F1": 4.0}, "_multi": [{"AF1": 2.0}, {"AF1": 3.0}]}'
    response2 = model.request_episodic_decision("event2", "event1", context2, episode1)
    print("episode id:", episode1.episode_id)
    print("event id:", response2.event_id)
    print("chosen action:", response2.chosen_action_id)

    # episode2, event2
    context2 = '{"shared":{"F2": 4.0}, "_multi": [{"AF2": 2.0}, {"AF2": 3.0}]}'
    response2 = model.request_episodic_decision("event2", "event1", context2, episode2)
    print("episode id:", episode2.episode_id)
    print("event id:", response2.event_id)
    print("chosen action:", response2.chosen_action_id)

    model.report_outcome(episode1.episode_id, "event1", 1.0)
    model.report_outcome(episode2.episode_id, "event2", 1.0)


if __name__ == "__main__":
    # basic_usage_cb()
    basic_usage_multistep()
