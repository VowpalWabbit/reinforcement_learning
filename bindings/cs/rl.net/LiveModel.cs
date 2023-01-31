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
            public static extern IntPtr CreateLiveModel(IntPtr config, IntPtr factoryContext);

            [DllImport("rlnetnative")]
            public static extern void DeleteLiveModel(IntPtr liveModel);

            [DllImport("rlnetnative")]
            public static extern int LiveModelInit(IntPtr liveModel, IntPtr apiStatus);

            [DllImport("rlnetnative", EntryPoint = "LiveModelChooseRank")]
            private static extern int LiveModelChooseRankNative(IntPtr liveModel, IntPtr eventId, IntPtr contextJson, int contextJsonSize, IntPtr rankingResponse, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, int, IntPtr, IntPtr, int> LiveModelChooseRankOverride { get; set; }

            public static int LiveModelChooseRank(IntPtr liveModel, IntPtr eventId, IntPtr contextJson, int contextJsonSize, IntPtr rankingResponse, IntPtr apiStatus)
            {
                if (LiveModelChooseRankOverride != null)
                {
                    return LiveModelChooseRankOverride(liveModel, eventId, contextJson, contextJsonSize, rankingResponse, apiStatus);
                }

                return LiveModelChooseRankNative(liveModel, eventId, contextJson, contextJsonSize, rankingResponse, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "LiveModelChooseRankWithFlags")]
            private static extern int LiveModelChooseRankWithFlagsNative(IntPtr liveModel, IntPtr eventId, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr rankingResponse, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, int, uint, IntPtr, IntPtr, int> LiveModelChooseRankWithFlagsOverride { get; set; }

            public static int LiveModelChooseRankWithFlags(IntPtr liveModel, IntPtr eventId, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr rankingResponse, IntPtr apiStatus)
            {
                if (LiveModelChooseRankWithFlagsOverride != null)
                {
                    return LiveModelChooseRankWithFlagsOverride(liveModel, eventId, contextJson, contextJsonSize, flags, rankingResponse, apiStatus);
                }

                return LiveModelChooseRankWithFlagsNative(liveModel, eventId, contextJson, contextJsonSize, flags, rankingResponse, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "LiveModelRequestContinuousAction")]
            private static extern int LiveModelRequestContinuousActionNative(IntPtr liveModel, IntPtr eventId, IntPtr contextJson, int contextJsonSize, IntPtr continuousActionResponse, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, int, IntPtr, IntPtr, int> LiveModelRequestContinuousActionOverride { get; set; }

            public static int LiveModelRequestContinuousAction(IntPtr liveModel, IntPtr eventId, IntPtr contextJson, int contextJsonSize, IntPtr continuousActionResponse, IntPtr apiStatus)
            {
                if (LiveModelRequestContinuousActionOverride != null)
                {
                    return LiveModelRequestContinuousActionOverride(liveModel, eventId, contextJson, contextJsonSize, continuousActionResponse, apiStatus);
                }

                return LiveModelRequestContinuousActionNative(liveModel, eventId, contextJson, contextJsonSize, continuousActionResponse, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "LiveModelRequestContinuousActionWithFlags")]
            private static extern int LiveModelRequestContinuousActionWithFlagsNative(IntPtr liveModel, IntPtr eventId, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr continuousActionResponse, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, int, uint, IntPtr, IntPtr, int> LiveModelRequestContinuousActionWithFlagsOverride { get; set; }

            public static int LiveModelRequestContinuousActionWithFlags(IntPtr liveModel, IntPtr eventId, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr continuousActionResponse, IntPtr apiStatus)
            {
                if (LiveModelRequestContinuousActionWithFlagsOverride != null)
                {
                    return LiveModelRequestContinuousActionWithFlagsOverride(liveModel, eventId, contextJson, contextJsonSize, flags, continuousActionResponse, apiStatus);
                }

                return LiveModelRequestContinuousActionWithFlagsNative(liveModel, eventId, contextJson, contextJsonSize, flags, continuousActionResponse, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "LiveModelRequestDecision")]
            private static extern int LiveModelRequestDecisionNative(IntPtr liveModel, IntPtr contextJson, int contextJsonSize, IntPtr decisionResponse, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, int, IntPtr, IntPtr, int> LiveModelRequestDecisionOverride { get; set; }

            public static int LiveModelRequestDecision(IntPtr liveModel, IntPtr contextJson, int contextJsonSize, IntPtr decisionResponse, IntPtr apiStatus)
            {
                if (LiveModelRequestDecisionOverride != null)
                {
                    return LiveModelRequestDecisionOverride(liveModel, contextJson, contextJsonSize, decisionResponse, apiStatus);
                }

                return LiveModelRequestDecisionNative(liveModel, contextJson, contextJsonSize, decisionResponse, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "LiveModelRequestDecisionWithFlags")]
            private static extern int LiveModelRequestDecisionWithFlagsNative(IntPtr liveModel, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr decisionResponse, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, int, uint, IntPtr, IntPtr, int> LiveModelRequestDecisionWithFlagsOverride { get; set; }

            public static int LiveModelRequestDecisionWithFlags(IntPtr liveModel, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr decisionResponse, IntPtr apiStatus)
            {
                if (LiveModelRequestDecisionWithFlagsOverride != null)
                {
                    return LiveModelRequestDecisionWithFlagsOverride(liveModel, contextJson, contextJsonSize, flags, decisionResponse, apiStatus);
                }

                return LiveModelRequestDecisionWithFlagsNative(liveModel, contextJson, contextJsonSize, flags, decisionResponse, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "LiveModelRequestMultiSlotDecision")]
            private static extern int LiveModelRequestMultiSlotDecisionNative(IntPtr liveModel, IntPtr eventId, IntPtr contextJson, int contextJsonSize, IntPtr multiSlotResponse, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, int, IntPtr, IntPtr, int> LiveModelRequestMultiSlotDecisionOverride { get; set; }

            public static int LiveModelRequestMultiSlotDecision(IntPtr liveModel, IntPtr eventId, IntPtr contextJson, int contextJsonSize, IntPtr multiSlotResponse, IntPtr apiStatus)
            {
                if (LiveModelRequestMultiSlotDecisionOverride != null)
                {
                    return LiveModelRequestMultiSlotDecisionOverride(liveModel, eventId, contextJson, contextJsonSize, multiSlotResponse, apiStatus);
                }

                return LiveModelRequestMultiSlotDecisionNative(liveModel, eventId, contextJson, contextJsonSize, multiSlotResponse, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "LiveModelRequestMultiSlotDecisionWithFlags")]
            private static extern int LiveModelRequestMultiSlotDecisionWithFlagsNative(IntPtr liveModel, IntPtr eventId, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr multiSlotResponse, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, int, uint, IntPtr, IntPtr, int> LiveModelRequestMultiSlotDecisionWithFlagsOverride { get; set; }

            public static int LiveModelRequestMultiSlotDecisionWithFlags(IntPtr liveModel, IntPtr eventId, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr multiSlotResponse, IntPtr apiStatus)
            {
                if (LiveModelRequestMultiSlotDecisionWithFlagsOverride != null)
                {
                    return LiveModelRequestMultiSlotDecisionWithFlagsOverride(liveModel, eventId, contextJson, contextJsonSize, flags, multiSlotResponse, apiStatus);
                }

                return LiveModelRequestMultiSlotDecisionWithFlagsNative(liveModel, eventId, contextJson, contextJsonSize, flags, multiSlotResponse, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "LiveModelRequestMultiSlotDecisionWithBaselineAndFlags")]
            private static extern int LiveModelRequestMultiSlotDecisionWithBaselineAndFlagsNative(IntPtr liveModel, IntPtr eventId, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr multiSlotResponse, IntPtr baselineActions, IntPtr baselineActionsSize, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, int, uint, IntPtr, IntPtr, IntPtr, IntPtr, int> LiveModelRequestMultiSlotDecisionWithBaselineAndFlagsOverride { get; set; }

            public static int LiveModelRequestMultiSlotDecisionWithBaselineAndFlags(IntPtr liveModel, IntPtr eventId, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr multiSlotResponse, IntPtr baselineActions, IntPtr baselineActionsSize, IntPtr apiStatus)
            {
                if (LiveModelRequestMultiSlotDecisionWithBaselineAndFlagsOverride != null)
                {
                    return LiveModelRequestMultiSlotDecisionWithBaselineAndFlagsOverride(liveModel, eventId, contextJson, contextJsonSize, flags, multiSlotResponse, baselineActions, baselineActionsSize, apiStatus);
                }

                return LiveModelRequestMultiSlotDecisionWithBaselineAndFlagsNative(liveModel, eventId, contextJson, contextJsonSize, flags, multiSlotResponse, baselineActions, baselineActionsSize, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "LiveModelRequestMultiSlotDecisionDetailed")]
            private static extern int LiveModelRequestMultiSlotDecisionDetailedNative(IntPtr liveModel, IntPtr eventId, IntPtr contextJson, int contextJsonSize, IntPtr multiSlotResponseDetailed, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, int, IntPtr, IntPtr, int> LiveModelRequestMultiSlotDecisionDetailedOverride { get; set; }

            public static int LiveModelRequestMultiSlotDecisionDetailed(IntPtr liveModel, IntPtr eventId, IntPtr contextJson, int contextJsonSize, IntPtr multiSlotResponseDetailed, IntPtr apiStatus)
            {
                if (LiveModelRequestMultiSlotDecisionDetailedOverride != null)
                {
                    return LiveModelRequestMultiSlotDecisionDetailedOverride(liveModel, eventId, contextJson, contextJsonSize, multiSlotResponseDetailed, apiStatus);
                }

                return LiveModelRequestMultiSlotDecisionDetailedNative(liveModel, eventId, contextJson, contextJsonSize, multiSlotResponseDetailed, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "LiveModelRequestMultiSlotDecisionDetailedWithFlags")]
            private static extern int LiveModelRequestMultiSlotDecisionDetailedWithFlagsNative(IntPtr liveModel, IntPtr eventId, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr multiSlotResponseDetailed, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, int, uint, IntPtr, IntPtr, int> LiveModelRequestMultiSlotDecisionDetailedWithFlagsOverride { get; set; }

            public static int LiveModelRequestMultiSlotDecisionDetailedWithFlags(IntPtr liveModel, IntPtr eventId, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr multiSlotResponseDetailed, IntPtr apiStatus)
            {
                if (LiveModelRequestMultiSlotDecisionDetailedWithFlagsOverride != null)
                {
                    return LiveModelRequestMultiSlotDecisionDetailedWithFlagsOverride(liveModel, eventId, contextJson, contextJsonSize, flags, multiSlotResponseDetailed, apiStatus);
                }

                return LiveModelRequestMultiSlotDecisionDetailedWithFlagsNative(liveModel, eventId, contextJson, contextJsonSize, flags, multiSlotResponseDetailed, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "LiveModelRequestMultiSlotDecisionDetailedWithBaselineAndFlags")]
            private static extern int LiveModelRequestMultiSlotDecisionDetailedWithBaselineAndFlagsNative(IntPtr liveModel, IntPtr eventId, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr multiSlotResponseDetailed, IntPtr baselineActions, IntPtr baselineActionsSize, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, int, uint, IntPtr, IntPtr, IntPtr, IntPtr, int> LiveModelRequestMultiSlotDecisionDetailedWithBaselineAndFlagsOverride { get; set; }

            public static int LiveModelRequestMultiSlotDecisionDetailedWithBaselineAndFlags(IntPtr liveModel, IntPtr eventId, IntPtr contextJson, int contextJsonSize, uint flags, IntPtr multiSlotResponseDetailed, IntPtr baselineActions, IntPtr baselineActionsSize, IntPtr apiStatus)
            {
                if (LiveModelRequestMultiSlotDecisionDetailedWithBaselineAndFlagsOverride != null)
                {
                    return LiveModelRequestMultiSlotDecisionDetailedWithBaselineAndFlagsOverride(liveModel, eventId, contextJson, contextJsonSize, flags, multiSlotResponseDetailed, baselineActions, baselineActionsSize, apiStatus);
                }

                return LiveModelRequestMultiSlotDecisionDetailedWithBaselineAndFlagsNative(liveModel, eventId, contextJson, contextJsonSize, flags, multiSlotResponseDetailed, baselineActions, baselineActionsSize, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "LiveModelRequestEpisodicDecisionWithFlags")]
            private static extern int LiveModelRequestEpisodicDecisionWithFlagsNative(IntPtr liveModel, IntPtr eventId, IntPtr previousEventId, IntPtr contextJson, uint flags, IntPtr rankingResponse, IntPtr episodes, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, IntPtr, uint, IntPtr, IntPtr, IntPtr, int> LiveModelRequestEpisodicDecisionWithFlagsOverride { get; set; }

            public static int LiveModelRequestEpisodicDecisionWithFlags(IntPtr liveModel, IntPtr eventId, IntPtr previousEventId, IntPtr contextJson, uint flags, IntPtr rankingResponse, IntPtr episodes, IntPtr apiStatus)
            {
                if (LiveModelRequestEpisodicDecisionWithFlagsOverride != null)
                {
                    return LiveModelRequestEpisodicDecisionWithFlagsOverride(liveModel, eventId, previousEventId, contextJson, flags, rankingResponse, episodes, apiStatus);
                }

                return LiveModelRequestEpisodicDecisionWithFlagsNative(liveModel, eventId, previousEventId, contextJson, flags, rankingResponse, episodes, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "LiveModelReportActionTaken")]
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

            [DllImport("rlnetnative", EntryPoint = "LiveModelReportOutcomeF")]
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

            [DllImport("rlnetnative", EntryPoint = "LiveModelReportOutcomeJson")]
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

            [DllImport("rlnetnative", EntryPoint = "LiveModelReportOutcomeSlotF")]
            private static extern int LiveModelReportOutcomeSlotFNative(IntPtr liveModel, IntPtr eventId, uint slotIndex, float outcome, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, uint, float, IntPtr, int> LiveModelReportOutcomeSlotFOverride { get; set; }

            public static int LiveModelReportOutcomeSlotF(IntPtr liveModel, IntPtr eventId, uint slotIndex, float outcome, IntPtr apiStatus)
            {
                if (LiveModelReportOutcomeSlotFOverride != null)
                {
                    return LiveModelReportOutcomeSlotFOverride(liveModel, eventId, slotIndex, outcome, apiStatus);
                }

                return LiveModelReportOutcomeSlotFNative(liveModel, eventId, slotIndex, outcome, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "LiveModelReportOutcomeSlotJson")]
            private static extern int LiveModelReportOutcomeSlotJsonNative(IntPtr liveModel, IntPtr eventId, uint slotIndex, IntPtr outcomeJson, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, uint, IntPtr, IntPtr, int> LiveModelReportOutcomeSlotJsonOverride { get; set; }

            public static int LiveModelReportOutcomeSlotJson(IntPtr liveModel, IntPtr eventId, uint slotIndex, IntPtr outcomeJson, IntPtr apiStatus)
            {
                if (LiveModelReportOutcomeSlotJsonOverride != null)
                {
                    return LiveModelReportOutcomeSlotJsonOverride(liveModel, eventId, slotIndex, outcomeJson, apiStatus);
                }

                return LiveModelReportOutcomeSlotJsonNative(liveModel, eventId, slotIndex, outcomeJson, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "LiveModelReportOutcomeSlotStringIdF")]
            private static extern int LiveModelReportOutcomeSlotStringIdFNative(IntPtr liveModel, IntPtr eventId, IntPtr slotId, float outcome, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, float, IntPtr, int> LiveModelReportOutcomeSlotStringIdFOverride { get; set; }

            public static int LiveModelReportOutcomeSlotStringIdF(IntPtr liveModel, IntPtr eventId, IntPtr slotId, float outcome, IntPtr apiStatus)
            {
                if (LiveModelReportOutcomeSlotStringIdFOverride != null)
                {
                    return LiveModelReportOutcomeSlotStringIdFOverride(liveModel, eventId, slotId, outcome, apiStatus);
                }

                return LiveModelReportOutcomeSlotStringIdFNative(liveModel, eventId, slotId, outcome, apiStatus);
            }

            [DllImport("rlnetnative", EntryPoint = "LiveModelReportOutcomeSlotStringIdJson")]
            private static extern int LiveModelReportOutcomeSlotStringIdJsonNative(IntPtr liveModel, IntPtr eventId, IntPtr slotId, IntPtr outcomeJson, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, IntPtr, IntPtr, int> LiveModelReportOutcomeSlotStringIdJsonOverride { get; set; }

            public static int LiveModelReportOutcomeSlotStringIdJson(IntPtr liveModel, IntPtr eventId, IntPtr slotId, IntPtr outcomeJson, IntPtr apiStatus)
            {
                if (LiveModelReportOutcomeSlotStringIdJsonOverride != null)
                {
                    return LiveModelReportOutcomeSlotStringIdJsonOverride(liveModel, eventId, slotId, outcomeJson, apiStatus);
                }

                return LiveModelReportOutcomeSlotStringIdJsonNative(liveModel, eventId, slotId, outcomeJson, apiStatus);
            }

            [DllImport("rlnetnative")]
            public static extern int LiveModelRefreshModel(IntPtr liveModel, IntPtr apiStatus);

            public delegate void managed_background_error_callback_t(IntPtr apiStatus);

            [DllImport("rlnetnative")]
            public static extern void LiveModelSetCallback(IntPtr liveModel, [MarshalAs(UnmanagedType.FunctionPtr)] managed_background_error_callback_t callback = null);

            public delegate void managed_trace_callback_t(int logLevel, IntPtr msgUtf8Ptr);

            [DllImport("rlnetnative")]
            public static extern void LiveModelSetTrace(IntPtr liveModel, [MarshalAs(UnmanagedType.FunctionPtr)] managed_trace_callback_t callback = null);
        }
    }

    public sealed class LiveModel : NativeObject<LiveModel>
    {
        private readonly NativeMethods.managed_background_error_callback_t managedErrorCallback;
        private readonly NativeMethods.managed_trace_callback_t managedTraceCallback;

        private static New<LiveModel> BindConstructorArguments(Configuration config, FactoryContext factoryContext)
        {
            return new New<LiveModel>(() =>
            {
                factoryContext = factoryContext ?? new FactoryContext();
                IntPtr result = NativeMethods.CreateLiveModel(config.DangerousGetHandle(), factoryContext.DangerousGetHandle());

                // These references do not live on the heap in this delegate, and could disappear during the invocation
                // of CreateLiveModel. Thus, we need to ensure GC knows not to release them until after that call
                // returns.
                GC.KeepAlive(config);
                GC.KeepAlive(factoryContext);

                return result;
            });

        }

        public LiveModel(Configuration config) : this(config, null)
        { }

        public LiveModel(Configuration config, FactoryContext factoryContext) : base(BindConstructorArguments(config, factoryContext), new Delete<LiveModel>(NativeMethods.DeleteLiveModel))
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
                int contextJsonSize = NativeMethods.StringEncoding.GetByteCount(contextJson);
                IntPtr contextJsonUtf8Ptr = new IntPtr(contextJsonUtf8Bytes);

                // It is important to pass null on faithfully here, because we rely on this to switch between auto-generate
                // eventId and use supplied eventId at the rl.net.native layer.
                if (eventId == null)
                {
                    return NativeMethods.LiveModelChooseRank(liveModel, IntPtr.Zero, contextJsonUtf8Ptr, contextJsonSize, rankingResponse, apiStatus);
                }

                fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
                {
                    return NativeMethods.LiveModelChooseRank(liveModel, new IntPtr(eventIdUtf8Bytes), contextJsonUtf8Ptr, contextJsonSize, rankingResponse, apiStatus);
                }
            }
        }

        // TODO: Should we reduce the rl.net.native interface to only have one of these?
        unsafe private static int LiveModelChooseRankWithFlags(IntPtr liveModel, string eventId, string contextJson, uint flags, IntPtr rankingResponse, IntPtr apiStatus)
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
                    return NativeMethods.LiveModelChooseRankWithFlags(liveModel, IntPtr.Zero, contextJsonUtf8Ptr, contextJsonSize, flags, rankingResponse, apiStatus);
                }

                fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
                {
                    return NativeMethods.LiveModelChooseRankWithFlags(liveModel, new IntPtr(eventIdUtf8Bytes), contextJsonUtf8Ptr, contextJsonSize, flags, rankingResponse, apiStatus);
                }
            }
        }

        unsafe private static int LiveModelRequestContinuousAction(IntPtr liveModel, string eventId, string contextJson, IntPtr continuousActionResponse, IntPtr apiStatus)
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
                    return NativeMethods.LiveModelRequestContinuousAction(liveModel, IntPtr.Zero, contextJsonUtf8Ptr, contextJsonSize, continuousActionResponse, apiStatus);
                }

                fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
                {
                    return NativeMethods.LiveModelRequestContinuousAction(liveModel, new IntPtr(eventIdUtf8Bytes), contextJsonUtf8Ptr, contextJsonSize, continuousActionResponse, apiStatus);
                }
            }
        }

        unsafe private static int LiveModelRequestContinuousActionWithFlags(IntPtr liveModel, string eventId, string contextJson, uint flags, IntPtr continuousActionResponse, IntPtr apiStatus)
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
                    return NativeMethods.LiveModelRequestContinuousActionWithFlags(liveModel, IntPtr.Zero, contextJsonUtf8Ptr, contextJsonSize, flags, continuousActionResponse, apiStatus);
                }

                fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
                {
                    return NativeMethods.LiveModelRequestContinuousActionWithFlags(liveModel, new IntPtr(eventIdUtf8Bytes), contextJsonUtf8Ptr, contextJsonSize, flags, continuousActionResponse, apiStatus);
                }
            }
        }

        unsafe private static int LiveModelRequestDecision(IntPtr liveModel, string contextJson, IntPtr decisionResponse, IntPtr apiStatus)
        {
            CheckJsonString(contextJson);

            fixed (byte* contextJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(contextJson))
            {
                int contextJsonSize = NativeMethods.StringEncoding.GetByteCount(contextJson);
                return NativeMethods.LiveModelRequestDecision(liveModel, new IntPtr(contextJsonUtf8Bytes), contextJsonSize, decisionResponse, apiStatus);
            }
        }

        unsafe private static int LiveModelRequestDecisionWithFlags(IntPtr liveModel, string contextJson, uint flags, IntPtr decisionResponse, IntPtr apiStatus)
        {
            CheckJsonString(contextJson);

            fixed (byte* contextJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(contextJson))
            {
                int contextJsonSize = NativeMethods.StringEncoding.GetByteCount(contextJson);
                return NativeMethods.LiveModelRequestDecisionWithFlags(liveModel, new IntPtr(contextJsonUtf8Bytes), contextJsonSize, flags, decisionResponse, apiStatus);
            }
        }

        unsafe private static int LiveModelRequestMultiSlotDecision(IntPtr liveModel, string eventId, string contextJson, IntPtr multiSlotResponse, IntPtr apiStatus)
        {
            CheckJsonString(contextJson);

            fixed (byte* contextJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(contextJson))
            {
                int contextJsonSize = NativeMethods.StringEncoding.GetByteCount(contextJson);
                if (eventId == null)
                {
                    return NativeMethods.LiveModelRequestMultiSlotDecision(liveModel, IntPtr.Zero, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, multiSlotResponse, apiStatus);
                }

                fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
                {
                    return NativeMethods.LiveModelRequestMultiSlotDecision(liveModel, (IntPtr)eventIdUtf8Bytes, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, multiSlotResponse, apiStatus);
                }
            }
        }

        unsafe private static int LiveModelRequestMultiSlotDecisionWithFlags(IntPtr liveModel, string eventId, string contextJson, uint flags, IntPtr multiSlotResponse, IntPtr apiStatus)
        {
            CheckJsonString(contextJson);

            fixed (byte* contextJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(contextJson))
            {
                int contextJsonSize = NativeMethods.StringEncoding.GetByteCount(contextJson);
                if (eventId == null)
                {
                    return NativeMethods.LiveModelRequestMultiSlotDecisionWithFlags(liveModel, IntPtr.Zero, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, flags, multiSlotResponse, apiStatus);
                }

                fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
                {
                    return NativeMethods.LiveModelRequestMultiSlotDecisionWithFlags(liveModel, (IntPtr)eventIdUtf8Bytes, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, flags, multiSlotResponse, apiStatus);
                }
            }
        }

        unsafe private static int LiveModelRequestMultiSlotDecisionWithBaselineAndFlags(IntPtr liveModel, string eventId, string contextJson, uint flags, IntPtr multiSlotResponse, int[] baselineActions, IntPtr apiStatus)
        {
            CheckJsonString(contextJson);

            fixed (byte* contextJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(contextJson))
            fixed (int* baselineActionsFixed = baselineActions)
            {
                int contextJsonSize = NativeMethods.StringEncoding.GetByteCount(contextJson);
                if (eventId == null)
                {
                    return NativeMethods.LiveModelRequestMultiSlotDecisionWithBaselineAndFlags(liveModel, IntPtr.Zero, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, flags, multiSlotResponse, (IntPtr)baselineActionsFixed, (IntPtr)baselineActions.Length, apiStatus);
                }

                fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
                {
                    return NativeMethods.LiveModelRequestMultiSlotDecisionWithBaselineAndFlags(liveModel, (IntPtr)eventIdUtf8Bytes, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, flags, multiSlotResponse, (IntPtr)baselineActionsFixed, (IntPtr)baselineActions.Length, apiStatus);
                }
            }
        }

        unsafe private static int LiveModelRequestMultiSlotDecisionDetailed(IntPtr liveModel, string eventId, string contextJson, IntPtr multiSlotResponseDetailed, IntPtr apiStatus)
        {
            CheckJsonString(contextJson);

            fixed (byte* contextJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(contextJson))
            {
                int contextJsonSize = NativeMethods.StringEncoding.GetByteCount(contextJson);
                if (eventId == null)
                {
                    return NativeMethods.LiveModelRequestMultiSlotDecisionDetailed(liveModel, IntPtr.Zero, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, multiSlotResponseDetailed, apiStatus);
                }

                fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
                {
                    return NativeMethods.LiveModelRequestMultiSlotDecisionDetailed(liveModel, (IntPtr)eventIdUtf8Bytes, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, multiSlotResponseDetailed, apiStatus);
                }
            }
        }

        unsafe private static int LiveModelRequestMultiSlotDecisionDetailedWithFlags(IntPtr liveModel, string eventId, string contextJson, uint flags, IntPtr multiSlotResponseDetailed, IntPtr apiStatus)
        {
            CheckJsonString(contextJson);

            fixed (byte* contextJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(contextJson))
            {
                int contextJsonSize = NativeMethods.StringEncoding.GetByteCount(contextJson);
                if (eventId == null)
                {
                    return NativeMethods.LiveModelRequestMultiSlotDecisionDetailedWithFlags(liveModel, IntPtr.Zero, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, flags, multiSlotResponseDetailed, apiStatus);
                }

                fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
                {
                    return NativeMethods.LiveModelRequestMultiSlotDecisionDetailedWithFlags(liveModel, (IntPtr)eventIdUtf8Bytes, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, flags, multiSlotResponseDetailed, apiStatus);
                }
            }
        }

        unsafe private static int LiveModelRequestMultiSlotDecisionDetailedWithBaselineAndFlags(IntPtr liveModel, string eventId, string contextJson, uint flags, IntPtr multiSlotResponseDetailed, int[] baselineActions, IntPtr apiStatus)
        {
            CheckJsonString(contextJson);

            fixed (byte* contextJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(contextJson))
            fixed (int* baselineActionsFixed = baselineActions)
            {
                int contextJsonSize = NativeMethods.StringEncoding.GetByteCount(contextJson);
                if (eventId == null)
                {
                    return NativeMethods.LiveModelRequestMultiSlotDecisionDetailedWithBaselineAndFlags(liveModel, IntPtr.Zero, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, flags, multiSlotResponseDetailed, (IntPtr)baselineActionsFixed, (IntPtr)baselineActions.Length, apiStatus);
                }

                fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
                {
                    return NativeMethods.LiveModelRequestMultiSlotDecisionDetailedWithBaselineAndFlags(liveModel, (IntPtr)eventIdUtf8Bytes, (IntPtr)contextJsonUtf8Bytes, contextJsonSize, flags, multiSlotResponseDetailed, (IntPtr)baselineActionsFixed, (IntPtr)baselineActions.Length, apiStatus);
                }
            }
        }

        unsafe private static int LiveModelRequestEpisodicDecisionWithFlags(IntPtr liveModel, string eventId, string previousEventId, string contextJson, uint flags, IntPtr rankingResponse, IntPtr episodeState, IntPtr apiStatus)
        {
            CheckJsonString(contextJson);
            if (string.IsNullOrEmpty(eventId))
            {
                throw new ArgumentException("eventId cannot be null or empty", "eventId");
            }

            fixed (byte* contextJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(contextJson))
            fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
            {
                if (previousEventId == null)
                {
                    return NativeMethods.LiveModelRequestEpisodicDecisionWithFlags(liveModel, new IntPtr(eventIdUtf8Bytes), IntPtr.Zero, new IntPtr(contextJsonUtf8Bytes), flags, rankingResponse, episodeState, apiStatus);
                }
                else
                {
                    fixed (byte* previousEventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(previousEventId))
                    {
                        return NativeMethods.LiveModelRequestEpisodicDecisionWithFlags(liveModel, new IntPtr(eventIdUtf8Bytes), new IntPtr(previousEventIdUtf8Bytes), new IntPtr(contextJsonUtf8Bytes), flags, rankingResponse, episodeState, apiStatus);
                    }
                }
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

        unsafe private static int LiveModelReportOutcomeSlotF(IntPtr liveModel, string eventId, uint slotIndex, float outcome, IntPtr apiStatus)
        {
            if (eventId == null)
            {
                throw new ArgumentNullException("eventId");
            }

            fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
            {
                return NativeMethods.LiveModelReportOutcomeSlotF(liveModel, new IntPtr(eventIdUtf8Bytes), slotIndex, outcome, apiStatus);
            }
        }

        unsafe private static int LiveModelReportOutcomeSlotJson(IntPtr liveModel, string eventId, uint slotIndex, string outcomeJson, IntPtr apiStatus)
        {
            if (eventId == null)
            {
                throw new ArgumentNullException("eventId");
            }

            CheckJsonString(outcomeJson);

            fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
            fixed (byte* outcomeJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(outcomeJson))
            {
                return NativeMethods.LiveModelReportOutcomeSlotJson(liveModel, new IntPtr(eventIdUtf8Bytes), slotIndex, new IntPtr(outcomeJsonUtf8Bytes), apiStatus);
            }
        }

        unsafe private static int LiveModelReportOutcomeSlotStringIdF(IntPtr liveModel, string eventId, string slotId, float outcome, IntPtr apiStatus)
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
                return NativeMethods.LiveModelReportOutcomeSlotStringIdF(liveModel, new IntPtr(eventIdUtf8Bytes), new IntPtr(slotIdUtf8Bytes), outcome, apiStatus);
            }

        }

        unsafe private static int LiveModelReportOutcomeSlotStringIdJson(IntPtr liveModel, string eventId, string slotId, string outcomeJson, IntPtr apiStatus)
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
                return NativeMethods.LiveModelReportOutcomeSlotStringIdJson(liveModel, new IntPtr(eventIdUtf8Bytes), new IntPtr(slotIdUtf8Bytes), new IntPtr(outcomeJsonUtf8Bytes), apiStatus);
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

        public bool TryRequestContinuousAction(string eventId, string contextJson, out ContinuousActionResponse response, ApiStatus apiStatus = null)
        {
            response = new ContinuousActionResponse();
            return this.TryRequestContinuousAction(eventId, contextJson, response, apiStatus);
        }

        public bool TryRequestContinuousAction(string eventId, string contextJson, ContinuousActionResponse response, ApiStatus apiStatus = null)
        {
            int result = LiveModelRequestContinuousAction(this.DangerousGetHandle(), eventId, contextJson, response.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

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
            int result = LiveModelRequestContinuousActionWithFlags(this.DangerousGetHandle(), eventId, contextJson, (uint)flags, response.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

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

            return this.TryRequestDecision(contextJson, flags, response, apiStatus);
        }

        public bool TryRequestDecision(string contextJson, ActionFlags flags, DecisionResponse response, ApiStatus apiStatus)
        {
            int result = LiveModelRequestDecisionWithFlags(this.DangerousGetHandle(), contextJson, (uint)flags, response.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());
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
            int result = LiveModelRequestMultiSlotDecision(this.DangerousGetHandle(), eventId, contextJson, response.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

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
            int result = LiveModelRequestMultiSlotDecisionWithFlags(this.DangerousGetHandle(), eventId, contextJson, (uint)flags, response.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

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
            int result = LiveModelRequestMultiSlotDecisionWithBaselineAndFlags(this.DangerousGetHandle(), eventId, contextJson, (uint)flags, response.DangerousGetHandle(), baselineActions, apiStatus.ToNativeHandleOrNullptrDangerous());

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
            int result = LiveModelRequestMultiSlotDecisionDetailed(this.DangerousGetHandle(), eventId, contextJson, response.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

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
            int result = LiveModelRequestMultiSlotDecisionDetailedWithFlags(this.DangerousGetHandle(), eventId, contextJson, (uint)flags, response.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

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
            int result = LiveModelRequestMultiSlotDecisionDetailedWithBaselineAndFlags(this.DangerousGetHandle(), eventId, contextJson, (uint)flags, response.DangerousGetHandle(), baselineActions, apiStatus.ToNativeHandleOrNullptrDangerous());
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

        public bool TryRequestEpisodicDecision(string eventId, string previousEventId, string contextJson, ActionFlags flags, EpisodeState states, RankingResponse resp, ApiStatus apiStatus)
        {
            int result = LiveModelRequestEpisodicDecisionWithFlags(this.DangerousGetHandle(), eventId, previousEventId, contextJson, (uint)flags, states.DangerousGetHandle(), resp.DangerousGetHandle(), apiStatus.DangerousGetHandle());
            GC.KeepAlive(this);
            return result == NativeMethods.SuccessStatus;
        }

        public RankingResponse RequestEpisodicDecision(string eventId, string previousEventId, string contextJson, ActionFlags flags, EpisodeState states)
        {
            RankingResponse resp = new RankingResponse();

            using (ApiStatus apiStatus = new ApiStatus())
                if (!this.TryRequestEpisodicDecision(eventId, previousEventId, contextJson, flags, states, resp, apiStatus))
                {
                    throw new RLException(apiStatus);
                }
            return resp;
        }

        [Obsolete("Use TryQueueActionTakenEvent instead.")]
        public bool TryReportActionTaken(string eventId, ApiStatus apiStatus = null)
        => this.TryQueueActionTakenEvent(eventId, apiStatus);

        public bool TryQueueActionTakenEvent(string eventId, ApiStatus apiStatus = null)
        {
            int result = LiveModelReportActionTaken(this.DangerousGetHandle(), eventId, apiStatus.ToNativeHandleOrNullptrDangerous());

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

        [Obsolete("Use TryQueueOutcomeEvent instead.")]
        public bool TryReportOutcome(string eventId, float outcome, ApiStatus apiStatus = null)
            => this.TryQueueOutcomeEvent(eventId, outcome, apiStatus);

        public bool TryQueueOutcomeEvent(string eventId, float outcome, ApiStatus apiStatus = null)
        {
            int result = LiveModelReportOutcomeF(this.DangerousGetHandle(), eventId, outcome, apiStatus.ToNativeHandleOrNullptrDangerous());

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
            int result = LiveModelReportOutcomeJson(this.DangerousGetHandle(), eventId, outcomeJson, apiStatus.ToNativeHandleOrNullptrDangerous());

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

        public bool TryQueueOutcomeEvent(string eventId, uint slotIndex, float outcome, ApiStatus apiStatus = null)
        {
            int result = LiveModelReportOutcomeSlotF(this.DangerousGetHandle(), eventId, slotIndex, outcome, apiStatus.ToNativeHandleOrNullptrDangerous());

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
            int result = LiveModelReportOutcomeSlotJson(this.DangerousGetHandle(), eventId, slotIndex, outcomeJson, apiStatus.ToNativeHandleOrNullptrDangerous());

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
            int result = LiveModelReportOutcomeSlotStringIdF(this.DangerousGetHandle(), eventId, slotId, outcome, apiStatus.ToNativeHandleOrNullptrDangerous());

            GC.KeepAlive(apiStatus);
            GC.KeepAlive(this);
            return result == NativeMethods.SuccessStatus;
        }

        public void QueueOutcomeEvent(string eventId, string slotId, float outcome)
        {
            using (ApiStatus apiStatus = new ApiStatus())
                if (!this.TryQueueOutcomeEvent(eventId, slotId, outcome, apiStatus))
                {
                    throw new RLException(apiStatus);
                }
        }

        public bool TryQueueOutcomeEvent(string eventId, string slotId, string outcomeJson, ApiStatus apiStatus = null)
        {
            int result = LiveModelReportOutcomeSlotStringIdJson(this.DangerousGetHandle(), eventId, slotId, outcomeJson, apiStatus.ToNativeHandleOrNullptrDangerous());

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
            int result = NativeMethods.LiveModelRefreshModel(this.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

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