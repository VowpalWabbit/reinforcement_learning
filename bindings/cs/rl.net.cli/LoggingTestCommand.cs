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

        public event EventHandler<RLException> OnError;

        public override void Run()
        {
            this.OnError = (sender, exception) => Helpers.WriteStatusAndExit(exception);

            LiveModel rankingModel = Helpers.CreateLiveModelOrExit(this.ConfigPath);
            RlLogger loggingModel = Helpers.CreateRlLoggerOrExit(this.LoggingConfigPath);

            SimulatorStepProvider stepProvider = new SimulatorStepProvider(this.Steps);
            RunContext runContext = new RunContext();
            foreach (IStepContext<float> step in stepProvider)
            {
                string eventId = step.EventId;
                try {
                    RankingResponse rankingResponse = rankingModel.ChooseRank(eventId, step.DecisionContext);

                    long actionIndex = rankingResponse.ChosenAction;

                    float outcome = step.GetOutcome(actionIndex, runContext.ResponseContainer);

                    rankingModel.TryQueueOutcomeEvent(eventId, outcome);

                    string modelId = rankingResponse.ModelId;
                    var actions = new List<int>();
                    var probabilities = new List<float>();
                    foreach (var action_score in rankingResponse)
                    {
                        actions.Add((int)action_score.ActionIndex);
                        probabilities.Add((int)action_score.Probability);
                    }

                    loggingModel.Log(step.DecisionContext, RankingResponse.Create(eventId, modelId, actionIndex, actions, probabilities));
                    loggingModel.Log(eventId, outcome);
                }
                catch (RLException e)
                {
                    this.SafeRaiseError(e);
                }
            }
        }

        private void SafeRaiseError(RLException exception)
        {
            EventHandler<RLException> localHandler = this.OnError;
            if (localHandler != null)
            {
                localHandler(this, exception);
            }
        }
    }
}
