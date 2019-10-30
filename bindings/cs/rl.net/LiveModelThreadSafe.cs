using System;
using System.Runtime.InteropServices;
using System.Threading;
using Rl.Net.Native;

namespace Rl.Net
{
    public sealed class LiveModelThreadSafe
    {
        private int disposedValue = 0;
        private LiveModel liveModel;
        
        public LiveModelThreadSafe(Configuration config)
        {
           this.liveModel = new LiveModel(config);
        }

        private void InvokeDangerous(Action action)
        {
            bool refAdded = false;
            this.liveModel.DangerousAddRef(ref refAdded);

            try
            {
                action();
            }
            finally
            {
                if (refAdded)
                {
                    this.liveModel.DangerousRelease();
                }
            }
        }

        private TResult InvokeDangerous<TResult>(Func<TResult> func)
        {
            bool refAdded = false;
            this.liveModel.DangerousAddRef(ref refAdded);
            
            try
            {
                return func();
            }
            finally
            {
                if (refAdded)
                {
                    this.liveModel.DangerousRelease();
                }
            }
        }

        public void Init()
        {
           InvokeDangerous(this.liveModel.Init);
        }

        public RankingResponse ChooseRank(string eventId, string contextJson)
        {
            return InvokeDangerous(() => this.liveModel.ChooseRank(eventId, contextJson));
        }

        public RankingResponse ChooseRank(string eventId, string contextJson, ActionFlags flags)
        {
            return InvokeDangerous(() => this.liveModel.ChooseRank(eventId, contextJson, flags));
        }

        public DecisionResponse RequestDecision(string contextJson)
        {
            return InvokeDangerous(() => this.liveModel.RequestDecision(contextJson));
        }

        public DecisionResponse RequestDecision(string contextJson, ActionFlags flags)
        {
            return InvokeDangerous(() => this.liveModel.RequestDecision(contextJson, flags));
        }

        public void QueueActionTakenEvent(string eventId)
        {
            InvokeDangerous(() => this.liveModel.QueueActionTakenEvent(eventId));
        }

        public void QueueOutcomeEvent(string eventId, float outcome)
        {
            InvokeDangerous(() => this.liveModel.QueueOutcomeEvent(eventId, outcome));
        }

        public void QueueOutcomeEvent(string eventId, string outcomeJson)
        {
            InvokeDangerous(() => this.liveModel.QueueOutcomeEvent(eventId, outcomeJson));
        }

        public void RefreshModel()
        {
            InvokeDangerous(this.liveModel.RefreshModel);
        }

        public event EventHandler<ApiStatus> BackgroundError
        {
            add
            {
                InvokeDangerous(() => this.liveModel.BackgroundError += value);
            }
            remove
            {
                InvokeDangerous(() => this.liveModel.BackgroundError -= value);
            }
        }
        
        public event EventHandler<TraceLogEventArgs> TraceLoggerEvent
        {
            add
            {
                InvokeDangerous(() => this.liveModel.TraceLoggerEvent += value);
            }
            remove
            {
                InvokeDangerous(() => this.liveModel.TraceLoggerEvent -= value);
            }
        }
    }
}