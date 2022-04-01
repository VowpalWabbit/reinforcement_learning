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
            private static extern IntPtr GetContinuousActionEventIdNative(IntPtr response, int eventIdSize);

            internal static Func<IntPtr, IntPtr, int> GetContinuousActionEventIdOverride { get; set; }

            public static IntPtr GetContinuousActionEventId(IntPtr response, out int eventIdSize)
            {
                if (GetContinuousActionEventIdOverride != null)
                {
                    return GetContinuousActionEventIdOverride(response, eventIdSize);
                }

                return GetContinuousActionEventIdNative(response, eventIdSize);
            }

            [DllImport("rl.net.native.dll", EntryPoint = "GetContinuousActionModelId")]
            private static extern IntPtr GetContinuousActionModelIdNative(IntPtr response, int modelIdSize);

            internal static Func<IntPtr, IntPtr, int> GetContinuousActionModelIdOverride { get; set; }

            public static IntPtr GetContinuousActionModelId(IntPtr response, out int modelIdSize)
            {
                if (GetContinuousActionModelIdOverride != null)
                {
                    return GetContinuousActionModelIdOverride(response, modelIdSize);
                }

                return GetContinuousActionModelIdNative(response, modelIdSize);
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
                int eventIdSize;
                IntPtr eventIdUtf8Ptr = NativeMethods.GetContinuousActionEventId(this.DangerousGetHandle(), eventIdSize);

                string result = NativeMethods.StringMarshallingFunc(eventIdUtf8Ptr, eventIdSize);

                GC.KeepAlive(this);
                return result;
            }
        }

        public string ModelId
        {
            get
            {
                int modelIdSize;
                IntPtr modelIdUtf8Ptr = NativeMethods.GetContinuousActionModelId(this.DangerousGetHandle(), modelIdSize);

                GC.KeepAlive(this);
                return NativeMethods.StringMarshallingFunc(modelIdUtf8Ptr, modelIdSize);
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