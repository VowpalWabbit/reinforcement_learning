using Rl.Net;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace Rl.Net.Cli
{
    public interface IDriverStepProvider<out TOutcome> : IEnumerable<IStepContext<TOutcome>>
    {
    }

    public interface IStepContext<out TOutcome>
    {
        string EventId
        {
            get;
        }

        string DecisionContext
        {
            get;
        }
        string SlatesContext
        {
            get;
        }

        TOutcome GetOutcome(long actionIndex, IEnumerable<ActionProbability> actionDistribution);
        TOutcome GetOutcome(int[] actionIndexes, float[] probabilities);
    }

    internal class RunContext
    {
        public RankingResponse ResponseContainer
        {
            get;
        } = new RankingResponse();

        public ApiStatus ApiStatusContainer
        {
            get;
        } = new ApiStatus();

        public SlatesResponse SlatesContainer
        {
            get;
        } = new SlatesResponse();
    }

    internal interface IOutcomeReporter<TOutcome>
    {
        bool TryQueueOutcomeEvent(RunContext runContext, string eventId, TOutcome outcome);
    }

    public class RLDriver : IOutcomeReporter<float>, IOutcomeReporter<string>
    {
        private LiveModel liveModel;
        private bool useSlates;

        public RLDriver(LiveModel liveModel, bool useSlates)
        {
            this.liveModel = liveModel;
            this.useSlates = useSlates;
        }

        public TimeSpan StepInterval
        {
            get;
            set;
        } = TimeSpan.FromSeconds(2);

        public void Run<TOutcome>(IDriverStepProvider<TOutcome> stepProvider)
        {
            // TODO: Enable consumers to provider their own outcomeReporter
            IOutcomeReporter<TOutcome> outcomeReporter = this as IOutcomeReporter<TOutcome>;
            if (outcomeReporter == null)
            {
                throw new ArgumentException($"Invalid type argument {typeof(TOutcome).Name}", nameof(TOutcome));
            }

            int stepsCount = 0;
            RunContext runContext = new RunContext();
            foreach (IStepContext<TOutcome> step in stepProvider)
            {
                this.Step(runContext, outcomeReporter, step);

                // TODO: Change this to be a command-line arg
                Thread.Sleep(StepInterval);

                if (++stepsCount % 10000 == 0)
                {
                    Console.Out.WriteLine($"Processed {stepsCount} steps.");
                }
            }
        }

        public event EventHandler<ApiStatus> OnError;

        bool IOutcomeReporter<float>.TryQueueOutcomeEvent(RunContext runContext, string eventId, float outcome)
        {
            return this.liveModel.TryQueueOutcomeEvent(eventId, outcome, runContext.ApiStatusContainer);
        }

        bool IOutcomeReporter<string>.TryQueueOutcomeEvent(RunContext runContext, string eventId, string outcome)
        {
            return this.liveModel.TryQueueOutcomeEvent(eventId, outcome, runContext.ApiStatusContainer);
        }

        private void Step<TOutcome>(RunContext runContext, IOutcomeReporter<TOutcome> outcomeReporter, IStepContext<TOutcome> step)
        {
            string eventId = step.EventId;
            TOutcome outcome = default(TOutcome);

            if(useSlates) {
                if(!liveModel.TryRequestSlatesDecision(step.EventId, step.SlatesContext, runContext.SlatesContainer, runContext.ApiStatusContainer))
                {
                    this.SafeRaiseError(runContext.ApiStatusContainer);
                }

                int[] actions = runContext.SlatesContainer.Select(slot => slot.ActionId).ToArray();
                outcome = step.GetOutcome(actions, runContext.SlatesContainer.Select(slot => slot.Probability).ToArray());
                if (outcome == null)
                {
                    return;
                }
            } else
            {
                if (!liveModel.TryChooseRank(eventId, step.DecisionContext, runContext.ResponseContainer, runContext.ApiStatusContainer))
                {
                    this.SafeRaiseError(runContext.ApiStatusContainer);
                }

                long actionIndex = -1;
                if (!runContext.ResponseContainer.TryGetChosenAction(out actionIndex, runContext.ApiStatusContainer))
                {
                    this.SafeRaiseError(runContext.ApiStatusContainer);
                }

                outcome = step.GetOutcome(actionIndex, runContext.ResponseContainer.AsEnumerable());
                if (outcome == null)
                {
                    return;
                }
            }

            if (!outcomeReporter.TryQueueOutcomeEvent(runContext, eventId, outcome))
            {
                this.SafeRaiseError(runContext.ApiStatusContainer);
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
