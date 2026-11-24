from contextlib import redirect_stdout
from io import StringIO
from pathlib import Path
import unittest

import flatbuffers
import numpy as np

import parser
import data
from reinforcement_learning.messages.flatbuff.v2 import CbEvent


def capture(function, *args):
    output = StringIO()
    with redirect_stdout(output):
        function(*args)
    return output.getvalue()


class ChosenActionTests(unittest.TestCase):
    def test_nonverbose_choice_is_logged_id_in_every_learning_mode(self):
        for mode in (
            parser.LearningModeType.Online,
            parser.LearningModeType.Apprentice,
            parser.LearningModeType.LoggingOnly,
        ):
            for deferred in (False, True):
                with self.subTest(mode=mode, deferred=deferred):
                    payload = data.serialize(
                        data.CbPayload(
                            context='{"_multi":[{},{}]}',
                            actions=[2, 1],
                            probs=[0.25, 0.75],
                            deferred=deferred,
                            model_id="model",
                            learning_mode=mode,
                        )
                    )
                    self.assertEqual(
                        capture(parser.parse_cb, payload, False),
                        "\tcb: actions:2 chosen_action:2 model:b'model' "
                        f"lm:{parser.learning_mode_name(mode)} deferred:{deferred}\n",
                    )
                    verbose = capture(parser.parse_cb, payload, True)
                    self.assertNotIn("chosen_action", verbose)
                    self.assertIn("actions:[2 1] probs: [0.25 0.75]", verbose)
                    self.assertIn('\t\tcontext: {"_multi":[{},{}]}', verbose)
                    record = {}
                    parser.fill_cb(payload, record)
                    self.assertEqual(
                        set(record),
                        {"actions", "probs", "model", "learning_mode", "deferred"},
                    )
                    np.testing.assert_array_equal(record["actions"], [2, 1])
                    np.testing.assert_array_equal(record["probs"], [0.25, 0.75])

    def test_empty_or_missing_actions_do_not_invent_a_choice(self):
        for include_vector in (False, True):
            with self.subTest(include_vector=include_vector):
                builder = flatbuffers.Builder(64)
                if include_vector:
                    CbEvent.StartActionIdsVector(builder, 0)
                    actions = builder.EndVector()
                CbEvent.Start(builder)
                if include_vector:
                    CbEvent.AddActionIds(builder, actions)
                builder.Finish(CbEvent.End(builder))
                self.assertEqual(
                    capture(parser.parse_cb, builder.Output(), False),
                    "\tcb: actions:0 chosen_action:None model:None "
                    "lm:Online deferred:False\n",
                )

    def test_real_joined_fixtures_preserve_logged_choices(self):
        root = (
            Path(__file__).resolve().parents[2]
            / "external_parser"
            / "unit_tests"
            / "test_files"
            / "valid_joined_logs"
        )
        for name in (
            "cb_joined_with_pdrop_05.fb",
            "cb_deferred_actions_w_activations_and_apprentice_10.fb",
        ):
            with self.subTest(fixture=name):
                path = root / name
                records = [
                    record["message"]
                    for record in parser.get_records(path)
                    if record["type"] == "CB"
                ]
                lines = [
                    line
                    for line in capture(parser.dump_file, path).splitlines()
                    if line.startswith("\tcb:")
                ]
                self.assertTrue(records)
                self.assertEqual(len(lines), len(records))
                for line, record in zip(lines, records):
                    self.assertIn(f" chosen_action:{record['actions'][0]} ", line)
                    self.assertIn(f"deferred:{record['deferred']}", line)
                    self.assertNotIn("chosen_action", record)
                if name == "cb_joined_with_pdrop_05.fb":
                    self.assertIn("chosen_action:2 ", lines[0])


if __name__ == "__main__":
    unittest.main()
