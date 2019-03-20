using CommandLine;

namespace Rl.Net.Cli
{
    [Verb("simulator", HelpText = "Run simulator")]
    class RunSimulatorCommand : CommandBase
    {
        public override void Run()
        {
            LiveModel liveModel = CreateLiveModelOrExit(this.ConfigPath);

            RLSimulator rlSim = new RLSimulator(liveModel);
            rlSim.OnError += (sender, apiStatus) => WriteStatusAndExit(apiStatus);
            rlSim.Run();
        }
    }
}
