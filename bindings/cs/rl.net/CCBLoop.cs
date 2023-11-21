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
            public static extern IntPtr CreateCCBLoop(IntPtr config, IntPtr factoryContext);

            [DllImport("rlnetnative")]
            public static extern void DeleteCCBLoop(IntPtr ccbLoop);

            [DllImport("rlnetnative")]
            public static extern int CCBLoopInit(IntPtr ccbLoop, IntPtr apiStatus);

            [DllImport("rlnetnative", EntryPoint = "CCBLoopRequestDecision")]
            private static extern int CCBLoopRequestDecisionNative(IntPtr ccbLoop, IntPtr contextJson, int contextJsonSize, IntPtr decisionResponse, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, int, IntPtr, IntPtr, int> CCBLoopRequestDecisionOverride { get; set; }

            public static int CCBLoopRequestDecision(IntPtr ccbLoop, IntPtr contextJson, int contextJsonSize, IntPtr decisionResponse, IntPtr apiStatus)
            {
                if (CCBLoopRequestDecisionOverride != null)
                {
                    return CCBLoopRequestDecisionOverride(ccbLoop, contextJson, contextJsonSize, decisionResponse, apiStatus);
                }

                return CCBLoopRequestDecisionNative(ccbLoop, contextJson, contextJsonSize, decisionResponse, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "CCBLoopRequestDecisionWithFlags")]
            private static extern int CCBLoopRequestDecisionWithFlagsNative(IntPtr ccbLoop, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr decisionResponse, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, int, uint, IntPtr, IntPtr, int> CCBLoopRequestDecisionWithFlagsOverride { get; set; }

            public static int CCBLoopRequestDecisionWithFlags(IntPtr ccbLoop, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr decisionResponse, IntPtr apiStatus)
            {
                if (CCBLoopRequestDecisionWithFlagsOverride != null)
                {
                    return CCBLoopRequestDecisionWithFlagsOverride(ccbLoop, contextJson, contextJsonSize, flags, decisionResponse, apiStatus);
                }

                return CCBLoopRequestDecisionWithFlagsNative(ccbLoop, contextJson, contextJsonSize, flags, decisionResponse, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "CCBLoopRequestMultiSlotDecision")]
            private static extern int CCBLoopRequestMultiSlotDecisionNative(IntPtr ccbLoop, IntPtr eventId, IntPtr contextJson, int contextJsonSize, IntPtr multiSlotResponse, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, int, IntPtr, IntPtr, int> CCBLoopRequestMultiSlotDecisionOverride { get; set; }

            public static int CCBLoopRequestMultiSlotDecision(IntPtr ccbLoop, IntPtr eventId, IntPtr contextJson, int contextJsonSize, IntPtr multiSlotResponse, IntPtr apiStatus)
            {
                if (CCBLoopRequestMultiSlotDecisionOverride != null)
                {
                    return CCBLoopRequestMultiSlotDecisionOverride(ccbLoop, eventId, contextJson, contextJsonSize, multiSlotResponse, apiStatus);
                }

                return CCBLoopRequestMultiSlotDecisionNative(ccbLoop, eventId, contextJson, contextJsonSize, multiSlotResponse, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "CCBLoopRequestMultiSlotDecisionWithFlags")]
            private static extern int CCBLoopRequestMultiSlotDecisionWithFlagsNative(IntPtr ccbLoop, IntPtr eventId, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr multiSlotResponse, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, int, uint, IntPtr, IntPtr, int> CCBLoopRequestMultiSlotDecisionWithFlagsOverride { get; set; }

            public static int CCBLoopRequestMultiSlotDecisionWithFlags(IntPtr ccbLoop, IntPtr eventId, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr multiSlotResponse, IntPtr apiStatus)
            {
                if (CCBLoopRequestMultiSlotDecisionWithFlagsOverride != null)
                {
                    return CCBLoopRequestMultiSlotDecisionWithFlagsOverride(ccbLoop, eventId, contextJson, contextJsonSize, flags, multiSlotResponse, apiStatus);
                }

                return CCBLoopRequestMultiSlotDecisionWithFlagsNative(ccbLoop, eventId, contextJson, contextJsonSize, flags, multiSlotResponse, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "CCBLoopRequestMultiSlotDecisionWithBaselineAndFlags")]
            private static extern int CCBLoopRequestMultiSlotDecisionWithBaselineAndFlagsNative(IntPtr ccbLoop, IntPtr eventId, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr multiSlotResponse, IntPtr baselineActions, IntPtr baselineActionsSize, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, int, uint, IntPtr, IntPtr, IntPtr, IntPtr, int> CCBLoopRequestMultiSlotDecisionWithBaselineAndFlagsOverride { get; set; }

            public static int CCBLoopRequestMultiSlotDecisionWithBaselineAndFlags(IntPtr ccbLoop, IntPtr eventId, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr multiSlotResponse, IntPtr baselineActions, IntPtr baselineActionsSize, IntPtr apiStatus)
            {
                if (CCBLoopRequestMultiSlotDecisionWithBaselineAndFlagsOverride != null)
                {
                    return CCBLoopRequestMultiSlotDecisionWithBaselineAndFlagsOverride(ccbLoop, eventId, contextJson, contextJsonSize, flags, multiSlotResponse, baselineActions, baselineActionsSize, apiStatus);
                }

                return CCBLoopRequestMultiSlotDecisionWithBaselineAndFlagsNative(ccbLoop, eventId, contextJson, contextJsonSize, flags, multiSlotResponse, baselineActions, baselineActionsSize, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "CCBLoopRequestMultiSlotDecisionDetailed")]
            private static extern int CCBLoopRequestMultiSlotDecisionDetailedNative(IntPtr ccbLoop, IntPtr eventId, IntPtr contextJson, int contextJsonSize, IntPtr multiSlotResponseDetailed, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, int, IntPtr, IntPtr, int> CCBLoopRequestMultiSlotDecisionDetailedOverride { get; set; }

            public static int CCBLoopRequestMultiSlotDecisionDetailed(IntPtr ccbLoop, IntPtr eventId, IntPtr contextJson, int contextJsonSize, IntPtr multiSlotResponseDetailed, IntPtr apiStatus)
            {
                if (CCBLoopRequestMultiSlotDecisionDetailedOverride != null)
                {
                    return CCBLoopRequestMultiSlotDecisionDetailedOverride(ccbLoop, eventId, contextJson, contextJsonSize, multiSlotResponseDetailed, apiStatus);
                }

                return CCBLoopRequestMultiSlotDecisionDetailedNative(ccbLoop, eventId, contextJson, contextJsonSize, multiSlotResponseDetailed, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "CCBLoopRequestMultiSlotDecisionDetailedWithFlags")]
            private static extern int CCBLoopRequestMultiSlotDecisionDetailedWithFlagsNative(IntPtr ccbLoop, IntPtr eventId, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr multiSlotResponseDetailed, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, int, uint, IntPtr, IntPtr, int> CCBLoopRequestMultiSlotDecisionDetailedWithFlagsOverride { get; set; }

            public static int CCBLoopRequestMultiSlotDecisionDetailedWithFlags(IntPtr ccbLoop, IntPtr eventId, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr multiSlotResponseDetailed, IntPtr apiStatus)
            {
                if (CCBLoopRequestMultiSlotDecisionDetailedWithFlagsOverride != null)
                {
                    return CCBLoopRequestMultiSlotDecisionDetailedWithFlagsOverride(ccbLoop, eventId, contextJson, contextJsonSize, flags, multiSlotResponseDetailed, apiStatus);
                }

                return CCBLoopRequestMultiSlotDecisionDetailedWithFlagsNative(ccbLoop, eventId, contextJson, contextJsonSize, flags, multiSlotResponseDetailed, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "CCBLoopRequestMultiSlotDecisionDetailedWithBaselineAndFlags")]
            private static extern int CCBLoopRequestMultiSlotDecisionDetailedWithBaselineAndFlagsNative(IntPtr ccbLoop, IntPtr eventId, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr multiSlotResponseDetailed, IntPtr baselineActions, IntPtr baselineActionsSize, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, int, uint, IntPtr, IntPtr, IntPtr, IntPtr, int> CCBLoopRequestMultiSlotDecisionDetailedWithBaselineAndFlagsOverride { get; set; }

            public static int CCBLoopRequestMultiSlotDecisionDetailedWithBaselineAndFlags(IntPtr ccbLoop, IntPtr eventId, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr multiSlotResponseDetailed, IntPtr baselineActions, IntPtr baselineActionsSize, IntPtr apiStatus)
            {
                if (CCBLoopRequestMultiSlotDecisionDetailedWithBaselineAndFlagsOverride != null)
                {
                    return CCBLoopRequestMultiSlotDecisionDetailedWithBaselineAndFlagsOverride(ccbLoop, eventId, contextJson, contextJsonSize, flags, multiSlotResponseDetailed, baselineActions, baselineActionsSize, apiStatus);
                }

                return CCBLoopRequestMultiSlotDecisionDetailedWithBaselineAndFlagsNative(ccbLoop, eventId, contextJson, contextJsonSize, flags, multiSlotResponseDetailed, baselineActions, baselineActionsSize, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "CCBLoopReportActionTaken")]
            private static extern int CCBLoopReportActionTakenNative(IntPtr ccbLoop, IntPtr eventId, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, int> CCBLoopReportActionTakenOverride { get; set; }

            public static int CCBLoopReportActionTaken(IntPtr ccbLoop, IntPtr eventId, IntPtr apiStatus)
            {
                if (CCBLoopReportActionTakenOverride != null)
                {
                    return CCBLoopReportActionTakenOverride(ccbLoop, eventId, apiStatus);
                }

                return CCBLoopReportActionTakenNative(ccbLoop, eventId, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "CCBLoopReportActionMultiIdTaken")]
            private static extern int CCBLoopReportActionTakenMultiIdNative(IntPtr ccbLoop, IntPtr primaryId, IntPtr secondaryId, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, IntPtr, int> CCBLoopReportActionTakenMultiIdOverride { get; set; }

            public static int CCBLoopReportActionMultiIdTaken(IntPtr ccbLoop, IntPtr primaryId, IntPtr secondaryId, IntPtr apiStatus)
            {
                if (CCBLoopReportActionTakenMultiIdOverride != null)
                {
                    return CCBLoopReportActionTakenMultiIdOverride(ccbLoop, primaryId, secondaryId, apiStatus);
                }

                return CCBLoopReportActionTakenMultiIdNative(ccbLoop, primaryId, secondaryId, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "CCBLoopReportOutcomeSlotF")]
            private static extern int CCBLoopReportOutcomeSlotFNative(IntPtr ccbLoop, IntPtr eventId, uint slotIndex, float outcome, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, uint, float, IntPtr, int> CCBLoopReportOutcomeSlotFOverride { get; set; }

            public static int CCBLoopReportOutcomeSlotF(IntPtr ccbLoop, IntPtr eventId, uint slotIndex, float outcome, IntPtr apiStatus)
            {
                if (CCBLoopReportOutcomeSlotFOverride != null)
                {
                    return CCBLoopReportOutcomeSlotFOverride(ccbLoop, eventId, slotIndex, outcome, apiStatus);
                }

                return CCBLoopReportOutcomeSlotFNative(ccbLoop, eventId, slotIndex, outcome, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "CCBLoopReportOutcomeSlotJson")]
            private static extern int CCBLoopReportOutcomeSlotJsonNative(IntPtr ccbLoop, IntPtr eventId, uint slotIndex, IntPtr outcomeJson, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, uint, IntPtr, IntPtr, int> CCBLoopReportOutcomeSlotJsonOverride { get; set; }

            public static int CCBLoopReportOutcomeSlotJson(IntPtr ccbLoop, IntPtr eventId, uint slotIndex, IntPtr outcomeJson, IntPtr apiStatus)
            {
                if (CCBLoopReportOutcomeSlotJsonOverride != null)
                {
                    return CCBLoopReportOutcomeSlotJsonOverride(ccbLoop, eventId, slotIndex, outcomeJson, apiStatus);
                }

                return CCBLoopReportOutcomeSlotJsonNative(ccbLoop, eventId, slotIndex, outcomeJson, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "CCBLoopReportOutcomeSlotStringIdF")]
            private static extern int CCBLoopReportOutcomeSlotStringIdFNative(IntPtr ccbLoop, IntPtr eventId, IntPtr slotId, float outcome, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, float, IntPtr, int> CCBLoopReportOutcomeSlotStringIdFOverride { get; set; }

            public static int CCBLoopReportOutcomeSlotStringIdF(IntPtr ccbLoop, IntPtr eventId, IntPtr slotId, float outcome, IntPtr apiStatus)
            {
                if (CCBLoopReportOutcomeSlotStringIdFOverride != null)
                {
                    return CCBLoopReportOutcomeSlotStringIdFOverride(ccbLoop, eventId, slotId, outcome, apiStatus);
                }

                return CCBLoopReportOutcomeSlotStringIdFNative(ccbLoop, eventId, slotId, outcome, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "CCBLoopReportOutcomeSlotStringIdJson")]
            private static extern int CCBLoopReportOutcomeSlotStringIdJsonNative(IntPtr ccbLoop, IntPtr eventId, IntPtr slotId, IntPtr outcomeJson, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, IntPtr, IntPtr, int> CCBLoopReportOutcomeSlotStringIdJsonOverride { get; set; }

            public static int CCBLoopReportOutcomeSlotStringIdJson(IntPtr ccbLoop, IntPtr eventId, IntPtr slotId, IntPtr outcomeJson, IntPtr apiStatus)
            {
                if (CCBLoopReportOutcomeSlotStringIdJsonOverride != null)
                {
                    return CCBLoopReportOutcomeSlotStringIdJsonOverride(ccbLoop, eventId, slotId, outcomeJson, apiStatus);
                }

                return CCBLoopReportOutcomeSlotStringIdJsonNative(ccbLoop, eventId, slotId, outcomeJson, apiStatus);
            }

            [DllImport("rlnetnative")]
            public static extern int CCBLoopRefreshModel(IntPtr ccbLoop, IntPtr apiStatus);

            [DllImport("rlnetnative")]
            public static extern void CCBLoopSetCallback(IntPtr ccbLoop, [MarshalAs(UnmanagedType.FunctionPtr)] managed_background_error_callback_t callback = null);

            [DllImport("rlnetnative")]
            public static extern void CCBLoopSetTrace(IntPtr ccbLoop, [MarshalAs(UnmanagedType.FunctionPtr)] managed_trace_callback_t callback = null);
        }
    }

    public sealed class CCBLoop : NativeObject<CCBLoop>, ILoop
    {
        private readonly NativeMethods.managed_background_error_callback_t managedErrorCallback;
        private readonly NativeMethods.managed_trace_callback_t managedTraceCallback;

        private static New<CCBLoop> BindConstructorArguments(Configuration config, FactoryContext factoryContext)
        {
            return new New<CCBLoop>(() =>
            {
                factoryContext = factoryContext ?? new FactoryContext();
                IntPtr result = NativeMethods.CreateCCBLoop(config.DangerousGetHandle(), factoryContext.DangerousGetHandle());

                // These references do not live on the heap in this delegate, and could disappear during the invocation
                // of CreateCCBLoop. Thus, we need to ensure GC knows not to release them until after that call
                // returns.
                GC.KeepAlive(config);
                GC.KeepAlive(factoryContext);

                return result;
            });

        }

        public CCBLoop(Configuration config) : this(config, null)
        { }

        public CCBLoop(Configuration config, FactoryContext factoryContext) : base(BindConstructorArguments(config, factoryContext), new Delete<CCBLoop>(NativeMethods.DeleteCCBLoop))
        {
            this.managedErrorCallback = new NativeMethods.managed_background_error_callback_t(this.WrapStatusAndRaiseBackgroundError);

            // DangerousGetHandle here is trivially safe, because .Dispose() cannot be called before the object is
            // constructed.
            NativeMethods.CCBLoopSetCallback(this.DangerousGetHandle(), this.managedErrorCallback);

            this.managedTraceCallback = new NativeMethods.managed_trace_callback_t(this.SendTrace);
        }

        private static void CheckJsonString(string json)
        {
            if (String.IsNullOrWhiteSpace(json))
            {
                throw new ArgumentException("Input json is empty", "json");
            }
        }

        unsafe private static int CCBLoopRequestDecision(IntPtr ccbLoop, string contextJson, IntPtr decisionResponse, IntPtr apiStatus)
        {
            CheckJsonString(contextJson);

            fixed (byte* contextJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(contextJson))
            {
                int contextJsonSize = NativeMethods.StringEncoding.GetByteCount(contextJson);
                return NativeMethods.CCBLoopRequestDecision(ccbLoop, new IntPtr(contextJsonUtf8Bytes), contextJsonSize, decisionResponse, apiStatus);
            }
        }

        unsafe private static int CCBLoopRequestDecisionWithFlags(IntPtr ccbLoop, string contextJson, uint flags, IntPtr decisionResponse, IntPtr apiStatus)
        {
            CheckJsonString(contextJson);

            fixed (byte* contextJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(contextJson))
            {
                int contextJsonSize = NativeMethods.StringEncoding.GetByteCount(contextJson);
                return NativeMethods.CCBLoopRequestDecisionWithFlags(ccbLoop, new IntPtr(contextJsonUtf8Bytes), contextJsonSize, flags, decisionResponse, apiStatus);
            }
        }

        unsafe private static int CCBLoopRequestMultiSlotDecision(IntPtr ccbLoop, string eventId, string contextJson, IntPtr multiSlotResponse, IntPtr apiStatus)
        {
            CheckJsonString(contextJson);

            fixed (byte* contextJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(contextJson))
            {
                int contextJsonSize = NativeMethods.StringEncoding.GetByteCount(contextJson);
                if (eventId == null)
                {
                    return NativeMethods.CCBLoopRequestMultiSlotDecision(ccbLoop, IntPtr.Zero, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, multiSlotResponse, apiStatus);
                }

                fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
                {
                    return NativeMethods.CCBLoopRequestMultiSlotDecision(ccbLoop, (IntPtr)eventIdUtf8Bytes, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, multiSlotResponse, apiStatus);
                }
            }
        }

        unsafe private static int CCBLoopRequestMultiSlotDecisionWithFlags(IntPtr ccbLoop, string eventId, string contextJson, uint flags, IntPtr multiSlotResponse, IntPtr apiStatus)
        {
            CheckJsonString(contextJson);

            fixed (byte* contextJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(contextJson))
            {
                int contextJsonSize = NativeMethods.StringEncoding.GetByteCount(contextJson);
                if (eventId == null)
                {
                    return NativeMethods.CCBLoopRequestMultiSlotDecisionWithFlags(ccbLoop, IntPtr.Zero, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, flags, multiSlotResponse, apiStatus);
                }

                fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
                {
                    return NativeMethods.CCBLoopRequestMultiSlotDecisionWithFlags(ccbLoop, (IntPtr)eventIdUtf8Bytes, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, flags, multiSlotResponse, apiStatus);
                }
            }
        }

        unsafe private static int CCBLoopRequestMultiSlotDecisionWithBaselineAndFlags(IntPtr ccbLoop, string eventId, string contextJson, uint flags, IntPtr multiSlotResponse, int[] baselineActions, IntPtr apiStatus)
        {
            CheckJsonString(contextJson);

            fixed (byte* contextJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(contextJson))
            fixed (int* baselineActionsFixed = baselineActions)
            {
                int contextJsonSize = NativeMethods.StringEncoding.GetByteCount(contextJson);
                if (eventId == null)
                {
                    return NativeMethods.CCBLoopRequestMultiSlotDecisionWithBaselineAndFlags(ccbLoop, IntPtr.Zero, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, flags, multiSlotResponse, (IntPtr)baselineActionsFixed, (IntPtr)baselineActions.Length, apiStatus);
                }

                fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
                {
                    return NativeMethods.CCBLoopRequestMultiSlotDecisionWithBaselineAndFlags(ccbLoop, (IntPtr)eventIdUtf8Bytes, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, flags, multiSlotResponse, (IntPtr)baselineActionsFixed, (IntPtr)baselineActions.Length, apiStatus);
                }
            }
        }

        unsafe private static int CCBLoopRequestMultiSlotDecisionDetailed(IntPtr ccbLoop, string eventId, string contextJson, IntPtr multiSlotResponseDetailed, IntPtr apiStatus)
        {
            CheckJsonString(contextJson);

            fixed (byte* contextJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(contextJson))
            {
                int contextJsonSize = NativeMethods.StringEncoding.GetByteCount(contextJson);
                if (eventId == null)
                {
                    return NativeMethods.CCBLoopRequestMultiSlotDecisionDetailed(ccbLoop, IntPtr.Zero, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, multiSlotResponseDetailed, apiStatus);
                }

                fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
                {
                    return NativeMethods.CCBLoopRequestMultiSlotDecisionDetailed(ccbLoop, (IntPtr)eventIdUtf8Bytes, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, multiSlotResponseDetailed, apiStatus);
                }
            }
        }

        unsafe private static int CCBLoopRequestMultiSlotDecisionDetailedWithFlags(IntPtr ccbLoop, string eventId, string contextJson, uint flags, IntPtr multiSlotResponseDetailed, IntPtr apiStatus)
        {
            CheckJsonString(contextJson);

            fixed (byte* contextJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(contextJson))
            {
                int contextJsonSize = NativeMethods.StringEncoding.GetByteCount(contextJson);
                if (eventId == null)
                {
                    return NativeMethods.CCBLoopRequestMultiSlotDecisionDetailedWithFlags(ccbLoop, IntPtr.Zero, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, flags, multiSlotResponseDetailed, apiStatus);
                }

                fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
                {
                    return NativeMethods.CCBLoopRequestMultiSlotDecisionDetailedWithFlags(ccbLoop, (IntPtr)eventIdUtf8Bytes, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, flags, multiSlotResponseDetailed, apiStatus);
                }
            }
        }

        unsafe private static int CCBLoopRequestMultiSlotDecisionDetailedWithBaselineAndFlags(IntPtr ccbLoop, string eventId, string contextJson, uint flags, IntPtr multiSlotResponseDetailed, int[] baselineActions, IntPtr apiStatus)
        {
            CheckJsonString(contextJson);

            fixed (byte* contextJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(contextJson))
            fixed (int* baselineActionsFixed = baselineActions)
            {
                int contextJsonSize = NativeMethods.StringEncoding.GetByteCount(contextJson);
                if (eventId == null)
                {
                    return NativeMethods.CCBLoopRequestMultiSlotDecisionDetailedWithBaselineAndFlags(ccbLoop, IntPtr.Zero, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, flags, multiSlotResponseDetailed, (IntPtr)baselineActionsFixed, (IntPtr)baselineActions.Length, apiStatus);
                }

                fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
                {
                    return NativeMethods.CCBLoopRequestMultiSlotDecisionDetailedWithBaselineAndFlags(ccbLoop, (IntPtr)eventIdUtf8Bytes, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, flags, multiSlotResponseDetailed, (IntPtr)baselineActionsFixed, (IntPtr)baselineActions.Length, apiStatus);
                }
            }
        }

        unsafe private static int CCBLoopReportActionTaken(IntPtr ccbLoop, string eventId, IntPtr apiStatus)
        {
            if (eventId == null)
            {
                throw new ArgumentNullException("eventId");
            }

            fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
            {
                return NativeMethods.CCBLoopReportActionTaken(ccbLoop, new IntPtr(eventIdUtf8Bytes), apiStatus);
            }
        }

        unsafe private static int CCBLoopReportActionMultiIdTaken(IntPtr ccbLoop, string primaryId, string secondaryId, IntPtr apiStatus)
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
                return NativeMethods.CCBLoopReportActionMultiIdTaken(ccbLoop, new IntPtr(episodeIdUtf8Bytes), new IntPtr(eventIdUtf8Bytes), apiStatus);
            }
        }

        unsafe private static int CCBLoopReportOutcomeSlotF(IntPtr ccbLoop, string eventId, uint slotIndex, float outcome, IntPtr apiStatus)
        {
            if (eventId == null)
            {
                throw new ArgumentNullException("eventId");
            }

            fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
            {
                return NativeMethods.CCBLoopReportOutcomeSlotF(ccbLoop, new IntPtr(eventIdUtf8Bytes), slotIndex, outcome, apiStatus);
            }
        }

        unsafe private static int CCBLoopReportOutcomeSlotJson(IntPtr ccbLoop, string eventId, uint slotIndex, string outcomeJson, IntPtr apiStatus)
        {
            if (eventId == null)
            {
                throw new ArgumentNullException("eventId");
            }

            CheckJsonString(outcomeJson);

            fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
            fixed (byte* outcomeJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(outcomeJson))
            {
                return NativeMethods.CCBLoopReportOutcomeSlotJson(ccbLoop, new IntPtr(eventIdUtf8Bytes), slotIndex, new IntPtr(outcomeJsonUtf8Bytes), apiStatus);
            }
        }

        unsafe private static int CCBLoopReportOutcomeSlotStringIdF(IntPtr ccbLoop, string eventId, string slotId, float outcome, IntPtr apiStatus)
        {
            if (eventId == null)
            {
                throw new ArgumentNullException("eventId");
            }

            if (slotId == null)
            {
                throw new ArgumentNullException("slotId");
            }

            fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
            fixed (byte* slotIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(slotId))
            {
                return NativeMethods.CCBLoopReportOutcomeSlotStringIdF(ccbLoop, new IntPtr(eventIdUtf8Bytes), new IntPtr(slotIdUtf8Bytes), outcome, apiStatus);
            }

        }

        unsafe private static int CCBLoopReportOutcomeSlotStringIdJson(IntPtr ccbLoop, string eventId, string slotId, string outcomeJson, IntPtr apiStatus)
        {
            if (eventId == null)
            {
                throw new ArgumentNullException("eventId");
            }

            if (slotId == null)
            {
                throw new ArgumentNullException("slotId");
            }

            CheckJsonString(outcomeJson);

            fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
            fixed (byte* slotIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(slotId))
            fixed (byte* outcomeJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(outcomeJson))
            {
                return NativeMethods.CCBLoopReportOutcomeSlotStringIdJson(ccbLoop, new IntPtr(eventIdUtf8Bytes), new IntPtr(slotIdUtf8Bytes), new IntPtr(outcomeJsonUtf8Bytes), apiStatus);
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
            int result = NativeMethods.CCBLoopInit(this.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

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

        public bool TryRequestDecision(string contextJson, out DecisionResponse response, ApiStatus apiStatus = null)
        {
            response = new DecisionResponse();
            return this.TryRequestDecision(contextJson, response, apiStatus);
        }

        public bool TryRequestDecision(string contextJson, DecisionResponse response, ApiStatus apiStatus = null)
        {
            int result = CCBLoopRequestDecision(this.DangerousGetHandle(), contextJson, response.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

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

            return this.TryRequestDecision(contextJson, flags, response, apiStatus);
        }

        public bool TryRequestDecision(string contextJson, ActionFlags flags, DecisionResponse response, ApiStatus apiStatus)
        {
            int result = CCBLoopRequestDecisionWithFlags(this.DangerousGetHandle(), contextJson, (uint)flags, response.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());
            GC.KeepAlive(this);
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

        public bool TryRequestMultiSlotDecision(string eventId, string contextJson, out MultiSlotResponse response, ApiStatus apiStatus = null)
        {
            response = new MultiSlotResponse();
            return this.TryRequestMultiSlotDecision(eventId, contextJson, response, apiStatus);
        }

        public bool TryRequestMultiSlotDecision(string eventId, string contextJson, MultiSlotResponse response, ApiStatus apiStatus = null)
        {
            int result = CCBLoopRequestMultiSlotDecision(this.DangerousGetHandle(), eventId, contextJson, response.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

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
            int result = CCBLoopRequestMultiSlotDecisionWithFlags(this.DangerousGetHandle(), eventId, contextJson, (uint)flags, response.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

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
            int result = CCBLoopRequestMultiSlotDecisionWithBaselineAndFlags(this.DangerousGetHandle(), eventId, contextJson, (uint)flags, response.DangerousGetHandle(), baselineActions, apiStatus.ToNativeHandleOrNullptrDangerous());

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
            int result = CCBLoopRequestMultiSlotDecisionDetailed(this.DangerousGetHandle(), eventId, contextJson, response.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

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
            int result = CCBLoopRequestMultiSlotDecisionDetailedWithFlags(this.DangerousGetHandle(), eventId, contextJson, (uint)flags, response.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

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
            int result = CCBLoopRequestMultiSlotDecisionDetailedWithBaselineAndFlags(this.DangerousGetHandle(), eventId, contextJson, (uint)flags, response.DangerousGetHandle(), baselineActions, apiStatus.ToNativeHandleOrNullptrDangerous());
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
            int result = CCBLoopReportActionTaken(this.DangerousGetHandle(), eventId, apiStatus.ToNativeHandleOrNullptrDangerous());

            GC.KeepAlive(apiStatus);
            GC.KeepAlive(this);
            return result == NativeMethods.SuccessStatus;
        }

        public bool TryQueueActionTakenEvent(string primaryId, string secondaryId, ApiStatus apiStatus = null)
        {
            int result = CCBLoopReportActionMultiIdTaken(this.DangerousGetHandle(), primaryId, secondaryId, apiStatus.ToNativeHandleOrNullptrDangerous());

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

        public bool TryQueueOutcomeEvent(string eventId, uint slotIndex, float outcome, ApiStatus apiStatus = null)
        {
            int result = CCBLoopReportOutcomeSlotF(this.DangerousGetHandle(), eventId, slotIndex, outcome, apiStatus.ToNativeHandleOrNullptrDangerous());

            GC.KeepAlive(apiStatus);
            GC.KeepAlive(this);
            return result == NativeMethods.SuccessStatus;
        }

        public void QueueOutcomeEvent(string eventId, uint slotIndex, float outcome)
        {
            using (ApiStatus apiStatus = new ApiStatus())
                if (!this.TryQueueOutcomeEvent(eventId, slotIndex, outcome, apiStatus))
                {
                    throw new RLException(apiStatus);
                }
        }

        public bool TryQueueOutcomeEvent(string eventId, uint slotIndex, string outcomeJson, ApiStatus apiStatus = null)
        {
            int result = CCBLoopReportOutcomeSlotJson(this.DangerousGetHandle(), eventId, slotIndex, outcomeJson, apiStatus.ToNativeHandleOrNullptrDangerous());

            GC.KeepAlive(apiStatus);
            GC.KeepAlive(this);
            return result == NativeMethods.SuccessStatus;
        }

        public void QueueOutcomeEvent(string eventId, uint slotIndex, string outcomeJson)
        {
            using (ApiStatus apiStatus = new ApiStatus())
                if (!this.TryQueueOutcomeEvent(eventId, slotIndex, outcomeJson, apiStatus))
                {
                    throw new RLException(apiStatus);
                }
        }

        public bool TryQueueOutcomeEvent(string eventId, string slotId, float outcome, ApiStatus apiStatus = null)
        {
            int result = CCBLoopReportOutcomeSlotStringIdF(this.DangerousGetHandle(), eventId, slotId, outcome, apiStatus.ToNativeHandleOrNullptrDangerous());

            GC.KeepAlive(apiStatus);
            GC.KeepAlive(this);
            return result == NativeMethods.SuccessStatus;
        }

        public void QueueOutcomeEvent(string secondaryId, string slotId, float outcome)
        {
            using (ApiStatus apiStatus = new ApiStatus())
                if (!this.TryQueueOutcomeEvent(secondaryId, slotId, outcome, apiStatus))
                {
                    throw new RLException(apiStatus);
                }
        }

        public bool TryQueueOutcomeEvent(string eventId, string slotId, string outcomeJson, ApiStatus apiStatus = null)
        {
            int result = CCBLoopReportOutcomeSlotStringIdJson(this.DangerousGetHandle(), eventId, slotId, outcomeJson, apiStatus.ToNativeHandleOrNullptrDangerous());

            GC.KeepAlive(apiStatus);
            GC.KeepAlive(this);
            return result == NativeMethods.SuccessStatus;
        }

        public void QueueOutcomeEvent(string eventId, string slotId, string outcomeJson)
        {
            using (ApiStatus apiStatus = new ApiStatus())
                if (!this.TryQueueOutcomeEvent(eventId, slotId, outcomeJson, apiStatus))
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
            int result = NativeMethods.CCBLoopRefreshModel(this.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

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
                    NativeMethods.CCBLoopSetTrace(this.DangerousGetHandle(), this.managedTraceCallback);
                    GC.KeepAlive(this);
                }

                this.OnTraceLoggerEventInternal += value;
            }
            remove
            {
                this.OnTraceLoggerEventInternal -= value;

                if (this.OnTraceLoggerEventInternal == null)
                {
                    NativeMethods.CCBLoopSetTrace(this.DangerousGetHandle(), null);
                    GC.KeepAlive(this);
                }
            }
        }
    }
}