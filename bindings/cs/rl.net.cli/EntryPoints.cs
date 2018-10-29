using System;
using System.Collections.Generic;
using System.IO;
using Rl.Net;

namespace Rl.Net.Cli {
    static class EntryPoints
    {
        public static void Main(string [] args)
        {
            //BasicUsageExample(args);
            //RunSimulator(args);
            RunReplay(args);
        }

        private static void WriteErrorAndExit(string errorMessage, int exitCode = -1)
        {
            Console.Error.WriteLine(errorMessage);
            Environment.Exit(exitCode);
        }

        private static void WriteStatusAndExit(ApiStatus apiStatus)
        {
            WriteErrorAndExit(apiStatus.ErrorMessage);
        }

        private static LiveModel CreateLiveModelOrExit(string clientJsonPath)
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
            if (!liveModel.TryInit(apiStatus))
            {
                WriteStatusAndExit(apiStatus);
            }

            liveModel.BackgroundError += LiveModel_BackgroundError;

            return liveModel;
        }

        private static void LiveModel_BackgroundError(object sender, ApiStatus e)
        {
            Console.Error.WriteLine(e.ErrorMessage);
        }

        // TODO: Pull this out to a separate sample.
        public static void BasicUsageExample(string [] args)
        {
            const float outcome = 1.0f;
            const string eventId = "event_id";
            const string contextJson = "{\"GUser\":{\"id\":\"a\",\"major\":\"eng\",\"hobby\":\"hiking\"},\"_multi\":[ { \"TAction\":{\"a1\":\"f1\"} },{\"TAction\":{\"a2\":\"f2\"}}]}";

            if (args.Length != 1) 
            {
                WriteErrorAndExit("Missing path to client configuration json");
            }

            LiveModel liveModel = CreateLiveModelOrExit(args[0]);

            ApiStatus apiStatus = new ApiStatus();

            RankingResponse rankingResponse = new RankingResponse();
            if (!liveModel.TryChooseRank(eventId, contextJson, rankingResponse, apiStatus))
            {
                WriteStatusAndExit(apiStatus);
            }

            long actionId;
            if (!rankingResponse.TryGetChosenAction(out actionId, apiStatus))
            {
                WriteStatusAndExit(apiStatus);
            }

            Console.WriteLine($"Chosen action id: {actionId}");

            if (!liveModel.TryReportOutcome(eventId, outcome, apiStatus))
            {
                WriteStatusAndExit(apiStatus);
            }
        }

        public static void RunSimulator(string [] args)
        {
            if (args.Length != 1)
            {
                // TODO: Better usage
                WriteErrorAndExit("Missing path to client configuration json");
            }

            LiveModel liveModel = CreateLiveModelOrExit(args[0]);

            RLSimulator rlSim = new RLSimulator(liveModel);
            rlSim.OnError += (sender, apiStatus) => WriteStatusAndExit(apiStatus);
            rlSim.Run();
        }

        public static void RunReplay(string [] args)
        {
            if (args.Length != 2)
            {
                // TODO: Better usage
                WriteErrorAndExit("Missing path to client configuration json and dsjson log");
            }

            LiveModel liveModel = CreateLiveModelOrExit(args[0]);
            RLDriver rlDriver = new RLDriver(liveModel);

            using (TextReader textReader = File.OpenText(args[1]))
            {
                IEnumerable<string> dsJsonLines = textReader.LazyReadLines();
                ReplayStepProvider stepProvider = new ReplayStepProvider(dsJsonLines);

                rlDriver.Run(stepProvider);
            }
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