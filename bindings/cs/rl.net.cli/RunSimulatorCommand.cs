using System;
using CommandLine;

namespace Rl.Net.Cli
{
    [Verb("simulator", HelpText = "Run simulator")]
    class RunSimulatorCommand : CommandBase
    {
        [Option(longName: "sleep", HelpText = "sleep interval in milliseconds", Required = true)]
        public int SleepIntervalMs { get; set; }

        [Option(longName: "steps", HelpText = "Amount of steps", Required = true, Default = SimulatorStepProvider.InfinitySteps)]
        public int Steps { get; set; }

        public override void Run()
        {
            LiveModel liveModel = Helpers.CreateLiveModelOrExit(this.ConfigPath);

            RLSimulator rlSim = new RLSimulator(liveModel);
            rlSim.StepInterval = TimeSpan.FromMilliseconds(this.SleepIntervalMs);
            rlSim.OnError += (sender, apiStatus) => Helpers.WriteStatusAndExit(apiStatus);
            rlSim.Run(this.Steps);
        }
    }
}
