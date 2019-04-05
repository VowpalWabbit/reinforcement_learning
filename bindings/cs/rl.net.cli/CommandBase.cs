using System;
using System.IO;
using CommandLine;

namespace Rl.Net.Cli
{
    public abstract class CommandBase
    {
        [Option(longName: "config", HelpText = "the path to client config", Required = true)]
        public string ConfigPath { get; set; }

        public abstract void Run();
    }
}
