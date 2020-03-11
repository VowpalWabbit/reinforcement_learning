using System;
using System.Collections.Generic;
using System.IO;
using CommandLine;


namespace Rl.Net.Cli {
    static class EntryPoints
    {
        public static void Main(string [] args)
        {
            Parser.Default.ParseArguments
                <RunSimulatorCommand, ReplayCommand, PerfTestCommand, LoggingTestCommand>(args)
                .WithParsed<CommandBase>(command => command.Run());
            //BasicUsage(args[0]);
            //BasicUsageLogging(args[0]);
            //PdfExample(args[0]);
        }

        public static void BasicUsage(string configPath)
        {
            const float outcome = 1.0f;
            const string eventId = "event_id";
            const string contextJson = "{\"GUser\":{\"id\":\"a\",\"major\":\"eng\",\"hobby\":\"hiking\"},\"_multi\":[ { \"TAction\":{\"a1\":\"f1\"} },{\"TAction\":{\"a2\":\"f2\"}}]}";

            LiveModel liveModel = Helpers.CreateLiveModelOrExit(configPath);

            ApiStatus apiStatus = new ApiStatus();

            RankingResponse rankingResponse = new RankingResponse();
            if (!liveModel.TryChooseRank(eventId, contextJson, rankingResponse, apiStatus))
            {
                Helpers.WriteStatusAndExit(apiStatus);
            }

            long actionId;
            if (!rankingResponse.TryGetChosenAction(out actionId, apiStatus))
            {
                Helpers.WriteStatusAndExit(apiStatus);
            }

            Console.WriteLine($"Chosen action id: {actionId}");

            if (!liveModel.TryQueueOutcomeEvent(eventId, outcome, apiStatus))
            {
                Helpers.WriteStatusAndExit(apiStatus);
            }
        }

        public static void BasicUsageLogging(string configPath)
        {
            const float outcome = 1.0f;
            const string eventId = "event_id";
            const string contextJson = "{\"GUser\":{\"id\":\"a\",\"major\":\"eng\",\"hobby\":\"hiking\"},\"_multi\":[ { \"TAction\":{\"a1\":\"f1\"} },{\"TAction\":{\"a2\":\"f2\"}}]}";

            RlLogger rlLogger = Helpers.CreateRlLoggerOrExit(configPath);

            ApiStatus apiStatus = new ApiStatus();

            RankingResponse rankingResponse = RankingResponse.Create(eventId, "some_model", 1, new[] { 1, 0, 2, 3 }, new[] { 0.2f, 0.2f, 0.3f, 0.3f });

            try
            {
                rlLogger.Log(contextJson, rankingResponse);
                rlLogger.Log(eventId, outcome);
            }
            catch (RLException e)
            {
                Helpers.WriteStatusAndExit(e);
            }
        }

        public static void PdfExample(string configPath)
        {
            const float outcome = 1.0f;
            const string eventId = "event_id";
            const string contextJson = "{\"GUser\":{\"id\":\"a\",\"major\":\"eng\",\"hobby\":\"hiking\"},\"_multi\":[ { \"TAction\":{\"a1\":\"f1\"} },{\"TAction\":{\"a2\":\"f2\"}}],\"p\":[0.2, 0.8]}";

            LiveModel liveModel = Helpers.CreateLiveModelOrExit(configPath);

            ApiStatus apiStatus = new ApiStatus();

            RankingResponse rankingResponse = new RankingResponse();
            if (!liveModel.TryChooseRank(eventId, contextJson, rankingResponse, apiStatus))
            {
                Helpers.WriteStatusAndExit(apiStatus);
            }

            long actionId;
            if (!rankingResponse.TryGetChosenAction(out actionId, apiStatus))
            {
                Helpers.WriteStatusAndExit(apiStatus);
            }

            Console.WriteLine($"Chosen action id: {actionId}");

            if (!liveModel.TryQueueOutcomeEvent(eventId, outcome, apiStatus))
            {
                Helpers.WriteStatusAndExit(apiStatus);
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