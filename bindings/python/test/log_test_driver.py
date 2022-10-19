import argparse
import numbers
import json
import sys
import os

import rl_client


def on_error(self, error_code, error_message):
    print("Background error:")
    print(error_message)


def load_config_from_json(file_name):
    with open(file_name, "r") as config_file:
        return rl_client.create_config_from_json(config_file.read())


def process_cmd_line(args):
    parser = argparse.ArgumentParser()
    parser.add_argument("--json_config", help="client json config", required=True)
    parser.add_argument("--log_file", help="log file to replay", required=True)
    return parser.parse_args(args)


def main(args):
    options = process_cmd_line(args)

    config = load_config_from_json(options.json_config)
    model = rl_client.LiveModel(config, on_error)
    model.init()

    with open(options.log_file) as fp:
        for count, line in enumerate(fp):
            current_example = json.loads(line)

            context_json = json.dumps(current_example["c"])
            response = model.choose_rank(
                context_json, event_id=current_example["EventId"]
            )

            if "o" in current_example:
                for observation in current_example["o"]:
                    outcome = (
                        observation["v"]
                        if isinstance(observation["v"], numbers.Real)
                        else json.dumps(observation["v"])
                    )
                    model.report_outcome(observation["EventId"], outcome)


if __name__ == "__main__":
    main(sys.argv[1:])
