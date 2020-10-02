using System;
using System.IO;
using Rl.Net;

namespace Rl.Net.Cli {
    internal class RobotJoint
    {
        public RobotJoint(string id, float temperature, float angularVelocity, float load, Func<float, float> rewardProbabilityDistribution, Func<float> randomSource)
        {
            this.Id = id;
            this.Temperature = temperature;
            this.AngularVelocity = angularVelocity;
            this.Load = load;
            this.RewardProbabilityDistribution = rewardProbabilityDistribution;
            this.RandomSource = randomSource;
        }

        public RobotJoint(string id, float temperature, float angularVelocity, float load, Func<float, float> rewardProbabilityDistribution)
            : this(id, temperature, angularVelocity, load, rewardProbabilityDistribution, () => (float)SimulatorStepProvider.RandomSource.NextDouble())
        {}

        public string Id
        {
            get;
            private set;
        }

        public float Temperature
        {
            get;
            private set;
        }

        public float AngularVelocity
        {
            get;
            private set;
        }

        public float Load
        {
            get;
            private set;
        }

        public Func<float, float> RewardProbabilityDistribution
        {
            get;
            private set;
        }

        private Func<float> RandomSource
        {
            get;
            set;
        }

        public string FeaturesJson
        {
            get
            {
                return $"\"id\":\"{this.Id}\", \"{this.Temperature}\":1, \"{this.AngularVelocity}\":1, \"{this.Load}\":1";
            }
        }

        public float GenerateOutcome(float observedFriction)
        {
            float sample = this.RandomSource();
            float target = this.RewardProbabilityDistribution(observedFriction);

            if (sample <= target)
            {
                return 1.0f;
            }
            else
            {
                return 0.0f;
            }
        }
    }
}