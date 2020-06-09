using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using Rl.Net;

namespace Rl.Net.Cli
{
    public enum Topic : long
    {
        HerbGarden,
        MachineLearning,
        Soccer,
        SpaceExploration
    }

    internal static class ActionDistributionExtensions
    {
        public static string ToDistributionString(this IEnumerable<ActionProbability> actionDistribution)
        {
            StringBuilder stringBuilder = new StringBuilder("(");

            foreach (ActionProbability actionProbability in actionDistribution)
            {
                stringBuilder.Append($"[{actionProbability.ActionIndex}, {actionProbability.Probability}]");
            }

            stringBuilder.Append(')');

            return stringBuilder.ToString();
        }
    }

    internal class SimulatorStepProvider : IDriverStepProvider<float>
    {
        public static readonly Random RandomSource = new Random();
        public const int InfinitySteps = -1;
        internal const int DefaultSlatesSlotCount = 2;

        private static Func<Topic, float> GenerateRewardDistribution(params float[] probabilities)
        {
            Dictionary<Topic, float> topicProbabilities = new Dictionary<Topic, float>
            {
                { Topic.HerbGarden, probabilities[0] },
                { Topic.MachineLearning, probabilities[1] },
                { Topic.Soccer, probabilities[2] },
                { Topic.SpaceExploration, probabilities[3] },
            };

            return (topic) => topicProbabilities[topic];
        }

        internal static Person[] People = new[]
        {
            new Person("rnc", "engineering", "hiking", "spock", GenerateRewardDistribution(0.03f, 0.1f ,0.05f, 0.15f)),
            new Person("mk", "psychology", "kids", "7of9", GenerateRewardDistribution(0.3f, 0.1f, 0.08f, 0.1f))
        };

        private static Person GetRandomPerson()
        {
            int index = RandomSource.Next(People.Length);

            return People[index];
        }

        private readonly int steps;
        private readonly int slots;

        public SimulatorStepProvider(int steps, int slots)
        {
            this.steps = steps;
            this.slots = slots;
        }

        public IEnumerator<IStepContext<float>> GetEnumerator()
        {
            StatisticsCalculator stats = new StatisticsCalculator();

            int stepsSoFar = 0;
            while (steps < 0 || (stepsSoFar++ < steps))
            {
                SimulatorStep step = new SimulatorStep
                {
                    StatisticsCalculator = stats,
                    EventId = Guid.NewGuid().ToString(),
                    Person = GetRandomPerson(),
                    SlotCount = this.slots, 
                };

                yield return step;

                step.Record(stats);
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return this.GetEnumerator();
        }


        internal class SimulatorStep : IStepContext<float>
        {
            internal static readonly Topic[] ActionSet = new[] { Topic.HerbGarden, Topic.MachineLearning, Topic.Soccer, Topic.SpaceExploration };

            private static readonly string ActionsJson = string.Join(",", ActionSet.Select(topic => $"{{ \"TAction\": {{ \"topic\": \"{topic}\" }} }}"));
            private string SlotsJson => string.Join(",", Enumerable.Range(0, SlotCount).Select(slotId => $"{{ \"slot_id\": \"__{slotId}\" }}"));

            public StatisticsCalculator StatisticsCalculator
            {
                get;
                set;
            }

            public string EventId
            {
                get;
                set;
            }

            public Person Person
            {
                get;
                set;
            }

            public Topic? DecisionCache
            {
                get;
                set;
            }

            public string ActionDistributionString
            {
                get
                {
                    if (this.actionDistributionCache == null)
                    {
                        return null;
                    }

                    return this.actionDistributionCache.ToDistributionString();
                }
            }

            public int SlotCount
            {
                get;
                set;
            }

            public string DecisionContext => $"{{ { this.Person.FeaturesJson }, \"_multi\": [{ ActionsJson }] }}";
            public string SlatesContext => $"{{ { this.Person.FeaturesJson }, \"_slots\": [{SlotsJson}], \"_multi\":[{ActionsJson}] }}";


            private float? outcomeCache;
            private IEnumerable<ActionProbability> actionDistributionCache;
            public float GetOutcome(long actionIndex, IEnumerable<ActionProbability> actionDistribution)
            {
                if (!this.outcomeCache.HasValue)
                {
                    this.DecisionCache = (Topic)actionIndex;
                    this.actionDistributionCache = actionDistribution;
                    this.outcomeCache = this.Person.GenerateOutcome(this.DecisionCache.Value);
                }

                return this.outcomeCache.Value;
            }

            public void Record(StatisticsCalculator statisticsCalculator)
            {
                statisticsCalculator.Record(this.Person, this.DecisionCache.Value, this.outcomeCache.Value);

                Console.WriteLine($" {statisticsCalculator.TotalActions}, ctxt, {this.Person.Id}, action, {this.DecisionCache.Value}, outcome, {this.outcomeCache.Value}, dist, {this.ActionDistributionString}, {statisticsCalculator.GetStats(this.Person, this.DecisionCache.Value)}");
            }

            public float GetOutcome(int[] actionIndexes, float[] probabilities)
            {
                throw new NotImplementedException();
            }
        }
    }

    internal class RLSimulator
    {
        private RLDriver driver;

        public RLSimulator(LiveModel liveModel, bool useSlates)
        {
            this.driver = new RLDriver(liveModel, useSlates);
        }

        public TimeSpan StepInterval
        {
            get
            {
                return this.driver.StepInterval;
            }
            set
            {
                this.driver.StepInterval = value;
            }
        }

        public void Run(int steps = SimulatorStepProvider.InfinitySteps, int slots = SimulatorStepProvider.DefaultSlatesSlotCount)
        {
            SimulatorStepProvider stepProvider = new SimulatorStepProvider(steps, slots);

            this.driver.Run(stepProvider);
        }

        public event EventHandler<ApiStatus> OnError
        {
            add
            {
                this.driver.OnError += value;
            }
            remove
            {
                this.driver.OnError -= value;
            }
        }
    }
}