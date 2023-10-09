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
            [DllImport("rlnetnative")]
            public static extern IntPtr CreateCBLoop(IntPtr config, IntPtr factoryContext);

            [DllImport("rlnetnative")]
            public static extern void DeleteCBLoop(IntPtr cbLoop);

            [DllImport("rlnetnative")]
            public static extern int CBLoopInit(IntPtr cbLoop, IntPtr apiStatus);

            [DllImport("rlnetnative", EntryPoint = "CBLoopChooseRank")]
            private static extern int CBLoopChooseRankNative(IntPtr cbLoop, IntPtr eventId, IntPtr contextJson, int contextJsonSize, IntPtr rankingResponse, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, int, IntPtr, IntPtr, int> CBLoopChooseRankOverride { get; set; }

            public static int CBLoopChooseRank(IntPtr cbLoop, IntPtr eventId, IntPtr contextJson, int contextJsonSize, IntPtr rankingResponse, IntPtr apiStatus)
            {
                if (CBLoopChooseRankOverride != null)
                {
                    return CBLoopChooseRankOverride(cbLoop, eventId, contextJson, contextJsonSize, rankingResponse, apiStatus);
                }

                return CBLoopChooseRankNative(cbLoop, eventId, contextJson, contextJsonSize, rankingResponse, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "CBLoopChooseRankWithFlags")]
            private static extern int CBLoopChooseRankWithFlagsNative(IntPtr cbLoop, IntPtr eventId, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr rankingResponse, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, int, uint, IntPtr, IntPtr, int> CBLoopChooseRankWithFlagsOverride { get; set; }

            public static int CBLoopChooseRankWithFlags(IntPtr cbLoop, IntPtr eventId, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr rankingResponse, IntPtr apiStatus)
            {
                if (CBLoopChooseRankWithFlagsOverride != null)
                {
                    return CBLoopChooseRankWithFlagsOverride(cbLoop, eventId, contextJson, contextJsonSize, flags, rankingResponse, apiStatus);
                }

                return CBLoopChooseRankWithFlagsNative(cbLoop, eventId, contextJson, contextJsonSize, flags, rankingResponse, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "CBLoopReportActionTaken")]
            private static extern int CBLoopReportActionTakenNative(IntPtr cbLoop, IntPtr eventId, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, int> CBLoopReportActionTakenOverride { get; set; }

            public static int CBLoopReportActionTaken(IntPtr cbLoop, IntPtr eventId, IntPtr apiStatus)
            {
                if (CBLoopReportActionTakenOverride != null)
                {
                    return CBLoopReportActionTakenOverride(cbLoop, eventId, apiStatus);
                }

                return CBLoopReportActionTakenNative(cbLoop, eventId, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "CBLoopReportActionMultiIdTaken")]
            private static extern int CBLoopReportActionTakenMultiIdNative(IntPtr cbLoop, IntPtr primaryId, IntPtr secondaryId, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, IntPtr, int> CBLoopReportActionTakenMultiIdOverride { get; set; }

            public static int CBLoopReportActionMultiIdTaken(IntPtr cbLoop, IntPtr primaryId, IntPtr secondaryId, IntPtr apiStatus)
            {
                if (CBLoopReportActionTakenMultiIdOverride != null)
                {
                    return CBLoopReportActionTakenMultiIdOverride(cbLoop, primaryId, secondaryId, apiStatus);
                }

                return CBLoopReportActionTakenMultiIdNative(cbLoop, primaryId, secondaryId, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "CBLoopReportOutcomeF")]
            private static extern int CBLoopReportOutcomeFNative(IntPtr cbLoop, IntPtr eventId, float outcome, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, float, IntPtr, int> CBLoopReportOutcomeFOverride { get; set; }

            public static int CBLoopReportOutcomeF(IntPtr cbLoop, IntPtr eventId, float outcome, IntPtr apiStatus)
            {
                if (CBLoopReportOutcomeFOverride != null)
                {
                    return CBLoopReportOutcomeFOverride(cbLoop, eventId, outcome, apiStatus);
                }

                return CBLoopReportOutcomeFNative(cbLoop, eventId, outcome, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "CBLoopReportOutcomeJson")]
            private static extern int CBLoopReportOutcomeJsonNative(IntPtr cbLoop, IntPtr eventId, IntPtr outcomeJson, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, IntPtr, int> CBLoopReportOutcomeJsonOverride { get; set; }

            public static int CBLoopReportOutcomeJson(IntPtr cbLoop, IntPtr eventId, IntPtr outcomeJson, IntPtr apiStatus)
            {
                if (CBLoopReportOutcomeJsonOverride != null)
                {
                    return CBLoopReportOutcomeJsonOverride(cbLoop, eventId, outcomeJson, apiStatus);
                }

                return CBLoopReportOutcomeJsonNative(cbLoop, eventId, outcomeJson, apiStatus);
            }

            [DllImport("rlnetnative")]
            public static extern int CBLoopRefreshModel(IntPtr cbLoop, IntPtr apiStatus);

            [DllImport("rlnetnative")]
            public static extern void CBLoopSetCallback(IntPtr cbLoop, [MarshalAs(UnmanagedType.FunctionPtr)] managed_background_error_callback_t callback = null);

            [DllImport("rlnetnative")]
            public static extern void CBLoopSetTrace(IntPtr cbLoop, [MarshalAs(UnmanagedType.FunctionPtr)] managed_trace_callback_t callback = null);
        }
    }

    public sealed class CBLoop : NativeObject<CBLoop>
    {
        private readonly NativeMethods.managed_background_error_callback_t managedErrorCallback;
        private readonly NativeMethods.managed_trace_callback_t managedTraceCallback;

        private static New<CBLoop> BindConstructorArguments(Configuration config, FactoryContext factoryContext)
        {
            return new New<CBLoop>(() =>
            {
                factoryContext = factoryContext ?? new FactoryContext();
                IntPtr result = NativeMethods.CreateCBLoop(config.DangerousGetHandle(), factoryContext.DangerousGetHandle());

                // These references do not live on the heap in this delegate, and could disappear during the invocation
                // of CreateCBLoop. Thus, we need to ensure GC knows not to release them until after that call
                // returns.
                GC.KeepAlive(config);
                GC.KeepAlive(factoryContext);

                return result;
            });

        }

        public CBLoop(Configuration config) : this(config, null)
        { }

        public CBLoop(Configuration config, FactoryContext factoryContext) : base(BindConstructorArguments(config, factoryContext), new Delete<CBLoop>(NativeMethods.DeleteCBLoop))
        {
            this.managedErrorCallback = new NativeMethods.managed_background_error_callback_t(this.WrapStatusAndRaiseBackgroundError);

            // DangerousGetHandle here is trivially safe, because .Dispose() cannot be called before the object is
            // constructed.
            NativeMethods.CBLoopSetCallback(this.DangerousGetHandle(), this.managedErrorCallback);

            this.managedTraceCallback = new NativeMethods.managed_trace_callback_t(this.SendTrace);
        }

        private static void CheckJsonString(string json)
        {
            if (String.IsNullOrWhiteSpace(json))
            {
                throw new ArgumentException("Input json is empty", "json");
            }
        }

        unsafe private static int CBLoopChooseRank(IntPtr cbLoop, string eventId, string contextJson, IntPtr rankingResponse, IntPtr apiStatus)
        {
            CheckJsonString(contextJson);

            fixed (byte* contextJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(contextJson))
            {
                int contextJsonSize = NativeMethods.StringEncoding.GetByteCount(contextJson);
                IntPtr contextJsonUtf8Ptr = new IntPtr(contextJsonUtf8Bytes);

                // It is important to pass null on faithfully here, because we rely on this to switch between auto-generate
                // eventId and use supplied eventId at the rl.net.native layer.
                if (eventId == null)
                {
                    return NativeMethods.CBLoopChooseRank(cbLoop, IntPtr.Zero, contextJsonUtf8Ptr, contextJsonSize, rankingResponse, apiStatus);
                }

                fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
                {
                    return NativeMethods.CBLoopChooseRank(cbLoop, new IntPtr(eventIdUtf8Bytes), contextJsonUtf8Ptr, contextJsonSize, rankingResponse, apiStatus);
                }
            }
        }

        // TODO: Should we reduce the rl.net.native interface to only have one of these?
        unsafe private static int CBLoopChooseRankWithFlags(IntPtr cbLoop, string eventId, string contextJson, uint flags, IntPtr rankingResponse, IntPtr apiStatus)
        {
            CheckJsonString(contextJson);

            fixed (byte* contextJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(contextJson))
            {
                int contextJsonSize = NativeMethods.StringEncoding.GetByteCount(contextJson);
                IntPtr contextJsonUtf8Ptr = new IntPtr(contextJsonUtf8Bytes);

                // It is important to pass null on faithfully here, because we rely on this to switch between auto-generate
                // eventId and use supplied eventId at the rl.net.native layer.
                if (eventId == null)
                {
                    return NativeMethods.CBLoopChooseRankWithFlags(cbLoop, IntPtr.Zero, contextJsonUtf8Ptr, contextJsonSize, flags, rankingResponse, apiStatus);
                }

                fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
                {
                    return NativeMethods.CBLoopChooseRankWithFlags(cbLoop, new IntPtr(eventIdUtf8Bytes), contextJsonUtf8Ptr, contextJsonSize, flags, rankingResponse, apiStatus);
                }
            }
        }

        unsafe private static int CBLoopReportActionTaken(IntPtr cbLoop, string eventId, IntPtr apiStatus)
        {
            if (eventId == null)
            {
                throw new ArgumentNullException("eventId");
            }

            fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
            {
                return NativeMethods.CBLoopReportActionTaken(cbLoop, new IntPtr(eventIdUtf8Bytes), apiStatus);
            }
        }

        unsafe private static int CBLoopReportActionMultiIdTaken(IntPtr cbLoop, string primaryId, string secondaryId, IntPtr apiStatus)
        {
            if (primaryId == null)
            {
                throw new ArgumentNullException("primaryId");
            }

            if (secondaryId == null)
            {
                throw new ArgumentNullException("secondaryId");
            }

            fixed (byte* episodeIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(primaryId))
            fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(secondaryId))
            {
                return NativeMethods.CBLoopReportActionMultiIdTaken(cbLoop, new IntPtr(episodeIdUtf8Bytes), new IntPtr(eventIdUtf8Bytes), apiStatus);
            }
        }

        unsafe private static int CBLoopReportOutcomeF(IntPtr cbLoop, string eventId, float outcome, IntPtr apiStatus)
        {
            if (eventId == null)
            {
                throw new ArgumentNullException("eventId");
            }

            fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
            {
                return NativeMethods.CBLoopReportOutcomeF(cbLoop, new IntPtr(eventIdUtf8Bytes), outcome, apiStatus);
            }
        }

        unsafe private static int CBLoopReportOutcomeJson(IntPtr cbLoop, string eventId, string outcomeJson, IntPtr apiStatus)
        {
            if (eventId == null)
            {
                throw new ArgumentNullException("eventId");
            }

            CheckJsonString(outcomeJson);

            fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
            fixed (byte* outcomeJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(outcomeJson))
            {
                return NativeMethods.CBLoopReportOutcomeJson(cbLoop, new IntPtr(eventIdUtf8Bytes), new IntPtr(outcomeJsonUtf8Bytes), apiStatus);
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
            int result = NativeMethods.CBLoopInit(this.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

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
            int result = CBLoopChooseRank(this.DangerousGetHandle(), eventId, contextJson, response.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

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
            int result = CBLoopChooseRankWithFlags(this.DangerousGetHandle(), eventId, contextJson, (uint)flags, response.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

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

        [Obsolete("Use TryQueueActionTakenEvent instead.")]
        public bool TryReportActionTaken(string eventId, ApiStatus apiStatus = null)
        => this.TryQueueActionTakenEvent(eventId, apiStatus);

        public bool TryQueueActionTakenEvent(string eventId, ApiStatus apiStatus = null)
        {
            int result = CBLoopReportActionTaken(this.DangerousGetHandle(), eventId, apiStatus.ToNativeHandleOrNullptrDangerous());

            GC.KeepAlive(apiStatus);
            GC.KeepAlive(this);
            return result == NativeMethods.SuccessStatus;
        }

        public bool TryQueueActionTakenEvent(string primaryId, string secondaryId, ApiStatus apiStatus = null)
        {
            int result = CBLoopReportActionMultiIdTaken(this.DangerousGetHandle(), primaryId, secondaryId, apiStatus.ToNativeHandleOrNullptrDangerous());

            GC.KeepAlive(apiStatus);
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

        public void QueueActionTakenEvent(string primaryId, string secondaryId)
        {
            using (ApiStatus apiStatus = new ApiStatus())
                if (!this.TryQueueActionTakenEvent(primaryId, secondaryId, apiStatus))
                {
                    throw new RLException(apiStatus);
                }
        }

        [Obsolete("Use TryQueueOutcomeEvent instead.")]
        public bool TryReportOutcome(string eventId, float outcome, ApiStatus apiStatus = null)
            => this.TryQueueOutcomeEvent(eventId, outcome, apiStatus);

        public bool TryQueueOutcomeEvent(string eventId, float outcome, ApiStatus apiStatus = null)
        {
            int result = CBLoopReportOutcomeF(this.DangerousGetHandle(), eventId, outcome, apiStatus.ToNativeHandleOrNullptrDangerous());

            GC.KeepAlive(apiStatus);
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
            int result = CBLoopReportOutcomeJson(this.DangerousGetHandle(), eventId, outcomeJson, apiStatus.ToNativeHandleOrNullptrDangerous());

            GC.KeepAlive(apiStatus);
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
            int result = NativeMethods.CBLoopRefreshModel(this.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

            GC.KeepAlive(apiStatus);
            GC.KeepAlive(this);
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
                    NativeMethods.CBLoopSetTrace(this.DangerousGetHandle(), this.managedTraceCallback);
                    GC.KeepAlive(this);
                }

                this.OnTraceLoggerEventInternal += value;
            }
            remove
            {
                this.OnTraceLoggerEventInternal -= value;

                if (this.OnTraceLoggerEventInternal == null)
                {
                    NativeMethods.CBLoopSetTrace(this.DangerousGetHandle(), null);
                    GC.KeepAlive(this);
                }
            }
        }
    }
}