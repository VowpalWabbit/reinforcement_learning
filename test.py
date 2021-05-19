import rlclient_py

config = rlclient_py.configuration()
config.set("ApplicationID", "<appid>")
config.set("EventHubInteractionConnectionString", "<appid>")
config.set("EventHubObservationConnectionString", "<appid>")
config.set("model.vw.initial_command_line", "--slates --json --quiet --epsilon 0.0 --first_only --id N/A")
config.set("ModelBlobUri", "https://<storage>.blob.core.windows.net/mwt-models/current?sv=2017-07-29&sr=b&sig=<sig>&st=2018-06-26T09%3A00%3A55Z&se=2028-06-26T09%3A01%3A55Z&sp=r")
config.set("InitialExplorationEpsilon", "1.0")

def callback_func(code, string):
    print(string)

live_model = rlclient_py.live_model(config, callback_func)
response = live_model.choose_rank("tst", False)