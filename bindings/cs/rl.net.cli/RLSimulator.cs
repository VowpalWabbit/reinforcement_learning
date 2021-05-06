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

        // continuous actions
        private static List<float> jointFriction = new List<float> { 25.4f, 41.2f, 66.5f, 81.9f};
        private static Func<float, float> GenerateContinuousRewardDistribution(params float[] probabilities)
        {
            Dictionary<float, float> jointFrictionProbabilities = new Dictionary<float, float>
            {
                { jointFriction[0], probabilities[0] },
                { jointFriction[1], probabilities[1] },
                { jointFriction[2], probabilities[2] },
                { jointFriction[3], probabilities[3] },
            };

            return (observedFriction) => 
            {
                float prob = 0;
                // figure out which bucket from our pre-set frictions the observed_friction
                // falls into to and get it's probability
                foreach (KeyValuePair<float, float> frictionProb in jointFrictionProbabilities)
                {
                    if (observedFriction >= frictionProb.Key)
                    {
                        prob = frictionProb.Value;
                    }
                }

                return prob;
            };
        }

        internal static RobotJoint[] RobotJoints = new[]
        {
            new RobotJoint("j1", 20.3f, 102.4f, -10.2f, GenerateContinuousRewardDistribution(0.03f, 0.1f ,0.05f, 0.15f)),
            new RobotJoint("j2", 40.6f, 30.8f, 98.5f, GenerateContinuousRewardDistribution(0.3f, 0.1f, 0.08f, 0.1f))
        };

        private static RobotJoint GetRandomRobotJoint()
        {
            int index = RandomSource.Next(RobotJoints.Length);

            return RobotJoints[index];
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
            StatisticsCalculator<Person, int> stats = new StatisticsCalculator<Person, int>();
            StatisticsCalculator<RobotJoint, float> statsCA = new StatisticsCalculator<RobotJoint, float>();

            int stepsSoFar = 0;
            while (steps < 0 || (stepsSoFar++ < steps))
            {
                SimulatorStep step = new SimulatorStep
                {
                    StatisticsCalculator = stats,
                    ContinuousStatisticsCalculator = statsCA,
                    EventId = Guid.NewGuid().ToString(),
                    Person = GetRandomPerson(),
                    RobotJoint = GetRandomRobotJoint(),
                    SlotCount = this.slots,
                };

                yield return step;

                step.RecordContinuousAction(statsCA);
                step.Record(stats);
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return this.GetEnumerator();
        }


        internal class SimulatorStep : IStepContext<float>
        {
            internal static readonly (Topic topic, int slot_id)[] ActionSet = new[] { (Topic.HerbGarden, 0), (Topic.MachineLearning, 0), (Topic.Soccer, 1), (Topic.SpaceExploration, 1) };

            private static readonly string ActionsJsonWithSlotId = string.Join(",", ActionSet.Select(action => $"{{ \"TAction\": {{ \"topic\": \"{action.topic}\" }}, \"_slot_id\": {action.slot_id} }}"));
            private static readonly string ActionsJsonWithoutSlotId  = string.Join(",", ActionSet.Select(action => $"{{ \"TAction\": {{ \"topic\": \"{action.topic}\" }} }}"));
            private string SlotsJson => string.Join(",", Enumerable.Range(0, SlotCount).Select(slotId => $"{{ \"slot_id\": \"__{slotId}\" }}"));

            public StatisticsCalculator<Person, int> StatisticsCalculator
            {
                get;
                set;
            }

            public StatisticsCalculator<RobotJoint, float> ContinuousStatisticsCalculator
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

            public RobotJoint RobotJoint
            {
                get;
                set;
            }

            public Topic? DecisionCache
            {
                get;
                set;
            }

            public float? ContinuousActionCache
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

            public string DecisionContext => $"{{ { this.Person.FeaturesJson }, \"_multi\": [{ ActionsJsonWithoutSlotId }] }}";
            public string CcbContext => $"{{ { this.Person.FeaturesJson }, \"_multi\": [{ ActionsJsonWithSlotId }] }}";
            public string SlatesContext => $"{{ { this.Person.FeaturesJson }, \"_multi\":[{ActionsJsonWithSlotId}], \"_slots\": [{SlotsJson}] }}";
            public string ContinuousActionContext => $"{{ { this.RobotJoint.FeaturesJson } }}";

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

            public void Record(StatisticsCalculator<Person, int> statisticsCalculator)
            {
                if (this.DecisionCache.HasValue)
                {
                    statisticsCalculator.Record(this.Person, (int)this.DecisionCache.Value, this.outcomeCache.Value);
                    Console.WriteLine($" {statisticsCalculator.TotalActions}, ctxt, {this.Person.Id}, action, {this.DecisionCache.Value}, outcome, {this.outcomeCache.Value}, dist, {this.ActionDistributionString}, {statisticsCalculator.GetStats(this.Person, (int)this.DecisionCache.Value)}");
                }
            }

            public void RecordContinuousAction(StatisticsCalculator<RobotJoint, float> statisticsCalculator)
            {
                if (this.ContinuousActionCache.HasValue)
                {
                    statisticsCalculator.Record(this.RobotJoint, this.ContinuousActionCache.Value, this.outcomeCache.Value);
                    Console.WriteLine($" {statisticsCalculator.TotalActions}, ctxt, {this.RobotJoint.Id}, action, {this.ContinuousActionCache.Value}, outcome, {this.outcomeCache.Value}, dist, {this.ActionDistributionString}, {statisticsCalculator.GetStats(this.RobotJoint, this.ContinuousActionCache.Value)}");
                }
            }

            public float GetOutcome(int[] actionIndexes, float[] probabilities)
            {
                throw new NotImplementedException();
            }

            public float GetSlatesOutcome(int[] actionIndexes, float[] probabilities)
            {
                if (!this.outcomeCache.HasValue)
                {
                    this.DecisionCache = (Topic)0;
                    this.actionDistributionCache = new List<ActionProbability>();
                    this.outcomeCache = actionIndexes
                        .Zip(probabilities, (int action, float prob) => this.Person.GenerateOutcome(ActionSet[action].topic))
                        .Aggregate((float)0, (acc, x) => acc + x);
                }

                return this.outcomeCache.Value;
            }

            public float GetContinuousActionOutcome(float action, float pdf_value)
            {
                if (!this.outcomeCache.HasValue)
                {
                    this.ContinuousActionCache = action;
                    this.outcomeCache = this.RobotJoint.GenerateOutcome(this.ContinuousActionCache.Value);
                }

                return this.outcomeCache.Value;
            }
        }
    }

    internal class RLSimulator
    {
        private RLDriver driver;

        public RLSimulator(LiveModel liveModel, LoopKind loopKind)
        {
            this.driver = new RLDriver(liveModel, loopKind);
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