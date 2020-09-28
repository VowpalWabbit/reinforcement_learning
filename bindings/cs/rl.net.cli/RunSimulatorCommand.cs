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

        [Option(longName:"ca", HelpText = "Use continuous actions", Required = false, Default = false)]
        public bool UseCA { get; set; }

        private LoopKind getLoopKind()
        {
            if (this.UseSlates)
            {
                return LoopKind.Slates;
            }
            else if (this.UseCA)
            {
                return LoopKind.CA;
            }
            else
            {
                return LoopKind.CB;
            }
        }

        public override void Run()
        {
            LiveModel liveModel = Helpers.CreateLiveModelOrExit(this.ConfigPath);

            RLSimulator rlSim = new RLSimulator(liveModel, loopKind: this.getLoopKind());
            rlSim.StepInterval = TimeSpan.FromMilliseconds(this.SleepIntervalMs);
            rlSim.OnError += (sender, apiStatus) => Helpers.WriteStatusAndExit(apiStatus);
            rlSim.Run(this.Steps);
        }
    }
}
