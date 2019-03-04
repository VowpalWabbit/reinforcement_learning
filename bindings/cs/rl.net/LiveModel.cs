using System;
using System.Runtime.InteropServices;

using Rl.Net.Native;

namespace Rl.Net
{
    public sealed class LiveModel : NativeObject<LiveModel>
    {
        [DllImport("rl.net.native.dll")]
        private static extern IntPtr CreateLiveModel(IntPtr config);

        [DllImport("rl.net.native.dll")]
        private static extern void DeleteLiveModel(IntPtr liveModel);

        private static New<LiveModel> BindConstructorArguments(Configuration config)
        {
            return new New<LiveModel>(() => CreateLiveModel(config.NativeHandle));
        }

        [DllImport("rl.net.native.dll")]
        private static extern int LiveModelInit(IntPtr liveModel, IntPtr apiStatus);

        [DllImport("rl.net.native.dll")]
        private static extern int LiveModelChooseRank(IntPtr liveModel, [MarshalAs(NativeMethods.StringMarshalling)] string eventId, [MarshalAs(NativeMethods.StringMarshalling)] string contextJson, IntPtr rankingResponse, IntPtr apiStatus);

        [DllImport("rl.net.native.dll")]
        private static extern int LiveModelChooseRankWithFlags(IntPtr liveModel, [MarshalAs(NativeMethods.StringMarshalling)] string eventId, [MarshalAs(NativeMethods.StringMarshalling)] string contextJson, uint flags, IntPtr rankingResponse, IntPtr apiStatus);

        [DllImport("rl.net.native.dll")]
        private static extern int LiveModelReportActionTaken(IntPtr liveModel, [MarshalAs(NativeMethods.StringMarshalling)] string eventId, IntPtr apiStatus);

        [DllImport("rl.net.native.dll")]
        private static extern int LiveModelReportOutcomeF(IntPtr liveModel, [MarshalAs(NativeMethods.StringMarshalling)] string eventId, float outcome, IntPtr apiStatus);

        [DllImport("rl.net.native.dll")]
        private static extern int LiveModelReportOutcomeJson(IntPtr liveModel, [MarshalAs(NativeMethods.StringMarshalling)] string eventId, [MarshalAs(NativeMethods.StringMarshalling)] string outcomeJson, IntPtr apiStatus);

        private delegate void managed_background_error_callback_t(IntPtr apiStatus);
        private readonly managed_background_error_callback_t managedErrorCallback;

        [DllImport("rl.net.native.dll")]
        private static extern void LiveModelSetCallback(IntPtr liveModel, [MarshalAs(UnmanagedType.FunctionPtr)] managed_background_error_callback_t callback = null);

        private delegate void managed_trace_callback_t(int logLevel, [MarshalAs(NativeMethods.StringMarshalling)] string msg);
        private readonly managed_trace_callback_t managedTraceCallback;

        [DllImport("rl.net.native.dll")]
        private static extern void LiveModelSetTrace(IntPtr liveModel, [MarshalAs(UnmanagedType.FunctionPtr)] managed_trace_callback_t callback = null);

        public LiveModel(Configuration config) : base(BindConstructorArguments(config), new Delete<LiveModel>(DeleteLiveModel))
        {
            this.managedErrorCallback = new managed_background_error_callback_t(this.WrapStatusAndRaiseBackgroundError);
            LiveModelSetCallback(this.NativeHandle, this.managedErrorCallback);

            this.managedTraceCallback = new managed_trace_callback_t(this.SendTrace);
        }

        private void WrapStatusAndRaiseBackgroundError(IntPtr apiStatusHandle)
        {
            using (ApiStatus status = new ApiStatus(apiStatusHandle)) 
            {
                EventHandler<ApiStatus> trargetEventLocal = this.BackgroundErrorInternal;
                if (trargetEventLocal != null)
                {
                    trargetEventLocal.Invoke(this, status);
                }
                else
                {
                    // This comes strictly from the background thread - so simply throwing here has
                    // the right semantics with respect to AppDomain.UnhandledException. Unfortunately,
                    // that seems to bring down the process, if there is nothing Managed under the native
                    // stack this will cause an application-level unhandled native exception, and will
                    // likely terminate the applciation. So new up a thread, and throw from it.
                    // See https://stackoverflow.com/questions/42298126/raising-exception-on-managed-and-unmanaged-callback-chain-with-p-invoke

                    // IMPORTANT: This is safe solely because the status string is marshaled into the
                    // exception message on construction (in other words, before control returns to the
                    // unmanaged call-stack - the Dispose() is a no-op because in this case NativeObject does
                    // not own the unmanaged pointer, but we use it to remove itself from the finalizer queue)
                    RLException e = new RLException(status);
                    new System.Threading.Thread(() => throw e).Start();
                }
            }
        }

        private void SendTrace(int logLevel, string msg)
        {
            this.OnTraceLoggerEventInternal?.Invoke(this, new TraceLogEventArgs((RLLogLevel)logLevel, msg));
        }

        public bool TryInit(ApiStatus apiStatus = null)
        {
            int result = LiveModelInit(this.NativeHandle, apiStatus.ToNativeHandleOrNullptr());
            return result == NativeMethods.SuccessStatus;
        }

        public void Init()
        {
            using (ApiStatus apiStatus = new ApiStatus())
            if (!this.TryInit(apiStatus))
            {
                throw new RLException(apiStatus);
            }
        }

        public bool TryChooseRank(string eventId, string contextJson, out RankingResponse response, ApiStatus apiStatus = null)
        {
            response = new RankingResponse();
            return this.TryChooseRank(eventId, contextJson, response, apiStatus);
        }

        public bool TryChooseRank(string eventId, string contextJson, RankingResponse response, ApiStatus apiStatus = null)
        {
            int result = LiveModelChooseRank(this.NativeHandle, eventId, contextJson, response.NativeHandle, apiStatus.ToNativeHandleOrNullptr());
            return result == NativeMethods.SuccessStatus;
        }

        public RankingResponse ChooseRank(string eventId, string contextJson)
        {
            RankingResponse result = new RankingResponse();
            
            using (ApiStatus apiStatus = new ApiStatus())
            if (!this.TryChooseRank(eventId, contextJson, result, apiStatus))
            {
                throw new RLException(apiStatus);
            }

            return result;
        }

        public bool TryChooseRank(string eventId, string contextJson, ActionFlags flags, out RankingResponse response, ApiStatus apiStatus = null)
        {
            response = new RankingResponse();
            return this.TryChooseRank(eventId, contextJson, flags, response, apiStatus);
        }

        public bool TryChooseRank(string eventId, string contextJson, ActionFlags flags, RankingResponse response, ApiStatus apiStatus = null)
        {
            int result = LiveModelChooseRankWithFlags(this.NativeHandle, eventId, contextJson, (uint)flags, response.NativeHandle, apiStatus.ToNativeHandleOrNullptr());
            return result == NativeMethods.SuccessStatus;
        }

        public RankingResponse ChooseRank(string eventId, string contextJson, ActionFlags flags)
        {
            RankingResponse result = new RankingResponse();

            using (ApiStatus apiStatus = new ApiStatus())
            if (!this.TryChooseRank(eventId, contextJson, flags, result, apiStatus))
            {
                throw new RLException(apiStatus);
            }

            return result;
        }

        [Obsolete("Use TryQueueActionTakenEvent instead.")]
        public bool TryReportActionTaken(string eventId, ApiStatus apiStatus = null) 
        => this.TryQueueActionTakenEvent(eventId, apiStatus);

        public bool TryQueueActionTakenEvent(string eventId, ApiStatus apiStatus = null)
        {
            int result = LiveModelReportActionTaken(this.NativeHandle, eventId, apiStatus.ToNativeHandleOrNullptr());
            return result == NativeMethods.SuccessStatus;
        }

        [Obsolete("Use QueueActionTakenEvent instead.")]
        public void ReportActionTaken(string eventId)
            => this.QueueActionTakenEvent(eventId);

        public void QueueActionTakenEvent(string eventId)
        {
            using (ApiStatus apiStatus = new ApiStatus())
            if (!this.TryQueueActionTakenEvent(eventId, apiStatus))
            {
                throw new RLException(apiStatus);
            }
        }

        [Obsolete("Use TryQueueOutcomeEvent instead.")]
        public bool TryReportOutcome(string eventId, float outcome, ApiStatus apiStatus = null)
            => this.TryQueueOutcomeEvent(eventId, outcome, apiStatus);

        public bool TryQueueOutcomeEvent(string eventId, float outcome, ApiStatus apiStatus = null)
        {
            int result = LiveModelReportOutcomeF(this.NativeHandle, eventId, outcome, apiStatus.ToNativeHandleOrNullptr());
            return result == NativeMethods.SuccessStatus;
        }

        [Obsolete("Use QueueOutcomeReport insteaed.")]
        public void ReportOutcome(string eventId, float outcome)
            => this.QueueOutcomeEvent(eventId, outcome);

        public void QueueOutcomeEvent(string eventId, float outcome)
        {
            using (ApiStatus apiStatus = new ApiStatus())
            if (!this.TryQueueOutcomeEvent(eventId, outcome, apiStatus))
            {
                throw new RLException(apiStatus);
            }
        }

        [Obsolete("Use TryQueueOutcomeEvent instead.")]
        public bool TryReportOutcome(string eventId, string outcomeJson, ApiStatus apiStatus = null)
            => this.TryQueueOutcomeEvent(eventId, outcomeJson, apiStatus);

        public bool TryQueueOutcomeEvent(string eventId, string outcomeJson, ApiStatus apiStatus = null)
        {
            int result = LiveModelReportOutcomeJson(this.NativeHandle, eventId, outcomeJson, apiStatus.ToNativeHandleOrNullptr());
            return result == NativeMethods.SuccessStatus;
        }

        [Obsolete("Use QueueOutcomeEvent instead.")]
        public void ReportOutcome(string eventId, string outcomeJson)
            => this.QueueOutcomeEvent(eventId, outcomeJson);

        public void QueueOutcomeEvent(string eventId, string outcomeJson)
        {
            using (ApiStatus apiStatus = new ApiStatus())
            if (!this.TryQueueOutcomeEvent(eventId, outcomeJson, apiStatus))
            {
                throw new RLException(apiStatus);
            }
        }

        private event EventHandler<ApiStatus> BackgroundErrorInternal;

        // This event is thread-safe, because we do not hook/unhook the event in user-schedulable code anymore.
        public event EventHandler<ApiStatus> BackgroundError
        {
            add
            {
                this.BackgroundErrorInternal += value;
            }
            remove
            {
                this.BackgroundErrorInternal -= value;
            }
        }

        private event EventHandler<TraceLogEventArgs> OnTraceLoggerEventInternal;
        
        // TODO:
        /// <remarks>
        /// Add/remove here is not thread safe.
        /// </remarks>
        public event EventHandler<TraceLogEventArgs> TraceLoggerEvent
        {
            add
            {
                if (this.OnTraceLoggerEventInternal == null)
                {
                    LiveModelSetTrace(this.NativeHandle, this.managedTraceCallback);
                }

                this.OnTraceLoggerEventInternal += value;
            }
            remove
            {
                this.OnTraceLoggerEventInternal -= value;

                if (this.OnTraceLoggerEventInternal == null)
                {
                    LiveModelSetTrace(this.NativeHandle, null);
                }
            }
        }
    }
}