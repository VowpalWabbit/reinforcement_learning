using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Runtime.InteropServices;

using Rl.Net.Native;
using System.Collections;

namespace Rl.Net {
    namespace Native
    {
        internal partial class NativeMethods
        {
            [DllImport("rl.net.native.dll")]
            public static extern IntPtr CreateContinuousActionResponse();

            [DllImport("rl.net.native.dll")]
            public static extern void DeleteContinuousActionResponse(IntPtr response);

            [DllImport("rl.net.native.dll", EntryPoint = "GetContinuousActionEventId")]
            private static extern IntPtr GetContinuousActionEventIdNative(IntPtr response, out int eventIdSize);

            internal static Func<IntPtr, IntPtr> GetContinuousActionEventIdOverride { get; set; }

            public static IntPtr GetContinuousActionEventId(IntPtr response, out int eventIdSize)
            {
                eventIdSize = 0;
                if (GetContinuousActionEventIdOverride != null)
                {
                    IntPtr eventId = GetContinuousActionEventIdOverride(response);
                    string marshalledBack = NativeMethods.StringMarshallingFunc(eventId);
                    eventIdSize = NativeMethods.StringEncoding.GetByteCount(marshalledBack);
                    return eventId;
                }

                return GetContinuousActionEventIdNative(response, out eventIdSize);
            }

            [DllImport("rl.net.native.dll", EntryPoint = "GetContinuousActionModelId")]
            private static extern IntPtr GetContinuousActionModelIdNative(IntPtr response, out int modelIdSize);

            internal static Func<IntPtr, IntPtr> GetContinuousActionModelIdOverride { get; set; }

            public static IntPtr GetContinuousActionModelId(IntPtr response, out int modelIdSize)
            {
                modelIdSize = 0;
                if (GetContinuousActionModelIdOverride != null)
                {
                    IntPtr modelId = GetContinuousActionModelIdOverride(response);
                    string marshalledBack = NativeMethods.StringMarshallingFunc(modelId);
                    modelIdSize = NativeMethods.StringEncoding.GetByteCount(marshalledBack);
                    return modelId;
                }

                return GetContinuousActionModelIdNative(response, out modelIdSize);
            }

            [DllImport("rl.net.native.dll")]
            public static extern float GetContinuousActionChosenAction(IntPtr response);

            [DllImport("rl.net.native.dll")]
            public static extern float GetContinuousActionChosenActionPdfValue(IntPtr response);
        }
    }

    public sealed class ContinuousActionResponse : NativeObject<ContinuousActionResponse>
    {
        public ContinuousActionResponse() : base(new New<ContinuousActionResponse>(NativeMethods.CreateContinuousActionResponse), new Delete<ContinuousActionResponse>(NativeMethods.DeleteContinuousActionResponse))
        {
        }

        public string EventId
        {
            get
            {
                int eventIdSize = 0;
                IntPtr eventIdUtf8Ptr = NativeMethods.GetContinuousActionEventId(this.DangerousGetHandle(), out eventIdSize);

                string result = NativeMethods.StringMarshallingFuncWithSize(eventIdUtf8Ptr, eventIdSize);

                GC.KeepAlive(this);
                return result;
            }
        }

        public string ModelId
        {
            get
            {
                int modelIdSize = 0;
                IntPtr modelIdUtf8Ptr = NativeMethods.GetContinuousActionModelId(this.DangerousGetHandle(), out modelIdSize);

                GC.KeepAlive(this);
                return NativeMethods.StringMarshallingFuncWithSize(modelIdUtf8Ptr, modelIdSize);
            }
        }

        public float ChosenAction
        {
            get
            {
                float result = NativeMethods.GetContinuousActionChosenAction(this.DangerousGetHandle());

                GC.KeepAlive(this);
                return result;
            }
        }

        public float ChosenActionPdfValue
        {
            get
            {
                float result = NativeMethods.GetContinuousActionChosenActionPdfValue(this.DangerousGetHandle());

                GC.KeepAlive(this);
                return result;
            }
        }
    }
}