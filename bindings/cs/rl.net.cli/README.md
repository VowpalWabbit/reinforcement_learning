# Overview

Rl.Net.Cli is sample command line utility showcasing how to use rlclientlib's C# binding.
It follows the best practices when using it.

Beyond that, it provides a suite of commands that helps test and develop rlclientlib itself.
In particular, the simulation modes are useful to generate synthetic data.


## Stats

The stats command is used to reproduce a workload data using a JSON file that has some
summary statistics about the dataset. The generated data has no learning value, it's only
good to performance test an ingestion pipeline.

### JSON file

```json
{
    "EventCount": 100,
    "ObservationCount": 10,
    "ActivationCount": 15,
    "SharedContextFeaturesDistribution": { ... },
    "ActionFeaturesDistribution": { ... },
    "ActionCountDistribution": { ... },
    "RepetitionHistogram": { ... }
}
```

`SharedContextFeaturesDistribution`, `ActionFeaturesDistribution` and `ActionCountDistribution` are histograms with  the following format:

```json
{
    "Min": 1.5,
    "Max": 10,
    "Counts": [
        10,
        20,
        20,
        20
    ]
}
```

Min and max values are the inclusive bounds.

`RepetitionHistogram` has the following format:

```json
{
    "UniqueActions": 20,
    "Entries": 10,
    "Hist": [ 10, 20, 10, 20]
}
```

`UniqueActions` tells how many unique actions to use across the whole data set. This value is mandatory and
it's currently not possible to operate with per-decision randomly-generated actions.

`Entries` and `Hist` form a histogram of the distance from an action to its previous ocurrence
in the decision stream. This is used to model action diversity and simulate the advantages of
dedup. This two values are optional and if not provides, actions will be uniformly sampled.

`Entries` value can be bigger than the sum of all `Hist` entries and it reflects the case
when an action was not found within the measured repetition window.

### Generation process

If `ActivationCount` is non-zero, the simulator will produce deferred decisions.
Each even will be activated with `ActivationCount / EventCount` probability.

Each even will receive an observation with `ObservationCount / EventCount` probability.

The simulator created a fixed set of actions with size defined by `RepetitionHistogram::UniqueActions`.
Sample from `ActionFeaturesDistribution` for the number of features on each action.

Each decision context is constructed in the following way:

- Sample `SharedContextFeaturesDistribution` for the number of shared features.
- Sample `ActionCountDistribution` for the number of actions.

For each action we need to generate:
- Sample `RepetitionHistogram` for the distance to the previous action and, if positive, use it.
- If not positive, or `RepetitionHistogram:Hist` is missing, uniformly sample from the action set.
