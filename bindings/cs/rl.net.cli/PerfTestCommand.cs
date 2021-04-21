using CommandLine;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace Rl.Net.Cli
{
    [Verb("perf", HelpText = "Run perf test")]
    class PerfTestCommand : CommandBase
    {
        [Option(longName: "sleep", HelpText = "sleep interval in milliseconds", Required = false, Default = 0)]
        public int SleepIntervalMs { get; set; }

        [Option(longName: "duration", HelpText = "Duration of test in ms", Required = false, Default = 20000)]
        public int DurationMs { get; set; }

        [Option(longName: "data", HelpText = "Total size of messages in GB", Required = false, Default = 0.0)]
        public double DataSize { get; set; }

        [Option(longName: "actions", HelpText = "Amount of actions", Required = false, Default = 2)]
        public int ActionsCount { get; set; }

        [Option(longName: "shared-features", HelpText = "Amount of shared features", Required = false, Default = 2)]
        public int SharedFeatures { get; set; }

        [Option(longName: "action-features", HelpText = "Amount of action features", Required = false, Default = 2)]
        public int ActionFeatures { get; set; }

        [Option(longName: "slots", HelpText = "Number of slots", Required = false, Default = 0)]
        public int NumSlots { get; set; }

        [Option(longName: "tag", HelpText = "Tag of experiment", Required = false, Default = "test")]
        public string Tag { get; set; }

        [Option(longName: "parallelism", shortName: 'p', HelpText = "Degree of parallelism to use. Use 0 to use all available processors", Required = false, Default = 1)]
        public int Parallelism { get; set; }

        public override void Run()
        {
            Console.WriteLine("The number of processors on this computer is {0}.", Environment.ProcessorCount);
            this.Parallelism = this.Parallelism == 0 ? Environment.ProcessorCount : Math.Min(this.Parallelism, Environment.ProcessorCount);
            Console.WriteLine("Parallelism to be used in this perf test {0}.", this.Parallelism);
            IEnumerable<string> processortags = Enumerable.Range(0, Parallelism).Select(i => $"{this.Tag}-{i}");

            Statistics stats = new Statistics();
            object lockObj = new object();
            Parallel.ForEach(processortags, item => 
            {
                PerfTestStepProvider step = DoWork(item);
                lock (lockObj)
                {
                    stats += step.Stats;
                }
            });

            if (this.Parallelism > 1)
            {
                Console.WriteLine("Overall stats");
                stats.Print();
            }
        }

        private PerfTestStepProvider DoWork(string tag)
        {
            LiveModel liveModel = Helpers.CreateLiveModelOrExit(this.ConfigPath);

            PerfTestStepProvider stepProvider = new PerfTestStepProvider(this.ActionsCount, this.SharedFeatures, this.ActionFeatures, this.NumSlots)
            {
                Duration = TimeSpan.FromMilliseconds(this.DurationMs),
                Tag = tag,
                DataSize = this.DataSize * 1024 * 1024 * 1024 / this.Parallelism
            };

            Console.WriteLine(stepProvider.DataSize);
            using (RLDriver rlDriver = new RLDriver(liveModel, loopKind: this.GetLoopKind()))
            {
                rlDriver.StepInterval = TimeSpan.FromMilliseconds(this.SleepIntervalMs);
                rlDriver.Run(stepProvider);
            }

            stepProvider.Stats.Print();
            return stepProvider;
        }
    }
}
