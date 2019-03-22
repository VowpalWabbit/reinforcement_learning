using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace Rl.Net.Cli
{
    using FeatureSet = System.Collections.Generic.Dictionary<string, int>;

    class PerfTestStepProvider : IDriverStepProvider<float>
    {

        private IList<string> Contexts { get; set; } = new List<string>();

        private int RingSize { get; set; } = 100;

        internal class PerfTestStep : IStepContext<float>
        {
            public string EventId
            {
                get;
                set;
            }

            public float Outcome {
                get;
                set;
            }

            public string DecisionContext { get; set; }
            

            private IEnumerable<ActionProbability> actionDistributionCache;
            public float GetOutcome(long actionIndex, IEnumerable<ActionProbability> actionDistribution)
            {
                return this.Outcome;
            }
        }

        public string Tag { get; set; } = "Id";

        public long DataSent { get; set; } = 0;

        public long MessagesSent { get; set; } = 0;

        public TimeSpan Duration { get; set; } = TimeSpan.FromSeconds(20);

        public PerfTestStepProvider(int actionsCount, int sharedFeatures, int actionFeatures)
        {
            FeatureSet shared = Enumerable.Range(1, sharedFeatures).ToDictionary(i => $"f{i}");
            List<Dictionary<string, FeatureSet>> actions = Enumerable.Range(1, actionsCount).Select(i => new Dictionary<string, FeatureSet> { { $"a{i}", Enumerable.Range(1, actionFeatures).ToDictionary(j => $"af{j}") } }).ToList();
            var context = new Dictionary<string, object> { { "GUser", shared }, { "_multi", actions } };
            for (int i = 0; i < this.RingSize; ++i)
            {
                var message = JsonConvert.SerializeObject(context);
                Contexts.Add(message);
                UpdateFeatures(shared, actions);
            }
        }

        public IEnumerator<IStepContext<float>> GetEnumerator()
        {
            StatisticsCalculator stats = new StatisticsCalculator();

            var timer = Stopwatch.StartNew();
            while (timer.ElapsedMilliseconds < this.Duration.TotalMilliseconds)
            {
                DataSent += Encoding.UTF8.GetByteCount(this.Contexts[(int)MessagesSent % this.RingSize]);
                MessagesSent++;
                var step = new PerfTestStep
                {
                    EventId = $"{Tag}-{MessagesSent}",
                    DecisionContext = this.Contexts[(int)MessagesSent % this.RingSize],
                    Outcome = MessagesSent % 100
                };

                yield return step;
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return this.GetEnumerator();
        }

        private void UpdateFeatures(FeatureSet shared, List<Dictionary<string, FeatureSet>> actions)
        {
    /*        foreach (var v in shared.Values)
            {
                v++;
            }
            foreach (var a in actions)
            {
                foreach (var k in a.Keys)
                {
                    foreach (var f in a[k].Keys)
                    {
                        a[k][f]++;
                    }
                }
            }*/

        }
    }
}
