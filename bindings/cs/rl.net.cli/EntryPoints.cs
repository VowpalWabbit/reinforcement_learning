using CommandLine;
using System;
using System.Collections.Generic;
using System.IO;

namespace Rl.Net.Cli
{
    static class EntryPoints
    {
        public static void Main(string[] args)
        {
            Parser.Default.ParseArguments
                <RunSimulatorCommand, ReplayCommand, PerfTestCommand, StatsReplayCommand, BasicUsageCommand>(args)
                .WithParsed<CommandBase>(command => command.Run());
        }

        public static IEnumerable<string> LazyReadLines(this TextReader textReader)
        {
            string line;
            while ((line = textReader.ReadLine()) != null)
            {
                if (string.Empty == line.Trim())
                {
                    continue;
                }

                yield return line;
            }
        }
    }
}