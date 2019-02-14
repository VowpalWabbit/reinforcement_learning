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
            this.managedTraceCallback = new managed_trace_callback_t(this.SendTrace);
        }

        private void WrapStatusAndRaiseBackgroundError(IntPtr apiStatusHandle)
        {
            ApiStatus status = new ApiStatus(apiStatusHandle);

            this.BackgroundErrorInternal?.Invoke(this, status);
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

        public bool TryChooseRank(string actionId, string contextJson, out RankingResponse response, ApiStatus apiStatus = null)
        {
            response = new RankingResponse();
            return this.TryChooseRank(actionId, contextJson, response, apiStatus);
        }

        public bool TryChooseRank(string actionId, string contextJson, RankingResponse response, ApiStatus apiStatus = null)
        {
            int result = LiveModelChooseRank(this.NativeHandle, actionId, contextJson, response.NativeHandle, apiStatus.ToNativeHandleOrNullptr());
            return result == NativeMethods.SuccessStatus;
        }

        public bool TryChooseRank(string actionId, string contextJson, ActionFlags flags, out RankingResponse response, ApiStatus apiStatus = null)
        {
            response = new RankingResponse();
            return this.TryChooseRank(actionId, contextJson, flags, response, apiStatus);
        }

        public bool TryChooseRank(string actionId, string contextJson, ActionFlags flags, RankingResponse response, ApiStatus apiStatus = null)
        {
            int result = LiveModelChooseRankWithFlags(this.NativeHandle, actionId, contextJson, (uint)flags, response.NativeHandle, apiStatus.ToNativeHandleOrNullptr());
            return result == NativeMethods.SuccessStatus;
        }

        public bool TryReportActionTaken(string actionId, ApiStatus apiStatus = null)
        {
            int result = LiveModelReportActionTaken(this.NativeHandle, actionId, apiStatus.ToNativeHandleOrNullptr());
            return result == NativeMethods.SuccessStatus;
        }

        public bool TryReportOutcome(string actionId, float outcome, ApiStatus apiStatus = null)
        {
            int result = LiveModelReportOutcomeF(this.NativeHandle, actionId, outcome, apiStatus.ToNativeHandleOrNullptr());
            return result == NativeMethods.SuccessStatus;
        }

        public bool TryReportOutcome(string actionId, string outcomeJson, ApiStatus apiStatus = null)
        {
            int result = LiveModelReportOutcomeJson(this.NativeHandle, actionId, outcomeJson, apiStatus.ToNativeHandleOrNullptr());
            return result == NativeMethods.SuccessStatus;
        }

        private event EventHandler<ApiStatus> BackgroundErrorInternal;

        // TODO: This class need a pass to ensure thread-safety (or explicit declaration of non-thread-safe)
        public event EventHandler<ApiStatus> BackgroundError
        {
            add
            {
                if (this.BackgroundErrorInternal == null)
                {
                    LiveModelSetCallback(this.NativeHandle, this.managedErrorCallback);
                }

                this.BackgroundErrorInternal += value;
            }
            remove
            {
                this.BackgroundErrorInternal -= value;

                if (this.BackgroundErrorInternal == null)
                {
                    LiveModelSetCallback(this.NativeHandle, null);
                }
            }
        }

        private event EventHandler<TraceLogEventArgs> OnTraceLoggerEventInternal;
        // TODO: This class need a pass to ensure thread-safety (or explicit declaration of non-thread-safe)
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