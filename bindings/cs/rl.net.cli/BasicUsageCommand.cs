using System;
using CommandLine;

namespace Rl.Net.Cli
{
    [Verb("basicUsage", HelpText = "Basic usage of the API")]
    class BasicUsageCommand : CommandBase
    {
        [Option(longName: "testType", HelpText = "select from (liveModel, CBLoop, PdfExample) basic usage examples", Required = false, Default = "liveModel")]
        public string testType { get; set; }

        public override void Run()
        {
            switch (testType)
            {
                case "liveModel":
                    BasicUsage(this.ConfigPath);
                    break;
                case "CBLoop":
                    BasicUsageCBLoop(this.ConfigPath);
                    break;
                case "PdfExample":
                    PdfExample(this.ConfigPath);
                    break;
                default:
                    Console.WriteLine("Invalid test type");
                    break;
            }
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
            Console.WriteLine("Basice usage live model success");
        }

        public static void BasicUsageCBLoop(string configPath)
        {
            const float outcome = 1.0f;
            const string eventId = "event_id";
            const string contextJson = "{\"GUser\":{\"id\":\"a\",\"major\":\"eng\",\"hobby\":\"hiking\"},\"_multi\":[ { \"TAction\":{\"a1\":\"f1\"} },{\"TAction\":{\"a2\":\"f2\"}}]}";

            CBLoop cb_loop = Helpers.CreateCBLoopOrExit(configPath);

            ApiStatus apiStatus = new ApiStatus();

            RankingResponse rankingResponse = new RankingResponse();
            if (!cb_loop.TryChooseRank(eventId, contextJson, rankingResponse, apiStatus))
            {
                Helpers.WriteStatusAndExit(apiStatus);
            }

            long actionId;
            if (!rankingResponse.TryGetChosenAction(out actionId, apiStatus))
            {
                Helpers.WriteStatusAndExit(apiStatus);
            }

            Console.WriteLine($"Chosen action id: {actionId}");

            if (!cb_loop.TryQueueOutcomeEvent(eventId, outcome, apiStatus))
            {
                Helpers.WriteStatusAndExit(apiStatus);
            }
            Console.WriteLine("Basice usage cb loop success");
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
            Console.WriteLine("Basice usage pdf example success");
        }
    }
}
