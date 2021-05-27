import argparse
import random
import time

import rl_client


def on_error(self, error_code, error_message):
    print("Background error:")
    print(error_message)

def load_config_from_json(file_name):
    with open(file_name, 'r') as config_file:
        return rl_client.create_config_from_json(config_file.read())

class person:
    def __init__(self, id, major, hobby, fav_char, prob):
        self._id = id
        self._major = major
        self._hobby = hobby
        self._favorite_character = fav_char
        self._topic_click_probability = prob

    def get_features(self):
        return '"User":{{"id":"{}","major":"{}","hobby":"{}","favorite_character":"{}"}}'.format(self._id, self._major, self._hobby, self._favorite_character)

    def get_outcome(self, chosen_action):
        norm_draw_val = random.random()
        click_prob = self._topic_click_probability[chosen_action]
        if (norm_draw_val <= click_prob):
            return 1.0
        else:
            return 0.0

class rl_sim:
    def __init__(self, args):
        self._options = args

        self.config = load_config_from_json(self._options.json_config)
        self._rl_client = rl_client.LiveModel(self.config, on_error)

        tp1 = {'HerbGarden': 0.3, "MachineLearning": 0.2 }
        tp2 = {'HerbGarden': 0.1, "MachineLearning": 0.4 }

        self._actions = ['HerbGarden', 'MachineLearning']
        self._people = [
            person('rnc', 'engineering', 'hiking', 'spock', tp1),
            person('mk', 'psychology', 'kids', '7of9', tp2)]

    def loop(self):
        round = 0
        stats = {person._id : [[0,0] for _ in self._actions] for person in self._people}
        ctr = [0,0]
        while round != self._options.num_rounds:
            try:
                round += 1

                # random.sample() returns a list
                person = random.sample(self._people, 1)[0]

                # create context
                shared_features = person.get_features()
                action_features = '"_multi":[' + ','.join('{"a":{"topic":"'+action+'"}}' for action in self._actions) + ']'
                context = '{' + shared_features + ',' + action_features + '}'

                response = self._rl_client.choose_rank(context)

                stats[person._id][response.chosen_action_id][1] += 1
                ctr[1] += 1

                outcome = person.get_outcome(self._actions[response.chosen_action_id])
                if outcome != 0:
                    self._rl_client.report_outcome(response.event_id, outcome)
                    ctr[0] += 1
                    stats[person._id][response.chosen_action_id][0] += 1

                print('Round: {}, ctr: {:.1%}'.format(round, ctr[0]/ctr[1]), stats)

                time.sleep(0.1)
            except rl_client.rl_exception as e:
                print(e)
                time.sleep(2)
                continue


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--json_config', help='client json config', required=True)
    parser.add_argument('--num_rounds', help='number of rounds (default: infinity)', type=int, default=-1)

    vm = parser.parse_args()
    sim = rl_sim(vm)
    sim.loop()
