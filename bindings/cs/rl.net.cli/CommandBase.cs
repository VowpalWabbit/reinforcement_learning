using System;
using System.IO;
using CommandLine;

namespace Rl.Net.Cli
{
    public abstract class CommandBase
    {
        [Option(longName: "config", HelpText = "the path to client config", Required = true)]
        public string ConfigPath { get; set; }

        [Option(longName:"slates", HelpText = "Use slates for decisions", Required = false, Default = false)]
        public bool UseSlates { get; set; }

        [Option(longName:"ca", HelpText = "Use continuous actions", Required = false, Default = false)]
        public bool UseCA { get; set; }

        public LoopKind GetLoopKind()
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


        public abstract void Run();
    }
}
