using System;
using System.IO;

namespace Rl.Net.Cli
{
    static internal class Helpers
    {
        public static void WriteErrorAndExit(string errorMessage, int exitCode = -1)
        {
            Console.Error.WriteLine(errorMessage);
            Environment.Exit(exitCode);
        }

        public static void WriteStatusAndExit(ApiStatus apiStatus)
        {
            WriteErrorAndExit(apiStatus.ErrorMessage);
        }

        public static LiveModel CreateLiveModelOrExit(string clientJsonPath)
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
            string trace_log = config["trace.logger.implementation"];

            LiveModel liveModel = new LiveModel(config);

            liveModel.BackgroundError += LiveModel_BackgroundError;
            if (trace_log == "CONSOLE_TRACE_LOGGER")
            {
                liveModel.TraceLoggerEvent += LiveModel_TraceLogEvent;
            }

            if (!liveModel.TryInit(apiStatus))
            {
                WriteStatusAndExit(apiStatus);
            }

            return liveModel;
        }

        public static CBLoop CreateCBLoopOrExit(string clientJsonPath)
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

            CBLoop cb_loop = new CBLoop(config);

            cb_loop.BackgroundError += LiveModel_BackgroundError;
            cb_loop.TraceLoggerEvent += LiveModel_TraceLogEvent;

            if (!cb_loop.TryInit(apiStatus))
            {
                WriteStatusAndExit(apiStatus);
            }

            return cb_loop;
        }

        public static void LiveModel_BackgroundError(object sender, ApiStatus e)
        {
            Console.Error.WriteLine(e.ErrorMessage);
        }

        public static void LiveModel_TraceLogEvent(object sender, TraceLogEventArgs e)
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
