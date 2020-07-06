using System;
using CommandLine;

namespace Rl.Net.Cli
{
    [Verb("simulator", HelpText = "Run simulator")]
    class RunSimulatorCommand : CommandBase
    {
        [Option(longName: "sleep", HelpText = "sleep interval in milliseconds", Required = false, Default = 1000)]
        public int SleepIntervalMs { get; set; }

        [Option(longName: "steps", HelpText = "Amount of steps", Required = false, Default = SimulatorStepProvider.InfinitySteps)]
        public int Steps { get; set; }

        [Option(longName:"slates", HelpText = "Use slates for decisions", Required = false, Default = false)]
        public bool UseSlates { get; set; }

        public override void Run()
        {
            LiveModel liveModel = Helpers.CreateLiveModelOrExit(this.ConfigPath);

            RLSimulator rlSim = new RLSimulator(liveModel, useSlates: this.UseSlates);
            rlSim.StepInterval = TimeSpan.FromMilliseconds(this.SleepIntervalMs);
            rlSim.OnError += (sender, apiStatus) => Helpers.WriteStatusAndExit(apiStatus);
            rlSim.Run(this.Steps);
        }
    }
}
