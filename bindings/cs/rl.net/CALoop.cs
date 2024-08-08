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
            [DllImport(NativeImports.RLNETNATIVE)]
            public static extern IntPtr CreateCALoop(IntPtr config, IntPtr factoryContext);

            [DllImport(NativeImports.RLNETNATIVE)]
            public static extern void DeleteCALoop(IntPtr caLoop);

            [DllImport(NativeImports.RLNETNATIVE)]
            public static extern int CALoopInit(IntPtr caLoop, IntPtr apiStatus);

            [DllImport(NativeImports.RLNETNATIVE, EntryPoint = "CALoopRequestContinuousAction")]
            private static extern int CALoopRequestContinuousActionNative(IntPtr caLoop, IntPtr eventId, IntPtr contextJson, int contextJsonSize, IntPtr continuousActionResponse, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, int, IntPtr, IntPtr, int> CALoopRequestContinuousActionOverride { get; set; }

            public static int CALoopRequestContinuousAction(IntPtr caLoop, IntPtr eventId, IntPtr contextJson, int contextJsonSize, IntPtr continuousActionResponse, IntPtr apiStatus)
            {
                if (CALoopRequestContinuousActionOverride != null)
                {
                    return CALoopRequestContinuousActionOverride(caLoop, eventId, contextJson, contextJsonSize, continuousActionResponse, apiStatus);
                }

                return CALoopRequestContinuousActionNative(caLoop, eventId, contextJson, contextJsonSize, continuousActionResponse, apiStatus);
            }

            [DllImport(NativeImports.RLNETNATIVE, EntryPoint = "CALoopRequestContinuousActionWithFlags")]
            private static extern int CALoopRequestContinuousActionWithFlagsNative(IntPtr caLoop, IntPtr eventId, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr continuousActionResponse, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, int, uint, IntPtr, IntPtr, int> CALoopRequestContinuousActionWithFlagsOverride { get; set; }

            public static int CALoopRequestContinuousActionWithFlags(IntPtr caLoop, IntPtr eventId, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr continuousActionResponse, IntPtr apiStatus)
            {
                if (CALoopRequestContinuousActionWithFlagsOverride != null)
                {
                    return CALoopRequestContinuousActionWithFlagsOverride(caLoop, eventId, contextJson, contextJsonSize, flags, continuousActionResponse, apiStatus);
                }

                return CALoopRequestContinuousActionWithFlagsNative(caLoop, eventId, contextJson, contextJsonSize, flags, continuousActionResponse, apiStatus);
            }

            [DllImport(NativeImports.RLNETNATIVE, EntryPoint = "CALoopReportActionTaken")]
            private static extern int CALoopReportActionTakenNative(IntPtr caLoop, IntPtr eventId, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, int> CALoopReportActionTakenOverride { get; set; }

            public static int CALoopReportActionTaken(IntPtr caLoop, IntPtr eventId, IntPtr apiStatus)
            {
                if (CALoopReportActionTakenOverride != null)
                {
                    return CALoopReportActionTakenOverride(caLoop, eventId, apiStatus);
                }

                return CALoopReportActionTakenNative(caLoop, eventId, apiStatus);
            }

            [DllImport(NativeImports.RLNETNATIVE, EntryPoint = "CALoopReportActionMultiIdTaken")]
            private static extern int CALoopReportActionTakenMultiIdNative(IntPtr caLoop, IntPtr primaryId, IntPtr secondaryId, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, IntPtr, int> CALoopReportActionTakenMultiIdOverride { get; set; }

            public static int CALoopReportActionMultiIdTaken(IntPtr caLoop, IntPtr primaryId, IntPtr secondaryId, IntPtr apiStatus)
            {
                if (CALoopReportActionTakenMultiIdOverride != null)
                {
                    return CALoopReportActionTakenMultiIdOverride(caLoop, primaryId, secondaryId, apiStatus);
                }

                return CALoopReportActionTakenMultiIdNative(caLoop, primaryId, secondaryId, apiStatus);
            }

            [DllImport(NativeImports.RLNETNATIVE, EntryPoint = "CALoopReportOutcomeF")]
            private static extern int CALoopReportOutcomeFNative(IntPtr caLoop, IntPtr eventId, float outcome, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, float, IntPtr, int> CALoopReportOutcomeFOverride { get; set; }

            public static int CALoopReportOutcomeF(IntPtr caLoop, IntPtr eventId, float outcome, IntPtr apiStatus)
            {
                if (CALoopReportOutcomeFOverride != null)
                {
                    return CALoopReportOutcomeFOverride(caLoop, eventId, outcome, apiStatus);
                }

                return CALoopReportOutcomeFNative(caLoop, eventId, outcome, apiStatus);
            }

            [DllImport(NativeImports.RLNETNATIVE, EntryPoint = "CALoopReportOutcomeJson")]
            private static extern int CALoopReportOutcomeJsonNative(IntPtr caLoop, IntPtr eventId, IntPtr outcomeJson, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, IntPtr, int> CALoopReportOutcomeJsonOverride { get; set; }

            public static int CALoopReportOutcomeJson(IntPtr caLoop, IntPtr eventId, IntPtr outcomeJson, IntPtr apiStatus)
            {
                if (CALoopReportOutcomeJsonOverride != null)
                {
                    return CALoopReportOutcomeJsonOverride(caLoop, eventId, outcomeJson, apiStatus);
                }

                return CALoopReportOutcomeJsonNative(caLoop, eventId, outcomeJson, apiStatus);
            }

            [DllImport(NativeImports.RLNETNATIVE)]
            public static extern int CALoopRefreshModel(IntPtr caLoop, IntPtr apiStatus);

            [DllImport(NativeImports.RLNETNATIVE)]
            public static extern void CALoopSetCallback(IntPtr caLoop, [MarshalAs(UnmanagedType.FunctionPtr)] managed_background_error_callback_t callback = null);

            [DllImport(NativeImports.RLNETNATIVE)]
            public static extern void CALoopSetTrace(IntPtr caLoop, [MarshalAs(UnmanagedType.FunctionPtr)] managed_trace_callback_t callback = null);
        }
    }

    public sealed class CALoop : NativeObject<CALoop>, ILoop
    {
        private readonly NativeMethods.managed_background_error_callback_t managedErrorCallback;
        private readonly NativeMethods.managed_trace_callback_t managedTraceCallback;

        private static New<CALoop> BindConstructorArguments(Configuration config, FactoryContext factoryContext)
        {
            return new New<CALoop>(() =>
            {
                factoryContext = factoryContext ?? new FactoryContext();
                IntPtr result = NativeMethods.CreateCALoop(config.DangerousGetHandle(), factoryContext.DangerousGetHandle());

                // These references do not live on the heap in this delegate, and could disappear during the invocation
                // of CreateCALoop. Thus, we need to ensure GC knows not to release them until after that call
                // returns.
                GC.KeepAlive(config);
                GC.KeepAlive(factoryContext);

                return result;
            });

        }

        public CALoop(Configuration config) : this(config, null)
        { }

        public CALoop(Configuration config, FactoryContext factoryContext) : base(BindConstructorArguments(config, factoryContext), new Delete<CALoop>(NativeMethods.DeleteCALoop))
        {
            this.managedErrorCallback = new NativeMethods.managed_background_error_callback_t(this.WrapStatusAndRaiseBackgroundError);

            // DangerousGetHandle here is trivially safe, because .Dispose() cannot be called before the object is
            // constructed.
            NativeMethods.CALoopSetCallback(this.DangerousGetHandle(), this.managedErrorCallback);

            this.managedTraceCallback = new NativeMethods.managed_trace_callback_t(this.SendTrace);
        }

        private static void CheckJsonString(string json)
        {
            if (String.IsNullOrWhiteSpace(json))
            {
                throw new ArgumentException("Input json is empty", "json");
            }
        }

        unsafe private static int CALoopRequestContinuousAction(IntPtr caLoop, string eventId, string contextJson, IntPtr continuousActionResponse, IntPtr apiStatus)
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
                    return NativeMethods.CALoopRequestContinuousAction(caLoop, IntPtr.Zero, contextJsonUtf8Ptr, contextJsonSize, continuousActionResponse, apiStatus);
                }

                fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
                {
                    return NativeMethods.CALoopRequestContinuousAction(caLoop, new IntPtr(eventIdUtf8Bytes), contextJsonUtf8Ptr, contextJsonSize, continuousActionResponse, apiStatus);
                }
            }
        }

        unsafe private static int CALoopRequestContinuousActionWithFlags(IntPtr caLoop, string eventId, string contextJson, uint flags, IntPtr continuousActionResponse, IntPtr apiStatus)
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
                    return NativeMethods.CALoopRequestContinuousActionWithFlags(caLoop, IntPtr.Zero, contextJsonUtf8Ptr, contextJsonSize, flags, continuousActionResponse, apiStatus);
                }

                fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
                {
                    return NativeMethods.CALoopRequestContinuousActionWithFlags(caLoop, new IntPtr(eventIdUtf8Bytes), contextJsonUtf8Ptr, contextJsonSize, flags, continuousActionResponse, apiStatus);
                }
            }
        }

        unsafe private static int CALoopReportActionTaken(IntPtr caLoop, string eventId, IntPtr apiStatus)
        {
            if (eventId == null)
            {
                throw new ArgumentNullException("eventId");
            }

            fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
            {
                return NativeMethods.CALoopReportActionTaken(caLoop, new IntPtr(eventIdUtf8Bytes), apiStatus);
            }
        }

        unsafe private static int CALoopReportActionMultiIdTaken(IntPtr caLoop, string primaryId, string secondaryId, IntPtr apiStatus)
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
                return NativeMethods.CALoopReportActionMultiIdTaken(caLoop, new IntPtr(episodeIdUtf8Bytes), new IntPtr(eventIdUtf8Bytes), apiStatus);
            }
        }

        unsafe private static int CALoopReportOutcomeF(IntPtr caLoop, string eventId, float outcome, IntPtr apiStatus)
        {
            if (eventId == null)
            {
                throw new ArgumentNullException("eventId");
            }

            fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
            {
                return NativeMethods.CALoopReportOutcomeF(caLoop, new IntPtr(eventIdUtf8Bytes), outcome, apiStatus);
            }
        }

        unsafe private static int CALoopReportOutcomeJson(IntPtr caLoop, string eventId, string outcomeJson, IntPtr apiStatus)
        {
            if (eventId == null)
            {
                throw new ArgumentNullException("eventId");
            }

            CheckJsonString(outcomeJson);

            fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
            fixed (byte* outcomeJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(outcomeJson))
            {
                return NativeMethods.CALoopReportOutcomeJson(caLoop, new IntPtr(eventIdUtf8Bytes), new IntPtr(outcomeJsonUtf8Bytes), apiStatus);
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
            int result = NativeMethods.CALoopInit(this.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

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

        public bool TryRequestContinuousAction(string eventId, string contextJson, out ContinuousActionResponse response, ApiStatus apiStatus = null)
        {
            response = new ContinuousActionResponse();
            return this.TryRequestContinuousAction(eventId, contextJson, response, apiStatus);
        }

        public bool TryRequestContinuousAction(string eventId, string contextJson, ContinuousActionResponse response, ApiStatus apiStatus = null)
        {
            int result = CALoopRequestContinuousAction(this.DangerousGetHandle(), eventId, contextJson, response.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

            GC.KeepAlive(this);
            return result == NativeMethods.SuccessStatus;
        }

        public ContinuousActionResponse RequestContinuousAction(string eventId, string contextJson)
        {
            ContinuousActionResponse result = new ContinuousActionResponse();

            using (ApiStatus apiStatus = new ApiStatus())
                if (!this.TryRequestContinuousAction(eventId, contextJson, result, apiStatus))
                {
                    throw new RLException(apiStatus);
                }

            return result;
        }

        public bool TryRequestContinuousAction(string eventId, string contextJson, ActionFlags flags, out ContinuousActionResponse response, ApiStatus apiStatus = null)
        {
            response = new ContinuousActionResponse();
            return this.TryRequestContinuousAction(eventId, contextJson, flags, response, apiStatus);
        }

        public bool TryRequestContinuousAction(string eventId, string contextJson, ActionFlags flags, ContinuousActionResponse response, ApiStatus apiStatus = null)
        {
            int result = CALoopRequestContinuousActionWithFlags(this.DangerousGetHandle(), eventId, contextJson, (uint)flags, response.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

            GC.KeepAlive(this);
            return result == NativeMethods.SuccessStatus;
        }

        public ContinuousActionResponse RequestContinuousAction(string eventId, string contextJson, ActionFlags flags)
        {
            ContinuousActionResponse result = new ContinuousActionResponse();

            using (ApiStatus apiStatus = new ApiStatus())
                if (!this.TryRequestContinuousAction(eventId, contextJson, flags, result, apiStatus))
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
            int result = CALoopReportActionTaken(this.DangerousGetHandle(), eventId, apiStatus.ToNativeHandleOrNullptrDangerous());

            GC.KeepAlive(apiStatus);
            GC.KeepAlive(this);
            return result == NativeMethods.SuccessStatus;
        }

        public bool TryQueueActionTakenEvent(string primaryId, string secondaryId, ApiStatus apiStatus = null)
        {
            int result = CALoopReportActionMultiIdTaken(this.DangerousGetHandle(), primaryId, secondaryId, apiStatus.ToNativeHandleOrNullptrDangerous());

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
            int result = CALoopReportOutcomeF(this.DangerousGetHandle(), eventId, outcome, apiStatus.ToNativeHandleOrNullptrDangerous());

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
            int result = CALoopReportOutcomeJson(this.DangerousGetHandle(), eventId, outcomeJson, apiStatus.ToNativeHandleOrNullptrDangerous());

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
            int result = NativeMethods.CALoopRefreshModel(this.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

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
                    NativeMethods.CALoopSetTrace(this.DangerousGetHandle(), this.managedTraceCallback);
                    GC.KeepAlive(this);
                }

                this.OnTraceLoggerEventInternal += value;
            }
            remove
            {
                this.OnTraceLoggerEventInternal -= value;

                if (this.OnTraceLoggerEventInternal == null)
                {
                    NativeMethods.CALoopSetTrace(this.DangerousGetHandle(), null);
                    GC.KeepAlive(this);
                }
            }
        }
    }
}