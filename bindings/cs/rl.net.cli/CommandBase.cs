using CommandLine;

namespace Rl.Net.Cli
{
    public abstract class CommandBase
    {
        [Option(longName: "config", HelpText = "the path to client config", Required = true)]
        public string ConfigPath { get; set; }

        [Option(longName: "model", HelpText = "the path to model file", Required = false)]
        public string ModelPath { get; set; }

        [Option(longName: "slates", HelpText = "Use slates for decisions", Required = false, Default = false)]
        public bool UseSlates { get; set; }

        [Option(longName: "ca", HelpText = "Use continuous actions", Required = false, Default = false)]
        public bool UseCA { get; set; }

        [Option(longName: "ccb", Required = false, Default = false)]
        public bool UseCCB { get; set; }

        [Option(longName: "ccbv2", Required = false, Default = false)]
        public bool UseCCBv2 { get; set; }

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
            else if (this.UseCCB)
            {
                return LoopKind.CCB;
            }
            else if (this.UseCCBv2)
            {
                return LoopKind.CCBv2;
            }
            else
            {
                return LoopKind.CB;
            }
        }

        public abstract void Run();
    }
}
