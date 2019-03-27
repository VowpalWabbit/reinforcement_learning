using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using CommandLine;

namespace Rl.Net.Cli
{
    [Verb("perf", HelpText = "Run perf test")]
    class PerfTestCommand : CommandBase
    {
        [Option(longName: "sleep", HelpText = "sleep interval in milliseconds", Required = false, Default = 0)]
        public int SleepIntervalMs { get; set; }

        [Option(longName: "duration", HelpText = "Duration of test in ms", Required = false, Default = 20000)]
        public int DurationMs { get; set; }

        [Option(longName: "actions", HelpText = "Amount of actions", Required = false, Default = 2)]
        public int ActionsCount { get; set; }

        [Option(longName: "shared-features", HelpText = "Amount of shared features", Required = false, Default = 2)]
        public int SharedFeatures { get; set; }

        [Option(longName: "action-features", HelpText = "Amount of action features", Required = false, Default = 2)]
        public int ActionFeatures { get; set; }

        [Option(longName: "tag", HelpText = "Tag of experiment", Required = false, Default = "test")]
        public string Tag { get; set; }

        public override void Run()
        {
            LiveModel liveModel = CreateLiveModelOrExit(this.ConfigPath);
            RLDriver rlDriver = new RLDriver(liveModel);
            rlDriver.StepInterval = TimeSpan.FromMilliseconds(this.SleepIntervalMs);
            PerfTestStepProvider stepProvider = new PerfTestStepProvider(this.ActionsCount, this.SharedFeatures, this.ActionFeatures) { Duration = TimeSpan.FromMilliseconds(this.DurationMs) , Tag = this.Tag};
            rlDriver.Run(stepProvider);
            stepProvider.Stats.Print();
        }
    }
}
