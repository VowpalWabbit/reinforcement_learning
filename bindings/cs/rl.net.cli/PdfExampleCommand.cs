using System;
using CommandLine;

namespace Rl.Net.Cli
{
    [Verb("pdf-example", HelpText = "Pdf-passthrough test example")]
    class PdfExampleCommand : CommandBase
    {
        public override void Run()
        {
            const float outcome = 1.0f;
            const string eventId = "event_id";
            const string contextJson = "{\"GUser\":{\"id\":\"a\",\"major\":\"eng\",\"hobby\":\"hiking\"},\"_multi\":[ { \"TAction\":{\"a1\":\"f1\"} },{\"TAction\":{\"a2\":\"f2\"}}],\"p\":[0.2, 0.8]}";

            LiveModel liveModel = CreateLiveModelOrExit(this.ConfigPath);


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
    }
}
