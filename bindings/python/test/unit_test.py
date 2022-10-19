import unittest
import rl_client

test_config_json = '''
  {
    "appid": "pythontest",
    "interaction.sender.implementation": "INTERACTION_FILE_SENDER",
    "observation.sender.implementation": "OBSERVATION_FILE_SENDER",
    "IsExplorationEnabled": true,
    "model.source": "FILE_MODEL_DATA",
    "model_file_loader.file_must_exist": false,
    "InitialExplorationEpsilon": 1.0,
    "model.backgroundrefresh": false,
    "protocol.version": 2
  }
'''

class ConfigTests(unittest.TestCase):
    @classmethod
    def setUpClass(self):
        self.config = rl_client.create_config_from_json(test_config_json)

    def test_set(self):
        self.config.set("CustomKey", "CustomValue")
        self.assertEqual(self.config.get("CustomKey", None), "CustomValue")

    def test_get(self):
        self.assertEqual(self.config.get(rl_client.constants.APP_ID, None), "pythontest")
        self.assertEqual(self.config.get(rl_client.constants.INTERACTION_SENDER_IMPLEMENTATION, None), "INTERACTION_FILE_SENDER")

    def test_get_default(self):
        self.assertEqual(self.config.get("UnsetKey", "DefaultValue"), "DefaultValue")

class LiveModelTests(unittest.TestCase):
    @classmethod
    def setUpClass(self):
        self.config = rl_client.create_config_from_json(test_config_json)

    def test_choose_rank(self):
        model = rl_client.LiveModel(self.config)

        event_id = "event_id"
        context = '{"_multi":[{},{}]}'
        model.choose_rank(context, event_id=event_id)

    def test_choose_rank_invalid_context(self):
        model = rl_client.LiveModel(self.config)

        event_id = "event_id"
        invalid_context = ""
        self.assertRaises(rl_client.RLException, model.choose_rank, event_id, invalid_context)

    def test_choose_rank_invalid_event_id(self):
        model = rl_client.LiveModel(self.config)

        invalid_event_id = ""
        context = '{"_multi":[{},{}]}'
        self.assertRaises(rl_client.RLException, model.choose_rank, invalid_event_id, context)

    def test_exception_contains_code(self):
        try:
            # This function should fail with an empty config.
            model = rl_client.LiveModel(rl_client.Configuration())
        except rl_client.RLException as e:
            self.assertTrue(hasattr(e, "code"))
            self.assertTrue(hasattr(e, "__str__"))
            # Return early so the fail is not reached.
            return

        self.fail("rl_client.RLException was not raised")

    def test_report_outcome(self):
        model = rl_client.LiveModel(self.config)

        event_id = "event_id"
        context = '{"_multi":[{},{}]}'
        model.choose_rank(context, event_id=event_id)
        model.report_outcome(event_id, 1.0)
        model.report_outcome(event_id,"{'result':'res'}")

    def test_report_outcome_no_connection(self):
        # Requires dependency injection for network.
        return

    def test_report_outcome_server_failure(self):
        # Requires dependency injection for network.
        return

    def test_async_error_callback(self):
        def on_error(self, error_code, error_message):
            print("Background error:")
            print(error_message)

        model = rl_client.LiveModel(self.config, on_error)
        # Requires dependency injection to fake a background failure, but we can at least make sure it loads the callback.
        return

if __name__ == '__main__':
    unittest.main()
