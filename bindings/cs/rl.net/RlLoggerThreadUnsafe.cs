using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using Rl.Net.Native;

namespace Rl.Net
{
    namespace Native
    {
        // The publics in this class are just a verbose, but jittably-efficient way of enabling overriding a native invocation
        internal static partial class NativeMethods
        {
            [DllImport("rl.net.native.dll")]
            public static extern IntPtr CreateRlLogger(IntPtr config);

            [DllImport("rl.net.native.dll")]
            public static extern void DeleteRlLogger(IntPtr logger);

            [DllImport("rl.net.native.dll")]
            public static extern int RlLoggerInit(IntPtr logger, IntPtr apiStatus);

            [DllImport("rl.net.native.dll", EntryPoint = "RlLoggerLogCbInteraction")]
            private static extern int RlLoggerLogCbInteractionNative(IntPtr logger, IntPtr contextJson, IntPtr rankingResponse, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, IntPtr, int> RlLoggerLogCbInteractionOverride { get; set; }

            [DllImport("rl.net.native.dll", EntryPoint = "RlLoggerLogCcbInteraction")]
            private static extern int RlLoggerLogCcbInteractionNative(IntPtr logger, IntPtr contextJson, IntPtr decisionResponse, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, IntPtr, int> RlLoggerLogCcbInteractionOverride { get; set; }

            [DllImport("rl.net.native.dll", EntryPoint = "RlLoggerLogOutcomeF")]
            private static extern int RlLoggerLogOutcomeFNative(IntPtr logger, IntPtr eventId, float outcome, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, float, IntPtr, int> RlLoggerLogOutcomeFOverride { get; set; }

            [DllImport("rl.net.native.dll", EntryPoint = "RlLoggerLogOutcomeJson")]
            private static extern int RlLoggerLogOutcomeJsonNative(IntPtr logger, IntPtr eventId, IntPtr outcome, IntPtr apiStatus);

            internal static Func<IntPtr, IntPtr, IntPtr, IntPtr, int> RlLoggerLogOutcomeJsonOverride { get; set; }

            public static int RlLoggerLogCbInteraction(IntPtr liveModel, IntPtr contextJson, IntPtr rankingResponse, IntPtr apiStatus)
            {
                if (RlLoggerLogCbInteractionOverride != null)
                {
                    return RlLoggerLogCbInteractionOverride(liveModel, contextJson, rankingResponse, apiStatus);
                }

                return RlLoggerLogCbInteractionNative(liveModel, contextJson, rankingResponse, apiStatus);
            }

            public static int RlLoggerLogCcbInteraction(IntPtr liveModel, IntPtr contextJson, IntPtr decisionResponse, IntPtr apiStatus)
            {
                if (RlLoggerLogCcbInteractionOverride != null)
                {
                    return RlLoggerLogCcbInteractionOverride(liveModel, contextJson, decisionResponse, apiStatus);
                }

                return RlLoggerLogCcbInteractionNative(liveModel, contextJson, decisionResponse, apiStatus);
            }

            public static int RlLoggerLogOutcomeF(IntPtr liveModel, IntPtr eventId, float outcome, IntPtr apiStatus)
            {
                if (RlLoggerLogOutcomeFOverride != null)
                {
                    return RlLoggerLogOutcomeFOverride(liveModel, eventId, outcome, apiStatus);
                }

                return RlLoggerLogOutcomeFNative(liveModel, eventId, outcome, apiStatus);
            }

            public static int RlLoggerLogOutcomeJson(IntPtr liveModel, IntPtr eventId, IntPtr outcome, IntPtr apiStatus)
            {
                if (RlLoggerLogOutcomeJsonOverride != null)
                {
                    return RlLoggerLogOutcomeJsonOverride(liveModel, eventId, outcome, apiStatus);
                }

                return RlLoggerLogOutcomeJsonNative(liveModel, eventId, outcome, apiStatus);
            }

            [DllImport("rl.net.native.dll")]
            public static extern void RlLoggerSetCallback(IntPtr logger, [MarshalAs(UnmanagedType.FunctionPtr)] managed_background_error_callback_t callback = null);

            [DllImport("rl.net.native.dll")]
            public static extern void RlLoggerSetTrace(IntPtr logger, [MarshalAs(UnmanagedType.FunctionPtr)] managed_trace_callback_t callback = null);
        }
    }

    public sealed class RlLoggerThreadUnsafe : NativeObject<RlLoggerThreadUnsafe>
    {
        private readonly NativeMethods.managed_background_error_callback_t managedErrorCallback;
        private readonly NativeMethods.managed_trace_callback_t managedTraceCallback;

        private static New<RlLoggerThreadUnsafe> BindConstructorArguments(Configuration config)
        {
            return new New<RlLoggerThreadUnsafe>(() =>
            {
                IntPtr result = NativeMethods.CreateRlLogger(config.DangerousGetHandle());
                
                GC.KeepAlive(config); // TODO: Is this one necessary, or does it live on the heap inside of the delegate?
                return result;
            });
        }

        public RlLoggerThreadUnsafe(Configuration config) : base(BindConstructorArguments(config), new Delete<RlLoggerThreadUnsafe>(NativeMethods.DeleteRlLogger))
        {
            this.managedErrorCallback = new NativeMethods.managed_background_error_callback_t(this.WrapStatusAndRaiseBackgroundError);

            // DangerousGetHandle here is trivially safe, because .Dispose() cannot be called before the object is
            // constructed.
            NativeMethods.RlLoggerSetCallback(this.DangerousGetHandle(), this.managedErrorCallback);

            this.managedTraceCallback = new NativeMethods.managed_trace_callback_t(this.SendTrace);
        }

        private static void CheckJsonString(string json)
        {
            if (String.IsNullOrWhiteSpace(json))
            {
                throw new ArgumentException("Configuration json is empty", "json");
            }
        }

        unsafe private static int LoggerLogCbInteraction(IntPtr liveModel, string contextJson, IntPtr rankingResponse, IntPtr apiStatus)
        {
            CheckJsonString(contextJson);

            fixed (byte* contextJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(contextJson))
            {
                IntPtr contextJsonUtf8Ptr = new IntPtr(contextJsonUtf8Bytes);
                return NativeMethods.RlLoggerLogCbInteraction(liveModel, contextJsonUtf8Ptr, rankingResponse, apiStatus);
            }
        }

        unsafe private static int LoggerLogCcbInteraction(IntPtr liveModel, string contextJson, IntPtr decisionResponse, IntPtr apiStatus)
        {
            CheckJsonString(contextJson);

            fixed (byte* contextJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(contextJson))
            {
                IntPtr contextJsonUtf8Ptr = new IntPtr(contextJsonUtf8Bytes);
                return NativeMethods.RlLoggerLogCbInteraction(liveModel, contextJsonUtf8Ptr, decisionResponse, apiStatus);
            }
        }

        unsafe private static int LoggerLogOutcomeF(IntPtr liveModel, string eventId, float outcome, IntPtr apiStatus)
        {
            fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
            {
                return NativeMethods.RlLoggerLogOutcomeF(liveModel, new IntPtr(eventIdUtf8Bytes), outcome, apiStatus);
            }
        }

        unsafe private static int LoggerLogOutcomeJson(IntPtr liveModel, string eventId, string outcomeJson, IntPtr apiStatus)
        {
            fixed (byte* outcomeJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(outcomeJson))
            fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
            {
                return NativeMethods.RlLoggerLogOutcomeJson(liveModel, new IntPtr(eventIdUtf8Bytes), new IntPtr(outcomeJsonUtf8Bytes), apiStatus);
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
            int result = NativeMethods.RlLoggerInit(this.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

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

        public bool Log(string contextJson, RankingResponse response, ApiStatus apiStatus = null)
        {
            return this.TryLog(contextJson, response, apiStatus);
        }

        public bool TryLog(string contextJson, RankingResponse response, ApiStatus apiStatus = null)
        {
            int result = LoggerLogCbInteraction(this.DangerousGetHandle(), contextJson, response.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

            GC.KeepAlive(this);
            return result == NativeMethods.SuccessStatus;
        }

        public void Log(string contextJson, RankingResponse response)
        {
            using (ApiStatus apiStatus = new ApiStatus())
            {
                if (!this.TryLog(contextJson, response, apiStatus))
                {
                    throw new RLException(apiStatus);
                }
            }
        }

        public bool Log(string contextJson, DecisionResponse response, ApiStatus apiStatus = null)
        {
            return this.TryLog(contextJson, response, apiStatus);
        }

        public bool TryLog(string contextJson, DecisionResponse response, ApiStatus apiStatus = null)
        {
            int result = LoggerLogCbInteraction(this.DangerousGetHandle(), contextJson, response.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());

            GC.KeepAlive(this);
            return result == NativeMethods.SuccessStatus;
        }

        public void Log(string contextJson, DecisionResponse response)
        {
            using (ApiStatus apiStatus = new ApiStatus())
            {
                if (!this.TryLog(contextJson, response, apiStatus))
                {
                    throw new RLException(apiStatus);
                }
            }
        }

        public bool Log(string eventId, float outcome, ApiStatus apiStatus = null)
        {
            return this.TryLog(eventId, outcome, apiStatus);
        }

        public bool TryLog(string eventId, float outcome, ApiStatus apiStatus = null)
        {
            int result = LoggerLogOutcomeF(this.DangerousGetHandle(), eventId, outcome, apiStatus.ToNativeHandleOrNullptrDangerous());

            GC.KeepAlive(this);
            return result == NativeMethods.SuccessStatus;
        }

        public void Log(string eventId, float outcome)
        {
            using (ApiStatus apiStatus = new ApiStatus())
            {
                if (!this.TryLog(eventId, outcome, apiStatus))
                {
                    throw new RLException(apiStatus);
                }
            }
        }

        public bool Log(string eventId, string outcome, ApiStatus apiStatus = null)
        {
            return this.TryLog(eventId, outcome, apiStatus);
        }

        public bool TryLog(string eventId, string outcome, ApiStatus apiStatus = null)
        {
            int result = LoggerLogOutcomeJson(this.DangerousGetHandle(), eventId, outcome, apiStatus.ToNativeHandleOrNullptrDangerous());

            GC.KeepAlive(this);
            return result == NativeMethods.SuccessStatus;
        }

        public void Log(string eventId, string outcome)
        {
            using (ApiStatus apiStatus = new ApiStatus())
            {
                if (!this.TryLog(eventId, outcome, apiStatus))
                {
                    throw new RLException(apiStatus);
                }
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
