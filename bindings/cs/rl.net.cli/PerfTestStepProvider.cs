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
        public class Statistics
        {
            public int Messages { get; private set; }

            public int Bytes { get; private set; }

            public Stopwatch Timer { get; } = Stopwatch.StartNew();

            public long ElapsedMs { get; private set; }

            public void Update(PerfTestStep step)
            {
                Bytes += Encoding.UTF8.GetByteCount(step.DecisionContext) + Encoding.UTF8.GetByteCount(step.EventId);
                Messages++;
                ElapsedMs = Timer.ElapsedMilliseconds;
            }

            public void Print()
            {
                Console.WriteLine($"Data sent: {this.Bytes / 1024} Kb");
                Console.WriteLine($"Throughput: {this.Bytes / (1024 * this.ElapsedMs / 1000)} Kb / s");
                Console.WriteLine($"Messages sent: {this.Messages}");
                Console.WriteLine($"Qps: {this.Messages / (this.ElapsedMs / 1000)}");
            }
        }

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
            
            public float GetOutcome(long actionIndex, IEnumerable<ActionProbability> actionDistribution)
            {
                return this.Outcome;
            }
        }

        public string Tag { get; set; } = "Id";

        public TimeSpan Duration { get; set; } = TimeSpan.FromSeconds(20);

        public Statistics Stats { get; private set; }

        public PerfTestStepProvider(int actionsCount, int sharedFeatures, int actionFeatures)
        {
            for (int i = 0; i < this.RingSize; ++i)
            {
                FeatureSet shared = Enumerable.Range(1, sharedFeatures).ToDictionary(f => $"f{f}", f => f + i);
                List<Dictionary<string, FeatureSet>> actions = Enumerable.Range(1, actionsCount).Select(a => new Dictionary<string, FeatureSet> { { $"a{a}", Enumerable.Range(1, actionFeatures).ToDictionary(f => $"af{f}", f => f + i) } }).ToList();
                var context = new Dictionary<string, object> { { "GUser", shared }, { "_multi", actions } };
                var message = JsonConvert.SerializeObject(context);
                Contexts.Add(message);
            }
        }

        public IEnumerator<IStepContext<float>> GetEnumerator()
        {
            this.Stats = new Statistics();
            while (this.Stats.ElapsedMs < this.Duration.TotalMilliseconds)
            {
                var step = new PerfTestStep
                {
                    EventId = $"{Tag}-{this.Stats.Messages}",
                    DecisionContext = this.Contexts[this.Stats.Messages % this.RingSize],
                    Outcome = this.Stats.Messages % 100
                };

                yield return step;
                this.Stats.Update(step);
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return this.GetEnumerator();
        }
    }
}
