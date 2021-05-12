using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;

namespace Rl.Net.Cli
{
    public enum LoopKind : long
    {
        CB,
        CCB,
        CCBv2,
        Slates,
        CA
    }

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

        string CcbContext
        {
            get;
        }

        string SlatesContext
        {
            get;
        }

        string ContinuousActionContext
        {
            get;
        }

        TOutcome GetOutcome(long actionIndex, IEnumerable<ActionProbability> actionDistribution);
        TOutcome GetOutcome(int[] actionIndexes, float[] probabilities);
        TOutcome GetSlatesOutcome(int[] actionIndexes, float[] probabilities);
        TOutcome GetContinuousActionOutcome(float action, float pdfValue);
    }

    internal class RunContext
    {
        public RankingResponse ResponseContainer
        {
            get;
        } = new RankingResponse();

        public DecisionResponse DecisionResponseContainer
        {
            get;
        } = new DecisionResponse();

        public MultiSlotResponse MultiSlotResponseContainer
        {
            get;
        } = new MultiSlotResponse();

        public MultiSlotResponseDetailed MultiSlotResponseDetailedContainer
        {
            get;
        } = new MultiSlotResponseDetailed();

        public ContinuousActionResponse ContinuousActionContainer
        {
            get;
        } = new ContinuousActionResponse();

        public ApiStatus ApiStatusContainer
        {
            get;
        } = new ApiStatus();
    }

    internal interface IOutcomeReporter<TOutcome>
    {
        bool TryQueueOutcomeEvent(RunContext runContext, string eventId, TOutcome outcome);
        bool TryQueueOutcomeEvent(RunContext runContext, string eventId, string slotId, TOutcome outcome);
    }

    public class RLDriver : IOutcomeReporter<float>, IOutcomeReporter<string>, IDisposable
    {
        private LiveModel liveModel;
        private LoopKind loopKind;

        public RLDriver(LiveModel liveModel, LoopKind loopKind = LoopKind.CB)
        {
            this.liveModel = liveModel;
            this.loopKind = loopKind;
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

        bool IOutcomeReporter<float>.TryQueueOutcomeEvent(RunContext runContext, string eventId, string slotId, float outcome)
        {
            return this.liveModel.TryQueueOutcomeEvent(eventId, slotId, outcome, runContext.ApiStatusContainer);
        }

        bool IOutcomeReporter<string>.TryQueueOutcomeEvent(RunContext runContext, string eventId, string slotId, string outcome)
        {
            return this.liveModel.TryQueueOutcomeEvent(eventId, slotId, outcome, runContext.ApiStatusContainer);
        }

        private void Step<TOutcome>(RunContext runContext, IOutcomeReporter<TOutcome> outcomeReporter, IStepContext<TOutcome> step)
        {
            string eventId = step.EventId;
            TOutcome outcome = default(TOutcome);

            if (loopKind == LoopKind.Slates)
            {
                if (!liveModel.TryRequestMultiSlotDecision(eventId, step.SlatesContext, runContext.MultiSlotResponseContainer, runContext.ApiStatusContainer))
                {
                    this.SafeRaiseError(runContext.ApiStatusContainer);
                }

                int[] actions = runContext.MultiSlotResponseContainer.Select(slot => slot.ActionId).ToArray();
                float[] probs = runContext.MultiSlotResponseContainer.Select(slot => slot.Probability).ToArray();
                outcome = step.GetSlatesOutcome(actions, probs);
                if (outcome == null)
                {
                    return;
                }
            }
            else if (loopKind == LoopKind.CA)
            {
                if (!liveModel.TryRequestContinuousAction(eventId, step.ContinuousActionContext, runContext.ContinuousActionContainer, runContext.ApiStatusContainer))
                {
                    this.SafeRaiseError(runContext.ApiStatusContainer);
                }
                float action = runContext.ContinuousActionContainer.ChosenAction;
                float pdfValue = runContext.ContinuousActionContainer.ChosenActionPdfValue;
                outcome = step.GetContinuousActionOutcome(action, pdfValue);
                if (outcome == null)
                {
                    return;
                }
            }
            else if (loopKind == LoopKind.CCB)
            {
                if (!liveModel.TryRequestDecision(step.CcbContext, runContext.DecisionResponseContainer, runContext.ApiStatusContainer))
                {
                    this.SafeRaiseError(runContext.ApiStatusContainer);
                }
                // TODO: Populate actionProbs. Currently GetOutcome() just returns a fixed outcome value, so the values of actionProbs don't matter.
                ActionProbability[] actionProbs = new ActionProbability[runContext.DecisionResponseContainer.Count];
                foreach (var slot in runContext.DecisionResponseContainer)
                {
                    outcome = step.GetOutcome(slot.ActionId, actionProbs);
                    if (!outcomeReporter.TryQueueOutcomeEvent(runContext, slot.SlotId, outcome))
                    {
                        this.SafeRaiseError(runContext.ApiStatusContainer);
                    }
                }
                return;
            }
            else if (loopKind == LoopKind.CCBv2)
            {
                if (!liveModel.TryRequestMultiSlotDecisionDetailed(eventId, step.CcbContext, runContext.MultiSlotResponseDetailedContainer, runContext.ApiStatusContainer))
                {
                    this.SafeRaiseError(runContext.ApiStatusContainer);
                }
                foreach (var slot in runContext.MultiSlotResponseDetailedContainer)
                {
                    outcome = step.GetOutcome(slot.ChosenAction, slot);
                    if (!outcomeReporter.TryQueueOutcomeEvent(runContext, eventId, slot.SlotId, outcome))
                    {
                        this.SafeRaiseError(runContext.ApiStatusContainer);
                    }
                }
                return;
            }
            else
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

        #region IDisposable Support
        private bool disposedValue = false; // To detect redundant calls

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                if (disposing)
                {
                    this.liveModel?.Dispose();
                    this.liveModel = null;
                }

                disposedValue = true;
            }
        }


        public void Dispose()
        {
            // Do not change this code. Put cleanup code in Dispose(bool disposing) above.
            Dispose(true);
        }
        #endregion
    }
}
