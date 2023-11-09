using System;
using CommandLine;

namespace Rl.Net.Cli
{
    [Verb("basicUsage", HelpText = "Basic usage of the API")]
    class BasicUsageCommand : CommandBase
    {
        [Option(longName: "testType", HelpText = "select from (liveModel, caLoop, cbLoop, ccbLoop, PdfExample) basic usage examples", Required = false, Default = "liveModel")]
        public string testType { get; set; }

        public override void Run()
        {
            switch (testType)
            {
                case "liveModel":
                    BasicUsage(this.ConfigPath);
                    break;
                case "caLoop":
                    BasicUsageCALoop(this.ConfigPath);
                    break;
                case "cbLoop":
                    BasicUsageCBLoop(this.ConfigPath);
                    break;
                case "ccbLoop":
                    BasicUsageCCBLoop(this.ConfigPath);
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

        public static void BasicUsageCALoop(string configPath)
        {
            const float outcome = 1.0f;
            const string eventId = "event_id";
            const string contextJson = "{ \"id\":\"j1\", \"20.3\":1, \"102.4\":1, \"-10.2\":1 }";

            CALoop ca_loop = Helpers.CreateLoopOrExit<CALoop>(configPath, config => new CALoop(config));

            ApiStatus apiStatus = new ApiStatus();

            ContinuousActionResponse continuousResponse = new ContinuousActionResponse();
            if (!ca_loop.TryRequestContinuousAction(eventId, contextJson, continuousResponse, apiStatus))
            {
                Helpers.WriteStatusAndExit(apiStatus);
            }

            Console.WriteLine($"Chosen action id: {continuousResponse.ChosenAction}");
            Console.WriteLine($"Chosen action pdf value: {continuousResponse.ChosenActionPdfValue}");

            if (!ca_loop.TryQueueOutcomeEvent(eventId, outcome, apiStatus))
            {
                Helpers.WriteStatusAndExit(apiStatus);
            }
            Console.WriteLine("Basice usage ca loop success");
        }

        public static void BasicUsageCBLoop(string configPath)
        {
            const float outcome = 1.0f;
            const string eventId = "event_id";
            const string contextJson = "{\"GUser\":{\"id\":\"a\",\"major\":\"eng\",\"hobby\":\"hiking\"},\"_multi\":[ { \"TAction\":{\"a1\":\"f1\"} },{\"TAction\":{\"a2\":\"f2\"}}]}";

            CBLoop cb_loop = Helpers.CreateLoopOrExit<CBLoop>(configPath, config => new CBLoop(config));

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

        public static void BasicUsageCCBLoop(string configPath)
        {
            const float outcome = 1.0f;
            const string eventId = "event_id";
            const string contextJson = @"
            {
            ""GUser"": {
                ""f1"": 82
            },
            ""_multi"": [
                {
                ""a1"": {
                    ""af1"": 82
                }
                },
                {
                ""a2"": {
                    ""af1"": 82
                }
                }
            ],
            ""_slots"": [
                {
                ""_id"": ""slot0""
                },
                {
                ""_id"": ""slot1""
                }
            ]
            }";

            CCBLoop ccb_loop = Helpers.CreateLoopOrExit<CCBLoop>(configPath, config => new CCBLoop(config));

            ApiStatus apiStatus = new ApiStatus();

            MultiSlotResponseDetailed multiResponse = new MultiSlotResponseDetailed();
            if (!ccb_loop.TryRequestMultiSlotDecisionDetailed(eventId, contextJson, multiResponse, apiStatus))
            {
                Helpers.WriteStatusAndExit(apiStatus);
            }

            // TODO: Populate actionProbs. Currently GetOutcome() just returns a fixed outcome value, so the values of actionProbs don't matter.
            ActionProbability[] actionProbs = new ActionProbability[multiResponse.Count];
            foreach (var slot in multiResponse)
            {
                Console.WriteLine($"Slot: {slot.SlotId}, Chosen Action: {slot.ChosenAction}");
                if (!ccb_loop.TryQueueOutcomeEvent(eventId, slot.SlotId, outcome, apiStatus))
                {
                    Helpers.WriteStatusAndExit(apiStatus);
                }
            }

            Console.WriteLine("Basice usage ccb loop success");
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
