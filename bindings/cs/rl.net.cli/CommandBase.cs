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


        protected static void WriteErrorAndExit(string errorMessage, int exitCode = -1)
        {
            Console.Error.WriteLine(errorMessage);
            Environment.Exit(exitCode);
        }

        protected static void WriteStatusAndExit(ApiStatus apiStatus)
        {
            WriteErrorAndExit(apiStatus.ErrorMessage);
        }

        protected static LiveModel CreateLiveModelOrExit(string clientJsonPath)
        {
            if (!File.Exists(clientJsonPath))
            {
                WriteErrorAndExit($"Could not find file with path '{clientJsonPath}'.");
            }

            string json = File.ReadAllText(clientJsonPath);

            ApiStatus apiStatus = new ApiStatus();

            Configuration config;
            if (!Configuration.TryLoadConfigurationFromJson(json, out config, apiStatus))
            {
                WriteStatusAndExit(apiStatus);
            }

            LiveModel liveModel = new LiveModel(config);

            liveModel.BackgroundError += LiveModel_BackgroundError;
            liveModel.TraceLoggerEvent += LiveModel_TraceLogEvent;

            if (!liveModel.TryInit(apiStatus))
            {
                WriteStatusAndExit(apiStatus);
            }

            return liveModel;
        }

        protected static void LiveModel_BackgroundError(object sender, ApiStatus e)
        {
            Console.Error.WriteLine(e.ErrorMessage);
        }

        protected static void LiveModel_TraceLogEvent(object sender, TraceLogEventArgs e)
        {
            RLLogLevel logLevel = e.LogLevel;
            switch (logLevel)
            {
                case RLLogLevel.LEVEL_ERROR:
                    Console.Error.WriteLine($"LogLevel: {e.LogLevel} LogMessage: {e.Message}");
                    break;
                default:
                    Console.WriteLine($"LogLevel: {e.LogLevel} LogMessage: {e.Message}");
                    break;
            }
        }
    }
}
