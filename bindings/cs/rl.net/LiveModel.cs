using System;
using System.Runtime.InteropServices;

using Rl.Net.Native;

namespace Rl.Net
{
    namespace Native
    {
        // The publics in this class are just a verbose, but jittably-efficient way of enabling overriding a native invocation
        internal static partial class NativeMethods
        {
            [DllImport("rl.net.native.dll")]
            public static extern IntPtr CreateLiveModel(IntPtr config);

            [DllImport("rl.net.native.dll")]
            public static extern void DeleteLiveModel(IntPtr liveModel);

            [DllImport("rl.net.native.dll")]
            public static extern int LiveModelInit(IntPtr liveModel, IntPtr apiStatus);

            [DllImport("rl.net.native.dll", EntryPoint = "LiveModelChooseRank")]
            private static extern int LiveModelChooseRankNative(IntPtr liveModel, IntPtr eventId, IntPtr contextJson, IntPtr rankingResponse, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, IntPtr, IntPtr, int> LiveModelChooseRankOverride { get; set; }

            public static int LiveModelChooseRank(IntPtr liveModel, IntPtr eventId, IntPtr contextJson, IntPtr rankingResponse, IntPtr apiStatus)
            {
                if (LiveModelChooseRankOverride != null)
                {
                    return LiveModelChooseRankOverride(liveModel, eventId, contextJson, rankingResponse, apiStatus);
                }

                return LiveModelChooseRankNative(liveModel, eventId, contextJson, rankingResponse, apiStatus);
            }

            [DllImport("rl.net.native.dll", EntryPoint = "LiveModelChooseRankWithFlags")]
            private static extern int LiveModelChooseRankWithFlagsNative(IntPtr liveModel, IntPtr eventId, IntPtr contextJson, uint flags, IntPtr rankingResponse, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, uint, IntPtr, IntPtr, int> LiveModelChooseRankWithFlagsOverride { get; set; }

            public static int LiveModelChooseRankWithFlags(IntPtr liveModel, IntPtr eventId, IntPtr contextJson, uint flags, IntPtr rankingResponse, IntPtr apiStatus)
            {
                if (LiveModelChooseRankWithFlagsOverride != null)
                {
                    return LiveModelChooseRankWithFlagsOverride(liveModel, eventId, contextJson, flags, rankingResponse, apiStatus);
                }

                return LiveModelChooseRankWithFlagsNative(liveModel, eventId, contextJson, flags, rankingResponse, apiStatus);
            }

            [DllImport("rl.net.native.dll", EntryPoint = "LiveModelRequestDecision")]
            private static extern int LiveModelRequestDecisionNative(IntPtr liveModel, IntPtr contextJson, IntPtr decisionResponse, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, IntPtr, int> LiveModelRequestDecisionOverride { get; set; }

            public static int LiveModelRequestDecision(IntPtr liveModel, IntPtr contextJson, IntPtr decisionResponse, IntPtr apiStatus)
            {
                if (LiveModelRequestDecisionOverride != null)
                {
                    return LiveModelRequestDecisionOverride(liveModel, contextJson, decisionResponse, apiStatus);
                }

                return LiveModelRequestDecisionNative(liveModel, contextJson, decisionResponse, apiStatus);
            }

            [DllImport("rl.net.native.dll", EntryPoint = "LiveModelRequestDecisionWithFlags")]
            private static extern int LiveModelRequestDecisionWithFlagsNative(IntPtr liveModel, IntPtr contextJson, uint flags, IntPtr decisionResponse, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, uint, IntPtr, IntPtr, int> LiveModelRequestDecisionWithFlagsOverride { get; set; }

            public static int LiveModelRequestDecisionWithFlags(IntPtr liveModel, IntPtr contextJson, uint flags, IntPtr decisionResponse, IntPtr apiStatus)
            {
                if (LiveModelRequestDecisionWithFlagsOverride != null)
                {
                    return LiveModelRequestDecisionWithFlagsOverride(liveModel, contextJson, flags, decisionResponse, apiStatus);
                }

                return LiveModelRequestDecisionWithFlagsNative(liveModel, contextJson, flags, decisionResponse, apiStatus);
            }

            [DllImport("rl.net.native.dll", EntryPoint = "LiveModelReportActionTaken")]
            private static extern int LiveModelReportActionTakenNative(IntPtr liveModel, IntPtr eventId, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, int> LiveModelReportActionTakenOverride { get; set; }

            public static int LiveModelReportActionTaken(IntPtr liveModel, IntPtr eventId, IntPtr apiStatus)
            {
                if (LiveModelReportActionTakenOverride != null)
                {
                    return LiveModelReportActionTakenOverride(liveModel, eventId, apiStatus);
                }
            
                return LiveModelReportActionTakenNative(liveModel, eventId, apiStatus);
            }

            [DllImport("rl.net.native.dll", EntryPoint = "LiveModelReportOutcomeF")]
            private static extern int LiveModelReportOutcomeFNative(IntPtr liveModel, IntPtr eventId, float outcome, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, float, IntPtr, int> LiveModelReportOutcomeFOverride { get; set; }

            public static int LiveModelReportOutcomeF(IntPtr liveModel, IntPtr eventId, float outcome, IntPtr apiStatus)
            {
                if (LiveModelReportOutcomeFOverride != null)
                {
                    return LiveModelReportOutcomeFOverride(liveModel, eventId, outcome, apiStatus);
                }

                return LiveModelReportOutcomeFNative(liveModel, eventId, outcome, apiStatus);
            }

            [DllImport("rl.net.native.dll", EntryPoint = "LiveModelReportOutcomeJson")]
            private static extern int LiveModelReportOutcomeJsonNative(IntPtr liveModel, IntPtr eventId, IntPtr outcomeJson, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, IntPtr, int> LiveModelReportOutcomeJsonOverride { get; set; }

            public static int LiveModelReportOutcomeJson(IntPtr liveModel, IntPtr eventId, IntPtr outcomeJson, IntPtr apiStatus)
            {
                if (LiveModelReportOutcomeJsonOverride != null)
                {
                    return LiveModelReportOutcomeJsonOverride(liveModel, eventId, outcomeJson, apiStatus);
                }

                return LiveModelReportOutcomeJsonNative(liveModel, eventId, outcomeJson, apiStatus);
            }

            [DllImport("rl.net.native.dll")]
            public static extern int LiveModelRefreshModel(IntPtr liveModel, IntPtr apiStatus);

            public delegate void managed_background_error_callback_t(IntPtr apiStatus);

            [DllImport("rl.net.native.dll")]
            public static extern void LiveModelSetCallback(IntPtr liveModel, [MarshalAs(UnmanagedType.FunctionPtr)] managed_background_error_callback_t callback = null);

            public delegate void managed_trace_callback_t(int logLevel, IntPtr msgUtf8Ptr);

            [DllImport("rl.net.native.dll")]
            public static extern void LiveModelSetTrace(IntPtr liveModel, [MarshalAs(UnmanagedType.FunctionPtr)] managed_trace_callback_t callback = null);
        }
    }

    public sealed class LiveModel : NativeObject<LiveModel>
    {
        private readonly NativeMethods.managed_background_error_callback_t managedErrorCallback;
        private readonly NativeMethods.managed_trace_callback_t managedTraceCallback;
        
        private static New<LiveModel> BindConstructorArguments(Configuration config)
        {
            return new New<LiveModel>(() => 
            {
                IntPtr result = NativeMethods.CreateLiveModel(config.DangerousGetHandle());

                GC.KeepAlive(config); // TODO: Is this one necessary, or does it live on the heap inside of the delegate?
                return result;
            });
        }
        
        public LiveModel(Configuration config) : base(BindConstructorArguments(config), new Delete<LiveModel>(NativeMethods.DeleteLiveModel))
        {
            this.managedErrorCallback = new NativeMethods.managed_background_error_callback_t(this.WrapStatusAndRaiseBackgroundError);

            // DangerousGetHandle here is trivially safe, because .Dispose() cannot be called before the object is
            // constructed.
            NativeMethods.LiveModelSetCallback(this.DangerousGetHandle(), this.managedErrorCallback);

            this.managedTraceCallback = new NativeMethods.managed_trace_callback_t(this.SendTrace);
        }

        private static void CheckJsonString(string json)
        {
            if (String.IsNullOrWhiteSpace(json))
            {
                throw new ArgumentException("Input json is empty", "json");
            }
        }

        unsafe private static int LiveModelChooseRank(IntPtr liveModel, string eventId, string contextJson, IntPtr rankingResponse, IntPtr apiStatus)
        {
            CheckJsonString(contextJson);

            fixed (byte* contextJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(contextJson))
            {
                IntPtr contextJsonUtf8Ptr = new IntPtr(contextJsonUtf8Bytes);

                // It is important to pass null on faithfully here, because we rely on this to switch between auto-generate
                // eventId and use supplied eventId at the rl.net.native layer.
                if (eventId == null)
                {
                    return NativeMethods.LiveModelChooseRank(liveModel, IntPtr.Zero, contextJsonUtf8Ptr, rankingResponse, apiStatus);
                }
                
                fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
                {
                    return NativeMethods.LiveModelChooseRank(liveModel, new IntPtr(eventIdUtf8Bytes), contextJsonUtf8Ptr, rankingResponse, apiStatus);
                }
            }
        }

        // TODO: Should we reduce the rl.net.native interface to only have one of these?
        unsafe private static int LiveModelChooseRankWithFlags(IntPtr liveModel, string eventId, string contextJson, uint flags, IntPtr rankingResponse, IntPtr apiStatus)
        {
            CheckJsonString(contextJson);

            fixed (byte* contextJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(contextJson))
            {
                IntPtr contextJsonUtf8Ptr = new IntPtr(contextJsonUtf8Bytes);

                // It is important to pass null on faithfully here, because we rely on this to switch between auto-generate
                // eventId and use supplied eventId at the rl.net.native layer.
                if (eventId == null)
                {
                    return NativeMethods.LiveModelChooseRankWithFlags(liveModel, IntPtr.Zero, contextJsonUtf8Ptr, flags, rankingResponse, apiStatus);
                }

                fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
                {
                    return NativeMethods.LiveModelChooseRankWithFlags(liveModel, new IntPtr(eventIdUtf8Bytes), contextJsonUtf8Ptr, flags, rankingResponse, apiStatus);
                }
            }
        }

        unsafe private static int LiveModelRequestDecision(IntPtr liveModel, string contextJson, IntPtr decisionResponse, IntPtr apiStatus)
        {
            CheckJsonString(contextJson);

            fixed (byte* contextJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(contextJson))
            {
                return NativeMethods.LiveModelRequestDecision(liveModel, new IntPtr(contextJsonUtf8Bytes), decisionResponse, apiStatus);
            }
        }

        unsafe private static int LiveModelRequestDecisionWithFlags(IntPtr liveModel, string contextJson, uint flags, IntPtr decisionResponse, IntPtr apiStatus)
        {
            CheckJsonString(contextJson);

            fixed (byte* contextJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(contextJson))
            {
                return NativeMethods.LiveModelRequestDecisionWithFlags(liveModel, new IntPtr(contextJsonUtf8Bytes), flags, decisionResponse, apiStatus);
            }
        }

        unsafe private static int LiveModelReportActionTaken(IntPtr liveModel, string eventId, IntPtr apiStatus)
        {
            if (eventId == null)
            {
                throw new ArgumentNullException("eventId");
            }

            fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
            {
                return NativeMethods.LiveModelReportActionTaken(liveModel, new IntPtr(eventIdUtf8Bytes), apiStatus);
            }
        }

        unsafe private static int LiveModelReportOutcomeF(IntPtr liveModel, string eventId, float outcome, IntPtr apiStatus)
        {
            if (eventId == null)
            {
                throw new ArgumentNullException("eventId");
            }

            fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
            {
                return NativeMethods.LiveModelReportOutcomeF(liveModel, new IntPtr(eventIdUtf8Bytes), outcome, apiStatus);
            }
        }

        unsafe private static int LiveModelReportOutcomeJson(IntPtr liveModel, string eventId, string outcomeJson, IntPtr apiStatus)
        {
            if (eventId == null)
            {
                throw new ArgumentNullException("eventId");
            }

            CheckJsonString(outcomeJson);

            fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
            fixed (byte* outcomeJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(outcomeJson))
            {
                return NativeMethods.LiveModelReportOutcomeJson(liveModel, new IntPtr(eventIdUtf8Bytes), new IntPtr(outcomeJsonUtf8Bytes), apiStatus);
            }
        }

        private void WrapStatusAndRaiseBackgroundError(IntPtr apiStatusHandle)
        {
            using (ApiStatus status = new ApiStatus(apiStatusHandle)) 
            {
                EventHandler<ApiStatus> targetEventLocal = this.BackgroundErrorInternal;
                if (targetEventLocal != null)
                {
                    targetEventLocal.Invoke(this, status);
                }
                else
                {
                    // This comes strictly from the background thread - so simply throwing here has
                    // the right semantics with respect to AppDomain.UnhandledException. Unfortunately,
                    // that seems to bring down the process, if there is nothing Managed under the native
                    // stack this will cause an application-level unhandled native exception, and will
                    // likely terminate the application. So new up a thread, and throw from it.
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

        private void SendTrace(int logLevel, IntPtr msgUtf8Ptr)
        {
            string msg = NativeMethods.StringMarshallingFunc(msgUtf8Ptr);

            this.OnTraceLoggerEventInternal?.Invoke(this, new TraceLogEventArgs((RLLogLevel)logLevel, msg));
        }

        public bool TryInit(ApiStatus apiStatus = null)
        {
            int result = NativeMethods.LiveModelInit(this.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

            GC.KeepAlive(this);
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
            int result = LiveModelChooseRank(this.DangerousGetHandle(), eventId, contextJson, response.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

            GC.KeepAlive(this);
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
            int result = LiveModelChooseRankWithFlags(this.DangerousGetHandle(), eventId, contextJson, (uint)flags, response.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

            GC.KeepAlive(this);
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

        public bool TryRequestDecision(string contextJson, out DecisionResponse response, ApiStatus apiStatus = null)
        {
            response = new DecisionResponse();
            return this.TryRequestDecision(contextJson, response, apiStatus);
        }

        public bool TryRequestDecision(string contextJson, DecisionResponse response, ApiStatus apiStatus = null)
        {
            int result = LiveModelRequestDecision(this.DangerousGetHandle(), contextJson, response.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

            GC.KeepAlive(this);
            return result == NativeMethods.SuccessStatus;
        }

        public DecisionResponse RequestDecision(string contextJson)
        {
            DecisionResponse result = new DecisionResponse();

            using (ApiStatus apiStatus = new ApiStatus())
            if (!this.TryRequestDecision(contextJson, result, apiStatus))
            {
                throw new RLException(apiStatus);
            }

            return result;
        }

        public bool TryRequestDecision(string contextJson, ActionFlags flags, out DecisionResponse response, ApiStatus apiStatus)
        {
            response = new DecisionResponse();

            GC.KeepAlive(this);
            return this.TryRequestDecision(contextJson, flags, response, apiStatus);
        }

        public bool TryRequestDecision(string contextJson, ActionFlags flags, DecisionResponse response, ApiStatus apiStatus)
        {
            int result = LiveModelRequestDecisionWithFlags(this.DangerousGetHandle(), contextJson, (uint)flags, response.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());
            return result == NativeMethods.SuccessStatus;
        }

        public DecisionResponse RequestDecision(string contextJson, ActionFlags flags)
        {
            DecisionResponse result = new DecisionResponse();

            using (ApiStatus apiStatus = new ApiStatus())
            if (!this.TryRequestDecision(contextJson, flags, result, apiStatus))
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
            int result = LiveModelReportActionTaken(this.DangerousGetHandle(), eventId, apiStatus.ToNativeHandleOrNullptrDangerous());

            GC.KeepAlive(this);
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
            int result = LiveModelReportOutcomeF(this.DangerousGetHandle(), eventId, outcome, apiStatus.ToNativeHandleOrNullptrDangerous());

            GC.KeepAlive(this);
            return result == NativeMethods.SuccessStatus;
        }

        [Obsolete("Use QueueOutcomeReport instead.")]
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
            int result = LiveModelReportOutcomeJson(this.DangerousGetHandle(), eventId, outcomeJson, apiStatus.ToNativeHandleOrNullptrDangerous());

            GC.KeepAlive(this);
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

        public void RefreshModel()
        {
            using (ApiStatus apiStatus = new ApiStatus())
            if (!this.TryRefreshModel(apiStatus))
            {
                throw new RLException(apiStatus);
            }
        }

        public bool TryRefreshModel(ApiStatus apiStatus = null)
        {
            int result = NativeMethods.LiveModelRefreshModel(this.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());
            return result == NativeMethods.SuccessStatus;
        }

        private event EventHandler<ApiStatus> BackgroundErrorInternal;

        // This event is thread-safe, because we do not hook/unhook the event in user-scheduleable code anymore.
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
                    NativeMethods.LiveModelSetTrace(this.DangerousGetHandle(), this.managedTraceCallback);
                    GC.KeepAlive(this);
                }

                this.OnTraceLoggerEventInternal += value;
            }
            remove
            {
                this.OnTraceLoggerEventInternal -= value;

                if (this.OnTraceLoggerEventInternal == null)
                {
                    NativeMethods.LiveModelSetTrace(this.DangerousGetHandle(), null);
                    GC.KeepAlive(this);
                }
            }
        }
    }
}