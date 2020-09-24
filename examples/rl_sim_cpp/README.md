This simulates a news topic recommendation based on user's major, hobby and favorite characters.

### Build Target
Run `make rl_sim_cpp.out` at the repo root directory

### Update client.json
1. Create a loop in Personalizer following the [documentation](https://docs.microsoft.com/en-us/azure/cognitive-services/personalizer/how-to-create-resource)
2. Get the key and endpoint from Personalizer loop, then use [Personalizer API](https://westus2.dev.cognitive.microsoft.com/docs/services/personalizer-api/operations/GetClientConfiguration) to fetch client configuration.
3. After getting the configurations, the keys need to be capitalized. For example, the response from Personalizer API is
```
  {
    "applicationID": "...",
    "eventHubInteractionConnectionString": "...",
    "eventHubObservationConnectionString": "...",
    "modelBlobUri": "...",
    "initialExplorationEpsilon": 1,
    "learningMode": "..."
  }
```
needs to be replaced to:
```
{
  "ApplicationID": "",
  "EventHubInteractionConnectionString": "...",
  "EventHubObservationConnectionString": "...",
  "ModelBlobUri": "...",
  "InitialExplorationEpsilon": 1,
  "LearningMode": "...",
}
```

### Run the code

#### CB mode
CB mode can be run directly. 

From root directory, run
```
./build/examples/rl_sim_cpp/rl_sim_cpp.out -j examples/rl_sim_cpp/client.json
```

#### CA mode
Continuous Actions mode

1. In `client.json` file, add in:
```
"model.vw.initial_command_line": "--cats <num_actions> --min_value <min_value> --max_value <max_value> --bandwidth <bandwidth>",
"protocol.version": 2,
"continuous.actions.enabled": true
```

2. From root directory, run 
```
./build/examples/rl_sim_cpp/rl_sim_cpp.out -j examples/rl_sim_cpp/client.json --ca true
```

#### CCB mode
1. In `client.json` file, add in 
```
"model.vw.initial_command_line": "--ccb_explore_adf --epsilon 0.2"
```

2. From root directory, run 
```
./build/examples/rl_sim_cpp/rl_sim_cpp.out -j examples/rl_sim_cpp/client.json --ccb true
```

#### Slates mode
1. In `client.json` file, add in
```
"model.vw.initial_command_line": "--slates"
```

2. In Personalizer loop, go to `Model and learning settings`, change learning settings to include `--slates`

3. From root directory, run 
```
./build/examples/rl_sim_cpp/rl_sim_cpp.out -j examples/rl_sim_cpp/client.json --slates true
```