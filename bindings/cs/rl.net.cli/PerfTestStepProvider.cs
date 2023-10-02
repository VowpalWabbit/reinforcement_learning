using Newtonsoft.Json;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace Rl.Net.Cli
{
    using FeatureSet = Dictionary<string, int>;

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

            public float Outcome
            {
                get;
                set;
            }

            public string DecisionContext { get; set; }
            public string CcbContext { get; set; }
            public string SlatesContext { get => throw new NotImplementedException(); set => throw new NotImplementedException(); }
            public string ContinuousActionContext { get => throw new NotImplementedException(); set => throw new NotImplementedException(); }

            public ActionFlags DecisionFlags => ActionFlags.Default;

            public bool IsStepActivated => throw new NotImplementedException();

            public float GetOutcome(long actionIndex, IEnumerable<ActionProbability> actionDistribution)
            {
                return this.Outcome;
            }

            public float GetOutcome(int[] actionIndexes, float[] probabilities)
            {
                return this.Outcome;
            }

            public float GetSlatesOutcome(int[] actionIndexes, float[] probabilities)
            {
                throw new NotImplementedException();
            }

            public float GetContinuousActionOutcome(float action, float pdfValue)
            {
                throw new NotImplementedException();
            }
        }

        public string Tag { get; set; } = "Id";

        public TimeSpan Duration { get; set; } = TimeSpan.FromSeconds(20);

        public int NumLines { get; set; } = 0;

        public int NumLinesProcessed { get; set; } = 0;

        public double DataSize { get; set; } = 0;

        public Statistics Stats { get; private set; }

        public PerfTestStepProvider(int actionsCount, int sharedFeatures, int actionFeatures, int numSlots)
        {
            for (int i = 0; i < this.RingSize; ++i)
            {
                FeatureSet shared = Enumerable.Range(1, sharedFeatures).ToDictionary(f => $"f{f}", f => f + i);
                List<Dictionary<string, FeatureSet>> actions = Enumerable.Range(1, actionsCount).Select(a => new Dictionary<string, FeatureSet> { { $"a{a}", Enumerable.Range(1, actionFeatures).ToDictionary(f => $"af{f}", f => f + i) } }).ToList();
                var context = new Dictionary<string, object> { { "GUser", shared }, { "_multi", actions } };
                if (numSlots > 0)
                {
                    List<Dictionary<string, string>> slots = new List<Dictionary<string, string>>();
                    for (int j = 0; j < numSlots; ++j)
                    {
                        slots.Add(new Dictionary<string, string>
                        {
                            { "_id", "slot" + j }
                        });
                    }
                    context.Add("_slots", slots);
                }
                var message = JsonConvert.SerializeObject(context);
                Contexts.Add(message);
                Console.Out.WriteLine(message);
            }
        }

        public PerfTestStepProvider(string filePath)
        {
            using (StreamReader sr = new StreamReader(filePath))
            {
                string line;
                while ((line = sr.ReadLine()) != null)
                {
                    Contexts.Add(line);
                }
            }
            this.NumLines = Contexts.Count;
        }

        public IEnumerator<IStepContext<float>> GetEnumerator()
        {
            this.Stats = new Statistics();
            while (true)
            {
                if (this.NumLines > 0)
                {
                    if (this.NumLines == this.NumLinesProcessed)
                    {
                        break;
                    }
                    this.NumLinesProcessed++;
                }
                else if (!(this.Stats.Bytes < this.DataSize || this.Stats.ElapsedMs < this.Duration.TotalMilliseconds))
                {
                    break;
                }
                var step = new PerfTestStep
                {
                    EventId = $"{Tag}-{this.Stats.Messages}",
                    DecisionContext = this.Contexts[this.Stats.Messages % this.RingSize],
                    Outcome = this.Stats.Messages % 100
                };

                yield return step;
                this.Stats.Update(Encoding.UTF8.GetByteCount(step.DecisionContext) + Encoding.UTF8.GetByteCount(step.EventId));
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return this.GetEnumerator();
        }
    }
}
