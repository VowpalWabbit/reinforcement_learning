using System.Collections.Generic;
using System.IO;
using CommandLine;

namespace Rl.Net.Cli
{
    [Verb("replay", HelpText = "Replay existing log")]
    class ReplayCommand : CommandBase
    {
        [Option(longName: "log", HelpText = "path to the log file to replay", Required = true)]
        public string LogPath { get; set; }

        public override void Run()
        {
            LiveModel liveModel = CreateLiveModelOrExit(this.ConfigPath);
            RLDriver rlDriver = new RLDriver(liveModel);

            using (TextReader textReader = File.OpenText(this.LogPath))
            {
                IEnumerable<string> dsJsonLines = textReader.LazyReadLines();
                ReplayStepProvider stepProvider = new ReplayStepProvider(dsJsonLines);

                rlDriver.Run(stepProvider);
            }
        }
    }
}
