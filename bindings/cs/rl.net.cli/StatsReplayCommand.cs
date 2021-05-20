using System;
using System;
using System.Collections;
using System.Collections.Generic;
using CommandLine;
using System.Text;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System.IO;
using System.Linq;

namespace Rl.Net.Cli
{
    class RotatingActionHash
    {
        readonly int[] foundHashes = new int[1000];
        readonly string[] actions = new string[1000];
        int insertHead;

        public void Add(string str)
        {
            int hash = str.GetHashCode();
            if (hash == 0)
                hash = ("_" + str).GetHashCode();

            foundHashes[insertHead] = hash;
            actions[insertHead] = str;
            insertHead = (insertHead + 1) % foundHashes.Length;
        }

        public string Get(int distance)
        {
            if (distance < 1)
                return null;
            int idx = insertHead - distance;
            if (idx < 0)
                idx += foundHashes.Length;
            return actions[idx];
        }
    }

    class StatsFileStepProvider: IDriverStepProvider<float?>
    {
        private readonly Random rand;
        private readonly int steps;
        private readonly string[] actionList;
        private readonly RotatingActionHash recentActions;
        private readonly Histogram sharedCtxFeatures;
        private readonly Histogram actionFeatures;
        private readonly Histogram actionCount;
        private readonly HashedDistanceHistogram actions;
        private readonly double pObs;
        private readonly double pAct;

        class StatsStepContext : IStepContext<float?>
        {
            public float? OutcomeValue { get; set; }

            public string EventId { get; set; }

            public bool UseActivations { get; set; }

            public string DecisionContext { get; set; }

            public bool IsStepActivated { get; set; }

            public ActionFlags DecisionFlags => UseActivations ? ActionFlags.Deferred : ActionFlags.Default;

            public float? GetOutcome(long actionIndex, IEnumerable<ActionProbability> actionDistribution)
            {
                return OutcomeValue;
            }

            public float? GetOutcome(int[] actionIndexes, float[] probabilities)
            {
                throw new NotImplementedException();
            }

            public float? GetSlatesOutcome(int[] actionIndexes, float[] probabilities)
            {
                throw new NotImplementedException();
            }

            public float? GetContinuousActionOutcome(float action, float pdfValue)
            {
                throw new NotImplementedException();
            }

            public string CcbContext => throw new NotImplementedException();

            public string SlatesContext => throw new NotImplementedException();

            public string ContinuousActionContext => throw new NotImplementedException();

        }


        public StatsFileStepProvider(string statsConfig, int seed, int steps)
        {
            this.rand = new Random(seed);
            this.steps = steps;

            var obj  = JObject.Parse(File.ReadAllText(statsConfig));

            int evtCount = (int)obj["EventCount"];
            int obsCount = (int)obj["ObservationCount"];
            int activationCount = (int)obj["ActivationCount"];

            this.pObs = obsCount / (double)evtCount;
            this.pAct = activationCount / (double)evtCount;

            this.sharedCtxFeatures = new Histogram((JObject)obj["SharedContextFeaturesDistribution"]);
            this.actionFeatures = new Histogram((JObject)obj["ActionFeaturesDistribution"]);
            this.actionCount = new Histogram((JObject)obj["ActionCountDistribution"]);
            this.actions = new HashedDistanceHistogram((JObject)obj["RepetitionHistogram"]);
            this.actionList = CreateActionList(actionFeatures, actions);
            this.recentActions = new RotatingActionHash();

        }

        public IEnumerator<IStepContext<float?>> GetEnumerator()
        {
            int stepsSoFar = 0;
            while (steps < 0 || (stepsSoFar++ < steps))
            {
                float? outcome = rand.NextDouble() <= pObs ? 1f : (float?)null;
                bool activation = outcome.HasValue || rand.NextDouble() <= pAct;
                Console.WriteLine($"has outcome? {outcome.HasValue} activated? {activation}");

                yield return new StatsStepContext()
                {
                    EventId = rand.Next().ToString() + rand.Next().ToString(),
                    DecisionContext = MakeDecision(),
                    OutcomeValue = outcome,
                    UseActivations = pAct > 0,
                    IsStepActivated = activation
                };
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return this.GetEnumerator();
        }

        string MakeDecision()
        {
            string sharedCtx = MakeFeatureVector((int)sharedCtxFeatures.Sample(rand));
            int evtActions = (int)actionCount.Sample(rand);

            var sb = new StringBuilder();
            sb
                .Append("{")
                .Append(sharedCtx)
                .Append(",")
                .Append("\"_multi\": [");
            for (int j = 0; j < evtActions; ++j)
            {
                int distance = actions.Sample(rand);
                string action = null;
                if (distance == -1 || (action = recentActions.Get(distance)) == null)
                {
                    //If distance == -1 This means the action was not found. Unfortunately this is really hard to model since
                    //we didn't keep track of action distribution so the sequence we produce is unlikely to
                    //fit this. In this case we draw at random - this happens about 4% of the time
                    //The action history needs to be populated before we can emit something
                    action = actionList[rand.Next(0, actionList.Length)];
                }
                recentActions.Add(action);
                if (j > 0) sb.Append(",");
                sb.Append(action);
            }
            sb.Append("]");

           
            sb.Append("}");
            return sb.ToString();
        }

        string MakeFeatureVector(int count)
        {
            var dict = new Dictionary<string, float>();
            while (dict.Count < count)
            {
                var featureName = $"feature_{rand.Next(count * 10)}";
                if (!dict.ContainsKey(featureName))
                {
                    dict[featureName] = (float)rand.NextDouble();
                }
            }
            return $" \"g\": {JsonConvert.SerializeObject(dict)}";
        }
        private string[] CreateActionList(Histogram actionFeatures, HashedDistanceHistogram actions)
        {
            HashSet<string> actionSet = new HashSet<string>();
            while (actionSet.Count < actions.UniqueActions)
            {
                int featureCount = (int)actionFeatures.Sample(rand);
                var ac = $"{{ {MakeFeatureVector(featureCount)} }}";
                if (!actionSet.Contains(ac))
                {
                    actionSet.Add(ac);
                }
            }

            var actionList = actionSet.ToArray();
            return actionList;
        }
    }
    [Verb("stats", HelpText = "Run statistical simulator")]
    class StatsReplayCommand : CommandBase
    {
        [Option(longName: "sleep", HelpText = "sleep interval in milliseconds", Required = false, Default = 50)]
        public int SleepIntervalMs { get; set; }

        [Option(longName: "statsConfig", HelpText = "JSON file with statistical model to be used", Required = true)]
        public string StatsConfig{ get; set; }

        [Option(longName: "steps", HelpText = "Number of steps", Required = false, Default = SimulatorStepProvider.InfinitySteps)]
        public int Steps { get; set; }

        [Option(longName: "seed", HelpText = "Initial Random Seed", Required = false, Default = 1738277)]
        public int Seed { get; set; }

        public override void Run()
        {
            LiveModel liveModel = Helpers.CreateLiveModelOrExit(this.ConfigPath);

            RLDriver rlDriver = new RLDriver(liveModel, loopKind: this.GetLoopKind());
            rlDriver.StepInterval = TimeSpan.FromMilliseconds(this.SleepIntervalMs);

            rlDriver.Run(new StatsFileStepProvider(StatsConfig, Seed, Steps));
        }
    }
}
