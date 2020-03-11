using System;
using System.Collections.Generic;
using CommandLine;

namespace Rl.Net.Cli
{
    [Verb("logging-test", HelpText = "Run simulator")]
    class LoggingTestCommand : CommandBase
    {
        [Option(longName: "steps", HelpText = "Amount of steps", Required = false, Default = 100)]
        public int Steps { get; set; }

        [Option(longName: "logging-config", HelpText = "the path to logging client config", Required = true)]
        public string LoggingConfigPath { get; set; }

        public event EventHandler<ApiStatus> OnError;

        public override void Run()
        {
            this.OnError = (sender, apiStatus) => Helpers.WriteStatusAndExit(apiStatus);

            LiveModel rankingModel = Helpers.CreateLiveModelOrExit(this.ConfigPath);
            RlLoggerThreadUnsafe loggingModel = Helpers.CreateRlLoggerOrExit(this.LoggingConfigPath);

            SimulatorStepProvider stepProvider = new SimulatorStepProvider(this.Steps);
            RunContext runContext = new RunContext();
            foreach (IStepContext<float> step in stepProvider)
            {
                string eventId = step.EventId;

                if (!rankingModel.TryChooseRank(eventId, step.DecisionContext, runContext.ResponseContainer, runContext.ApiStatusContainer))
                {
                    this.SafeRaiseError(runContext.ApiStatusContainer);
                }

                long actionIndex = -1;
                if (!runContext.ResponseContainer.TryGetChosenAction(out actionIndex, runContext.ApiStatusContainer))
                {
                    this.SafeRaiseError(runContext.ApiStatusContainer);
                }

                float outcome = step.GetOutcome(actionIndex, runContext.ResponseContainer);

                if (!rankingModel.TryQueueOutcomeEvent(eventId, outcome, runContext.ApiStatusContainer))
                {
                    this.SafeRaiseError(runContext.ApiStatusContainer);
                }

                string modelId = runContext.ResponseContainer.ModelId;
                var actions = new List<int>();
                var probabilities = new List<float>();
                foreach (var action_score in runContext.ResponseContainer)
                {
                    actions.Add((int)action_score.ActionIndex);
                    probabilities.Add((int)action_score.Probability);
                }
                if (!loggingModel.TryLog(step.DecisionContext, RankingResponse.Create(eventId, modelId, actionIndex, actions, probabilities))
                    || !loggingModel.TryLog(eventId, outcome))
                {
                    this.SafeRaiseError(runContext.ApiStatusContainer);
                }
            }
        }

        private void SafeRaiseError(ApiStatus errorStatus)
        {
            EventHandler<ApiStatus> localHandler = this.OnError;
            if (localHandler != null)
            {
                localHandler(this, errorStatus);
            }
        }
    }
}
