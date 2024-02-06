using System;
using System.IO;
using System.Collections.Generic;

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

        public static LiveModel CreateLiveModelWithStaticModelOrExit(string clientJsonPath, string modelPath)
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

            config["model.source"] = "BINDING_DATA_TRANSPORT";
            List<byte> modelData = null;
            if (File.Exists(modelPath))
            {
                byte[] fileBytes = File.ReadAllBytes(modelPath);
                modelData = new List<byte>(fileBytes);
            }
            else
            {
                WriteErrorAndExit($"Could not find model file with path '{modelPath}'.");
            }

            // Use static model FactoryContext
            FactoryContext fc = new FactoryContext(modelData);
            LiveModel liveModel = new LiveModel(config, fc);

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

        public static LoopType CreateLoopOrExit<LoopType>(string clientJson, Func<Configuration, LoopType> createLoop, bool jsonStr = false) where LoopType : ILoop
        {
            string json = clientJson;
            if (!jsonStr)
            {
                if (!File.Exists(clientJson))
                {
                    WriteErrorAndExit($"Could not find file with path '{clientJson}'.");
                }

                json = File.ReadAllText(clientJson);
            }

            ApiStatus apiStatus = new ApiStatus();

            Configuration config;
            if (!Configuration.TryLoadConfigurationFromJson(json, out config, apiStatus))
            {
                WriteStatusAndExit(apiStatus);
            }
            string trace_log = config["trace.logger.implementation"];

            LoopType loop = createLoop(config);

            loop.BackgroundError += LiveModel_BackgroundError;
            if (trace_log == "CONSOLE_TRACE_LOGGER")
            {
                loop.TraceLoggerEvent += LiveModel_TraceLogEvent;
            }

            if (!loop.TryInit(apiStatus))
            {
                WriteStatusAndExit(apiStatus);
            }

            return loop;
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
