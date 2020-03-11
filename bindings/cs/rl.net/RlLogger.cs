using System;
using System.Runtime.InteropServices;
using System.Threading;
using Rl.Net.Native;

namespace Rl.Net
{
    public sealed class RlLogger
    {
        private int disposedValue = 0;
        private RlLoggerThreadUnsafe rlLogger;

        public RlLogger(Configuration config)
        {
            this.rlLogger = new RlLoggerThreadUnsafe(config);
        }

        private void InvokeDangerous(Action action)
        {
            bool refAdded = false;

            try
            {
                this.rlLogger.DangerousAddRef(ref refAdded);
                action();
            }
            finally
            {
                if (refAdded)
                {
                    this.rlLogger.DangerousRelease();
                }
            }
        }

        private TResult InvokeDangerous<TResult>(Func<TResult> func)
        {
            bool refAdded = false;

            try
            {
                this.rlLogger.DangerousAddRef(ref refAdded);
                return func();
            }
            finally
            {
                if (refAdded)
                {
                    this.rlLogger.DangerousRelease();
                }
            }
        }

        public void Init()
        {
            InvokeDangerous(this.rlLogger.Init);
        }

        public void Log(string contextJson, RankingResponse rankingResponse)
        {
            InvokeDangerous(() => this.rlLogger.Log(contextJson, rankingResponse));
        }

        public void Log(string contextJson, DecisionResponse decisionResponse)
        {
            InvokeDangerous(() => this.rlLogger.Log(contextJson, decisionResponse));
        }

        public void Log(string eventId, float outcome)
        {
            InvokeDangerous(() => this.rlLogger.Log(eventId, outcome));
        }

        public void QueueOutcomeEvent(string eventId, string outcomeJson)
        {
            InvokeDangerous(() => this.rlLogger.Log(eventId, outcomeJson));
        }

        public event EventHandler<ApiStatus> BackgroundError
        {
            add
            {
                InvokeDangerous(() => this.rlLogger.BackgroundError += value);
            }
            remove
            {
                InvokeDangerous(() => this.rlLogger.BackgroundError -= value);
            }
        }

        public event EventHandler<TraceLogEventArgs> TraceLoggerEvent
        {
            add
            {
                InvokeDangerous(() => this.rlLogger.TraceLoggerEvent += value);
            }
            remove
            {
                InvokeDangerous(() => this.rlLogger.TraceLoggerEvent -= value);
            }
        }
    }
}