using System;
using System.Diagnostics;
using Rl.Net;
using System;
using System.IO;

Console.WriteLine("Running RL C# test...");
var test = new RLTest();
test.RLTestSimple();
Console.WriteLine("Done");

public class RLTest
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

            LiveModel liveModel = new LiveModel(config);

            liveModel.BackgroundError += LiveModel_BackgroundError;
            liveModel.TraceLoggerEvent += LiveModel_TraceLogEvent;

            if (!liveModel.TryInit(apiStatus))
            {
                WriteStatusAndExit(apiStatus);
            }

            return liveModel;
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

    public void RLTestSimple()
    {
        const float outcome = 1.0f;
        const string eventId = "event_id";
        const string contextJson = "{\"GUser\":{\"id\":\"a\",\"major\":\"eng\",\"hobby\":\"hiking\"},\"_multi\":[ { \"TAction\":{\"a1\":\"f1\"} },{\"TAction\":{\"a2\":\"f2\"}}]}";

        LiveModel liveModel = Helpers.CreateLiveModelOrExit("client.json");

        ApiStatus apiStatus = new ApiStatus();

        RankingResponse rankingResponse = new RankingResponse();
        if (!liveModel.TryChooseRank(eventId, contextJson, rankingResponse, apiStatus))
        {
            Helpers.WriteStatusAndExit(apiStatus);
        }

        if (rankingResponse.Count != 2)
        {
            Helpers.WriteErrorAndExit("rankingResponse.Count is not 2");
        }

         if (rankingResponse.EventId != "event_id")
        {
            Helpers.WriteErrorAndExit("rankingResponse.EventId is not event_id");
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

        // Ensure senders are done sending the event
        System.Threading.Thread.Sleep(3000);

        var interactionFile = new FileInfo("interaction.fbs");
        var observationFile = new FileInfo("observation.fbs");
        if (!interactionFile.Exists)
        {
            Helpers.WriteErrorAndExit("interaction.fbs doesn't exist.");
        }

        if (!(interactionFile.Length > 0))
        {
            Helpers.WriteErrorAndExit("interaction.fbs is empty.");
        }

        if (!observationFile.Exists)
        {
            Helpers.WriteErrorAndExit("observation.fbs doesn't exist.");
        }

        if (!(observationFile.Length > 0))
        {
            Helpers.WriteErrorAndExit("observation.fbs is empty.");
        }
    }
}
