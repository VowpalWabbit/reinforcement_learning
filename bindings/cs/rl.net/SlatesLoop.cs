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
            public static extern IntPtr CreateSlatesLoop(IntPtr config, IntPtr factoryContext);

            [DllImport("rlnetnative")]
            public static extern void DeleteSlatesLoop(IntPtr slatesLoop);

            [DllImport("rlnetnative")]
            public static extern int SlatesLoopInit(IntPtr slatesLoop, IntPtr apiStatus);

            [DllImport("rlnetnative", EntryPoint = "SlatesLoopRequestMultiSlotDecision")]
            private static extern int SlatesLoopRequestMultiSlotDecisionNative(IntPtr slatesLoop, IntPtr eventId, IntPtr contextJson, int contextJsonSize, IntPtr multiSlotResponse, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, int, IntPtr, IntPtr, int> SlatesLoopRequestMultiSlotDecisionOverride { get; set; }

            public static int SlatesLoopRequestMultiSlotDecision(IntPtr slatesLoop, IntPtr eventId, IntPtr contextJson, int contextJsonSize, IntPtr multiSlotResponse, IntPtr apiStatus)
            {
                if (SlatesLoopRequestMultiSlotDecisionOverride != null)
                {
                    return SlatesLoopRequestMultiSlotDecisionOverride(slatesLoop, eventId, contextJson, contextJsonSize, multiSlotResponse, apiStatus);
                }

                return SlatesLoopRequestMultiSlotDecisionNative(slatesLoop, eventId, contextJson, contextJsonSize, multiSlotResponse, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "SlatesLoopRequestMultiSlotDecisionWithFlags")]
            private static extern int SlatesLoopRequestMultiSlotDecisionWithFlagsNative(IntPtr slatesLoop, IntPtr eventId, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr multiSlotResponse, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, int, uint, IntPtr, IntPtr, int> SlatesLoopRequestMultiSlotDecisionWithFlagsOverride { get; set; }

            public static int SlatesLoopRequestMultiSlotDecisionWithFlags(IntPtr slatesLoop, IntPtr eventId, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr multiSlotResponse, IntPtr apiStatus)
            {
                if (SlatesLoopRequestMultiSlotDecisionWithFlagsOverride != null)
                {
                    return SlatesLoopRequestMultiSlotDecisionWithFlagsOverride(slatesLoop, eventId, contextJson, contextJsonSize, flags, multiSlotResponse, apiStatus);
                }

                return SlatesLoopRequestMultiSlotDecisionWithFlagsNative(slatesLoop, eventId, contextJson, contextJsonSize, flags, multiSlotResponse, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "SlatesLoopRequestMultiSlotDecisionWithBaselineAndFlags")]
            private static extern int SlatesLoopRequestMultiSlotDecisionWithBaselineAndFlagsNative(IntPtr slatesLoop, IntPtr eventId, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr multiSlotResponse, IntPtr baselineActions, IntPtr baselineActionsSize, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, int, uint, IntPtr, IntPtr, IntPtr, IntPtr, int> SlatesLoopRequestMultiSlotDecisionWithBaselineAndFlagsOverride { get; set; }

            public static int SlatesLoopRequestMultiSlotDecisionWithBaselineAndFlags(IntPtr slatesLoop, IntPtr eventId, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr multiSlotResponse, IntPtr baselineActions, IntPtr baselineActionsSize, IntPtr apiStatus)
            {
                if (SlatesLoopRequestMultiSlotDecisionWithBaselineAndFlagsOverride != null)
                {
                    return SlatesLoopRequestMultiSlotDecisionWithBaselineAndFlagsOverride(slatesLoop, eventId, contextJson, contextJsonSize, flags, multiSlotResponse, baselineActions, baselineActionsSize, apiStatus);
                }

                return SlatesLoopRequestMultiSlotDecisionWithBaselineAndFlagsNative(slatesLoop, eventId, contextJson, contextJsonSize, flags, multiSlotResponse, baselineActions, baselineActionsSize, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "SlatesLoopRequestMultiSlotDecisionDetailed")]
            private static extern int SlatesLoopRequestMultiSlotDecisionDetailedNative(IntPtr slatesLoop, IntPtr eventId, IntPtr contextJson, int contextJsonSize, IntPtr multiSlotResponseDetailed, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, int, IntPtr, IntPtr, int> SlatesLoopRequestMultiSlotDecisionDetailedOverride { get; set; }

            public static int SlatesLoopRequestMultiSlotDecisionDetailed(IntPtr slatesLoop, IntPtr eventId, IntPtr contextJson, int contextJsonSize, IntPtr multiSlotResponseDetailed, IntPtr apiStatus)
            {
                if (SlatesLoopRequestMultiSlotDecisionDetailedOverride != null)
                {
                    return SlatesLoopRequestMultiSlotDecisionDetailedOverride(slatesLoop, eventId, contextJson, contextJsonSize, multiSlotResponseDetailed, apiStatus);
                }

                return SlatesLoopRequestMultiSlotDecisionDetailedNative(slatesLoop, eventId, contextJson, contextJsonSize, multiSlotResponseDetailed, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "SlatesLoopRequestMultiSlotDecisionDetailedWithFlags")]
            private static extern int SlatesLoopRequestMultiSlotDecisionDetailedWithFlagsNative(IntPtr slatesLoop, IntPtr eventId, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr multiSlotResponseDetailed, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, int, uint, IntPtr, IntPtr, int> SlatesLoopRequestMultiSlotDecisionDetailedWithFlagsOverride { get; set; }

            public static int SlatesLoopRequestMultiSlotDecisionDetailedWithFlags(IntPtr slatesLoop, IntPtr eventId, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr multiSlotResponseDetailed, IntPtr apiStatus)
            {
                if (SlatesLoopRequestMultiSlotDecisionDetailedWithFlagsOverride != null)
                {
                    return SlatesLoopRequestMultiSlotDecisionDetailedWithFlagsOverride(slatesLoop, eventId, contextJson, contextJsonSize, flags, multiSlotResponseDetailed, apiStatus);
                }

                return SlatesLoopRequestMultiSlotDecisionDetailedWithFlagsNative(slatesLoop, eventId, contextJson, contextJsonSize, flags, multiSlotResponseDetailed, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "SlatesLoopRequestMultiSlotDecisionDetailedWithBaselineAndFlags")]
            private static extern int SlatesLoopRequestMultiSlotDecisionDetailedWithBaselineAndFlagsNative(IntPtr slatesLoop, IntPtr eventId, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr multiSlotResponseDetailed, IntPtr baselineActions, IntPtr baselineActionsSize, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, int, uint, IntPtr, IntPtr, IntPtr, IntPtr, int> SlatesLoopRequestMultiSlotDecisionDetailedWithBaselineAndFlagsOverride { get; set; }

            public static int SlatesLoopRequestMultiSlotDecisionDetailedWithBaselineAndFlags(IntPtr slatesLoop, IntPtr eventId, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr multiSlotResponseDetailed, IntPtr baselineActions, IntPtr baselineActionsSize, IntPtr apiStatus)
            {
                if (SlatesLoopRequestMultiSlotDecisionDetailedWithBaselineAndFlagsOverride != null)
                {
                    return SlatesLoopRequestMultiSlotDecisionDetailedWithBaselineAndFlagsOverride(slatesLoop, eventId, contextJson, contextJsonSize, flags, multiSlotResponseDetailed, baselineActions, baselineActionsSize, apiStatus);
                }

                return SlatesLoopRequestMultiSlotDecisionDetailedWithBaselineAndFlagsNative(slatesLoop, eventId, contextJson, contextJsonSize, flags, multiSlotResponseDetailed, baselineActions, baselineActionsSize, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "SlatesLoopReportActionTaken")]
            private static extern int SlatesLoopReportActionTakenNative(IntPtr slatesLoop, IntPtr eventId, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, int> SlatesLoopReportActionTakenOverride { get; set; }

            public static int SlatesLoopReportActionTaken(IntPtr slatesLoop, IntPtr eventId, IntPtr apiStatus)
            {
                if (SlatesLoopReportActionTakenOverride != null)
                {
                    return SlatesLoopReportActionTakenOverride(slatesLoop, eventId, apiStatus);
                }

                return SlatesLoopReportActionTakenNative(slatesLoop, eventId, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "SlatesLoopReportActionMultiIdTaken")]
            private static extern int SlatesLoopReportActionTakenMultiIdNative(IntPtr slatesLoop, IntPtr primaryId, IntPtr secondaryId, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, IntPtr, int> SlatesLoopReportActionTakenMultiIdOverride { get; set; }

            public static int SlatesLoopReportActionMultiIdTaken(IntPtr slatesLoop, IntPtr primaryId, IntPtr secondaryId, IntPtr apiStatus)
            {
                if (SlatesLoopReportActionTakenMultiIdOverride != null)
                {
                    return SlatesLoopReportActionTakenMultiIdOverride(slatesLoop, primaryId, secondaryId, apiStatus);
                }

                return SlatesLoopReportActionTakenMultiIdNative(slatesLoop, primaryId, secondaryId, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "SlatesLoopReportOutcomeF")]
            private static extern int SlatesLoopReportOutcomeFNative(IntPtr slatesLoop, IntPtr eventId, float outcome, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, float, IntPtr, int> SlatesLoopReportOutcomeFOverride { get; set; }

            public static int SlatesLoopReportOutcomeF(IntPtr slatesLoop, IntPtr eventId, float outcome, IntPtr apiStatus)
            {
                if (SlatesLoopReportOutcomeFOverride != null)
                {
                    return SlatesLoopReportOutcomeFOverride(slatesLoop, eventId, outcome, apiStatus);
                }

                return SlatesLoopReportOutcomeFNative(slatesLoop, eventId, outcome, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "SlatesLoopReportOutcomeJson")]
            private static extern int SlatesLoopReportOutcomeJsonNative(IntPtr slatesLoop, IntPtr eventId, IntPtr outcomeJson, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, IntPtr, int> SlatesLoopReportOutcomeJsonOverride { get; set; }

            public static int SlatesLoopReportOutcomeJson(IntPtr slatesLoop, IntPtr eventId, IntPtr outcomeJson, IntPtr apiStatus)
            {
                if (SlatesLoopReportOutcomeJsonOverride != null)
                {
                    return SlatesLoopReportOutcomeJsonOverride(slatesLoop, eventId, outcomeJson, apiStatus);
                }

                return SlatesLoopReportOutcomeJsonNative(slatesLoop, eventId, outcomeJson, apiStatus);
            }

            [DllImport("rlnetnative")]
            public static extern int SlatesLoopRefreshModel(IntPtr slatesLoop, IntPtr apiStatus);

            [DllImport("rlnetnative")]
            public static extern void SlatesLoopSetCallback(IntPtr slatesLoop, [MarshalAs(UnmanagedType.FunctionPtr)] managed_background_error_callback_t callback = null);

            [DllImport("rlnetnative")]
            public static extern void SlatesLoopSetTrace(IntPtr slatesLoop, [MarshalAs(UnmanagedType.FunctionPtr)] managed_trace_callback_t callback = null);
        }
    }

    public sealed class SlatesLoop : NativeObject<SlatesLoop>, ILoop
    {
        private readonly NativeMethods.managed_background_error_callback_t managedErrorCallback;
        private readonly NativeMethods.managed_trace_callback_t managedTraceCallback;

        private static New<SlatesLoop> BindConstructorArguments(Configuration config, FactoryContext factoryContext)
        {
            return new New<SlatesLoop>(() =>
            {
                factoryContext = factoryContext ?? new FactoryContext();
                IntPtr result = NativeMethods.CreateSlatesLoop(config.DangerousGetHandle(), factoryContext.DangerousGetHandle());

                // These references do not live on the heap in this delegate, and could disappear during the invocation
                // of CreateSlatesLoop. Thus, we need to ensure GC knows not to release them until after that call
                // returns.
                GC.KeepAlive(config);
                GC.KeepAlive(factoryContext);

                return result;
            });

        }

        public SlatesLoop(Configuration config) : this(config, null)
        { }

        public SlatesLoop(Configuration config, FactoryContext factoryContext) : base(BindConstructorArguments(config, factoryContext), new Delete<SlatesLoop>(NativeMethods.DeleteSlatesLoop))
        {
            this.managedErrorCallback = new NativeMethods.managed_background_error_callback_t(this.WrapStatusAndRaiseBackgroundError);

            // DangerousGetHandle here is trivially safe, because .Dispose() cannot be called before the object is
            // constructed.
            NativeMethods.SlatesLoopSetCallback(this.DangerousGetHandle(), this.managedErrorCallback);

            this.managedTraceCallback = new NativeMethods.managed_trace_callback_t(this.SendTrace);
        }

        private static void CheckJsonString(string json)
        {
            if (String.IsNullOrWhiteSpace(json))
            {
                throw new ArgumentException("Input json is empty", "json");
            }
        }

        unsafe private static int SlatesLoopRequestMultiSlotDecision(IntPtr slatesLoop, string eventId, string contextJson, IntPtr multiSlotResponse, IntPtr apiStatus)
        {
            CheckJsonString(contextJson);

            fixed (byte* contextJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(contextJson))
            {
                int contextJsonSize = NativeMethods.StringEncoding.GetByteCount(contextJson);
                if (eventId == null)
                {
                    return NativeMethods.SlatesLoopRequestMultiSlotDecision(slatesLoop, IntPtr.Zero, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, multiSlotResponse, apiStatus);
                }

                fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
                {
                    return NativeMethods.SlatesLoopRequestMultiSlotDecision(slatesLoop, (IntPtr)eventIdUtf8Bytes, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, multiSlotResponse, apiStatus);
                }
            }
        }

        unsafe private static int SlatesLoopRequestMultiSlotDecisionWithFlags(IntPtr slatesLoop, string eventId, string contextJson, uint flags, IntPtr multiSlotResponse, IntPtr apiStatus)
        {
            CheckJsonString(contextJson);

            fixed (byte* contextJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(contextJson))
            {
                int contextJsonSize = NativeMethods.StringEncoding.GetByteCount(contextJson);
                if (eventId == null)
                {
                    return NativeMethods.SlatesLoopRequestMultiSlotDecisionWithFlags(slatesLoop, IntPtr.Zero, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, flags, multiSlotResponse, apiStatus);
                }

                fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
                {
                    return NativeMethods.SlatesLoopRequestMultiSlotDecisionWithFlags(slatesLoop, (IntPtr)eventIdUtf8Bytes, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, flags, multiSlotResponse, apiStatus);
                }
            }
        }

        unsafe private static int SlatesLoopRequestMultiSlotDecisionWithBaselineAndFlags(IntPtr slatesLoop, string eventId, string contextJson, uint flags, IntPtr multiSlotResponse, int[] baselineActions, IntPtr apiStatus)
        {
            CheckJsonString(contextJson);

            fixed (byte* contextJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(contextJson))
            fixed (int* baselineActionsFixed = baselineActions)
            {
                int contextJsonSize = NativeMethods.StringEncoding.GetByteCount(contextJson);
                if (eventId == null)
                {
                    return NativeMethods.SlatesLoopRequestMultiSlotDecisionWithBaselineAndFlags(slatesLoop, IntPtr.Zero, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, flags, multiSlotResponse, (IntPtr)baselineActionsFixed, (IntPtr)baselineActions.Length, apiStatus);
                }

                fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
                {
                    return NativeMethods.SlatesLoopRequestMultiSlotDecisionWithBaselineAndFlags(slatesLoop, (IntPtr)eventIdUtf8Bytes, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, flags, multiSlotResponse, (IntPtr)baselineActionsFixed, (IntPtr)baselineActions.Length, apiStatus);
                }
            }
        }

        unsafe private static int SlatesLoopRequestMultiSlotDecisionDetailed(IntPtr slatesLoop, string eventId, string contextJson, IntPtr multiSlotResponseDetailed, IntPtr apiStatus)
        {
            CheckJsonString(contextJson);

            fixed (byte* contextJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(contextJson))
            {
                int contextJsonSize = NativeMethods.StringEncoding.GetByteCount(contextJson);
                if (eventId == null)
                {
                    return NativeMethods.SlatesLoopRequestMultiSlotDecisionDetailed(slatesLoop, IntPtr.Zero, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, multiSlotResponseDetailed, apiStatus);
                }

                fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
                {
                    return NativeMethods.SlatesLoopRequestMultiSlotDecisionDetailed(slatesLoop, (IntPtr)eventIdUtf8Bytes, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, multiSlotResponseDetailed, apiStatus);
                }
            }
        }

        unsafe private static int SlatesLoopRequestMultiSlotDecisionDetailedWithFlags(IntPtr slatesLoop, string eventId, string contextJson, uint flags, IntPtr multiSlotResponseDetailed, IntPtr apiStatus)
        {
            CheckJsonString(contextJson);

            fixed (byte* contextJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(contextJson))
            {
                int contextJsonSize = NativeMethods.StringEncoding.GetByteCount(contextJson);
                if (eventId == null)
                {
                    return NativeMethods.SlatesLoopRequestMultiSlotDecisionDetailedWithFlags(slatesLoop, IntPtr.Zero, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, flags, multiSlotResponseDetailed, apiStatus);
                }

                fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
                {
                    return NativeMethods.SlatesLoopRequestMultiSlotDecisionDetailedWithFlags(slatesLoop, (IntPtr)eventIdUtf8Bytes, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, flags, multiSlotResponseDetailed, apiStatus);
                }
            }
        }

        unsafe private static int SlatesLoopRequestMultiSlotDecisionDetailedWithBaselineAndFlags(IntPtr slatesLoop, string eventId, string contextJson, uint flags, IntPtr multiSlotResponseDetailed, int[] baselineActions, IntPtr apiStatus)
        {
            CheckJsonString(contextJson);

            fixed (byte* contextJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(contextJson))
            fixed (int* baselineActionsFixed = baselineActions)
            {
                int contextJsonSize = NativeMethods.StringEncoding.GetByteCount(contextJson);
                if (eventId == null)
                {
                    return NativeMethods.SlatesLoopRequestMultiSlotDecisionDetailedWithBaselineAndFlags(slatesLoop, IntPtr.Zero, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, flags, multiSlotResponseDetailed, (IntPtr)baselineActionsFixed, (IntPtr)baselineActions.Length, apiStatus);
                }

                fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
                {
                    return NativeMethods.SlatesLoopRequestMultiSlotDecisionDetailedWithBaselineAndFlags(slatesLoop, (IntPtr)eventIdUtf8Bytes, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, flags, multiSlotResponseDetailed, (IntPtr)baselineActionsFixed, (IntPtr)baselineActions.Length, apiStatus);
                }
            }
        }

        unsafe private static int SlatesLoopReportActionTaken(IntPtr slatesLoop, string eventId, IntPtr apiStatus)
        {
            if (eventId == null)
            {
                throw new ArgumentNullException("eventId");
            }

            fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
            {
                return NativeMethods.SlatesLoopReportActionTaken(slatesLoop, new IntPtr(eventIdUtf8Bytes), apiStatus);
            }
        }

        unsafe private static int SlatesLoopReportActionMultiIdTaken(IntPtr slatesLoop, string primaryId, string secondaryId, IntPtr apiStatus)
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
                return NativeMethods.SlatesLoopReportActionMultiIdTaken(slatesLoop, new IntPtr(episodeIdUtf8Bytes), new IntPtr(eventIdUtf8Bytes), apiStatus);
            }
        }

        unsafe private static int SlatesLoopReportOutcomeF(IntPtr slatesLoop, string eventId, float outcome, IntPtr apiStatus)
        {
            if (eventId == null)
            {
                throw new ArgumentNullException("eventId");
            }

            fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
            {
                return NativeMethods.SlatesLoopReportOutcomeF(slatesLoop, new IntPtr(eventIdUtf8Bytes), outcome, apiStatus);
            }
        }

        unsafe private static int SlatesLoopReportOutcomeJson(IntPtr slatesLoop, string eventId, string outcomeJson, IntPtr apiStatus)
        {
            if (eventId == null)
            {
                throw new ArgumentNullException("eventId");
            }

            CheckJsonString(outcomeJson);

            fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
            fixed (byte* outcomeJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(outcomeJson))
            {
                return NativeMethods.SlatesLoopReportOutcomeJson(slatesLoop, new IntPtr(eventIdUtf8Bytes), new IntPtr(outcomeJsonUtf8Bytes), apiStatus);
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
            int result = NativeMethods.SlatesLoopInit(this.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

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

        public bool TryRequestMultiSlotDecision(string eventId, string contextJson, out MultiSlotResponse response, ApiStatus apiStatus = null)
        {
            response = new MultiSlotResponse();
            return this.TryRequestMultiSlotDecision(eventId, contextJson, response, apiStatus);
        }

        public bool TryRequestMultiSlotDecision(string eventId, string contextJson, MultiSlotResponse response, ApiStatus apiStatus = null)
        {
            int result = SlatesLoopRequestMultiSlotDecision(this.DangerousGetHandle(), eventId, contextJson, response.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

            GC.KeepAlive(this);
            return result == NativeMethods.SuccessStatus;
        }

        public MultiSlotResponse RequestMultiSlotDecision(string eventId, string contextJson)
        {
            MultiSlotResponse result = new MultiSlotResponse();

            using (ApiStatus apiStatus = new ApiStatus())
                if (!this.TryRequestMultiSlotDecision(eventId, contextJson, result, apiStatus))
                {
                    throw new RLException(apiStatus);
                }

            return result;
        }

        public bool TryRequestMultiSlotDecision(string eventId, string contextJson, ActionFlags flags, out MultiSlotResponse response, ApiStatus apiStatus)
        {
            response = new MultiSlotResponse();
            return this.TryRequestMultiSlotDecision(eventId, contextJson, flags, response, apiStatus);
        }

        public bool TryRequestMultiSlotDecision(string eventId, string contextJson, ActionFlags flags, MultiSlotResponse response, ApiStatus apiStatus)
        {
            int result = SlatesLoopRequestMultiSlotDecisionWithFlags(this.DangerousGetHandle(), eventId, contextJson, (uint)flags, response.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

            GC.KeepAlive(this);
            return result == NativeMethods.SuccessStatus;
        }

        public MultiSlotResponse RequestMultiSlotDecision(string eventId, string contextJson, ActionFlags flags)
        {
            MultiSlotResponse result = new MultiSlotResponse();

            using (ApiStatus apiStatus = new ApiStatus())
                if (!this.TryRequestMultiSlotDecision(eventId, contextJson, flags, result, apiStatus))
                {
                    throw new RLException(apiStatus);
                }

            return result;
        }

        public bool TryRequestMultiSlotDecision(string eventId, string contextJson, ActionFlags flags, out MultiSlotResponse response, int[] baselineActions, ApiStatus apiStatus)
        {
            response = new MultiSlotResponse();
            return this.TryRequestMultiSlotDecision(eventId, contextJson, flags, response, baselineActions, apiStatus);
        }

        public bool TryRequestMultiSlotDecision(string eventId, string contextJson, ActionFlags flags, MultiSlotResponse response, int[] baselineActions, ApiStatus apiStatus)
        {
            int result = SlatesLoopRequestMultiSlotDecisionWithBaselineAndFlags(this.DangerousGetHandle(), eventId, contextJson, (uint)flags, response.DangerousGetHandle(), baselineActions, apiStatus.ToNativeHandleOrNullptrDangerous());

            GC.KeepAlive(this);
            return result == NativeMethods.SuccessStatus;
        }

        public MultiSlotResponse RequestMultiSlotDecision(string eventId, string contextJson, ActionFlags flags, int[] baselineActions)
        {
            MultiSlotResponse result = new MultiSlotResponse();

            using (ApiStatus apiStatus = new ApiStatus())
                if (!this.TryRequestMultiSlotDecision(eventId, contextJson, flags, result, baselineActions, apiStatus))
                {
                    throw new RLException(apiStatus);
                }

            return result;
        }

        public bool TryRequestMultiSlotDecisionDetailed(string eventId, string contextJson, out MultiSlotResponseDetailed response, ApiStatus apiStatus = null)
        {
            response = new MultiSlotResponseDetailed();
            return TryRequestMultiSlotDecisionDetailed(eventId, contextJson, response, apiStatus);
        }

        public bool TryRequestMultiSlotDecisionDetailed(string eventId, string contextJson, MultiSlotResponseDetailed response, ApiStatus apiStatus = null)
        {
            int result = SlatesLoopRequestMultiSlotDecisionDetailed(this.DangerousGetHandle(), eventId, contextJson, response.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

            GC.KeepAlive(this);
            return result == NativeMethods.SuccessStatus;
        }

        public MultiSlotResponseDetailed RequestMultiSlotDecisionDetailed(string eventId, string contextJson)
        {
            MultiSlotResponseDetailed result = new MultiSlotResponseDetailed();

            using (ApiStatus apiStatus = new ApiStatus())
                if (!TryRequestMultiSlotDecisionDetailed(eventId, contextJson, result, apiStatus))
                {
                    throw new RLException(apiStatus);
                }

            return result;
        }

        public bool TryRequestMultiSlotDecisionDetailed(string eventId, string contextJson, ActionFlags flags, out MultiSlotResponseDetailed response, ApiStatus apiStatus)
        {
            response = new MultiSlotResponseDetailed();
            return this.TryRequestMultiSlotDecisionDetailed(eventId, contextJson, flags, response, apiStatus);
        }

        public bool TryRequestMultiSlotDecisionDetailed(string eventId, string contextJson, ActionFlags flags, MultiSlotResponseDetailed response, ApiStatus apiStatus)
        {
            int result = SlatesLoopRequestMultiSlotDecisionDetailedWithFlags(this.DangerousGetHandle(), eventId, contextJson, (uint)flags, response.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

            GC.KeepAlive(this);
            return result == NativeMethods.SuccessStatus;
        }

        public MultiSlotResponseDetailed RequestMultiSlotDecisionDetailed(string eventId, string contextJson, ActionFlags flags)
        {
            MultiSlotResponseDetailed result = new MultiSlotResponseDetailed();

            using (ApiStatus apiStatus = new ApiStatus())
                if (!this.TryRequestMultiSlotDecisionDetailed(eventId, contextJson, flags, result, apiStatus))
                {
                    throw new RLException(apiStatus);
                }

            return result;
        }

        public bool TryRequestMultiSlotDecisionDetailed(string eventId, string contextJson, ActionFlags flags, out MultiSlotResponseDetailed response, int[] baselineActions, ApiStatus apiStatus)
        {
            response = new MultiSlotResponseDetailed();
            return this.TryRequestMultiSlotDecisionDetailed(eventId, contextJson, flags, response, baselineActions, apiStatus);
        }

        public bool TryRequestMultiSlotDecisionDetailed(string eventId, string contextJson, ActionFlags flags, MultiSlotResponseDetailed response, int[] baselineActions, ApiStatus apiStatus)
        {
            int result = SlatesLoopRequestMultiSlotDecisionDetailedWithBaselineAndFlags(this.DangerousGetHandle(), eventId, contextJson, (uint)flags, response.DangerousGetHandle(), baselineActions, apiStatus.ToNativeHandleOrNullptrDangerous());
            GC.KeepAlive(this);
            return result == NativeMethods.SuccessStatus;
        }

        public MultiSlotResponseDetailed RequestMultiSlotDecisionDetailed(string eventId, string contextJson, ActionFlags flags, int[] baselineActions)
        {
            MultiSlotResponseDetailed result = new MultiSlotResponseDetailed();

            using (ApiStatus apiStatus = new ApiStatus())
                if (!this.TryRequestMultiSlotDecisionDetailed(eventId, contextJson, flags, result, baselineActions, apiStatus))
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
            int result = SlatesLoopReportActionTaken(this.DangerousGetHandle(), eventId, apiStatus.ToNativeHandleOrNullptrDangerous());

            GC.KeepAlive(apiStatus);
            GC.KeepAlive(this);
            return result == NativeMethods.SuccessStatus;
        }

        public bool TryQueueActionTakenEvent(string primaryId, string secondaryId, ApiStatus apiStatus = null)
        {
            int result = SlatesLoopReportActionMultiIdTaken(this.DangerousGetHandle(), primaryId, secondaryId, apiStatus.ToNativeHandleOrNullptrDangerous());

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
            int result = SlatesLoopReportOutcomeF(this.DangerousGetHandle(), eventId, outcome, apiStatus.ToNativeHandleOrNullptrDangerous());

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
            int result = SlatesLoopReportOutcomeJson(this.DangerousGetHandle(), eventId, outcomeJson, apiStatus.ToNativeHandleOrNullptrDangerous());

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
            int result = NativeMethods.SlatesLoopRefreshModel(this.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

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
                    NativeMethods.SlatesLoopSetTrace(this.DangerousGetHandle(), this.managedTraceCallback);
                    GC.KeepAlive(this);
                }

                this.OnTraceLoggerEventInternal += value;
            }
            remove
            {
                this.OnTraceLoggerEventInternal -= value;

                if (this.OnTraceLoggerEventInternal == null)
                {
                    NativeMethods.SlatesLoopSetTrace(this.DangerousGetHandle(), null);
                    GC.KeepAlive(this);
                }
            }
        }
    }
}