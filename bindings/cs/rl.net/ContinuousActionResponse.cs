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
            private static extern IntPtr GetContinuousActionEventIdNative(IntPtr response);

            internal static Func<IntPtr, IntPtr> GetContinuousActionEventIdOverride { get; set; }

            public static IntPtr GetContinuousActionEventId(IntPtr response)
            {
                if (GetContinuousActionEventIdOverride != null)
                {
                    return GetContinuousActionEventIdOverride(response);
                }

                return GetContinuousActionEventIdNative(response);
            }

            [DllImport("rl.net.native.dll", EntryPoint = "GetContinuousActionModelId")]
            private static extern IntPtr GetContinuousActionModelIdNative(IntPtr response);

            internal static Func<IntPtr, IntPtr> GetContinuousActionModelIdOverride { get; set; }

            public static IntPtr GetContinuousActionModelId(IntPtr response)
            {
                if (GetContinuousActionModelIdOverride != null)
                {
                    return GetContinuousActionModelIdOverride(response);
                }

                return GetContinuousActionModelIdNative(response);
            }

            [DllImport("rl.net.native.dll")]
            public static extern float GetContinuousChosenAction(IntPtr response);

            [DllImport("rl.net.native.dll")]
            public static extern float GetContinuousChosenActionPdfValue(IntPtr response);
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
                IntPtr eventIdUtf8Ptr = NativeMethods.GetContinuousActionEventId(this.DangerousGetHandle());

                string result = NativeMethods.StringMarshallingFunc(eventIdUtf8Ptr);

                GC.KeepAlive(this);
                return result;
            }
        }

        public string ModelId
        {
            get
            {
                IntPtr modelIdUtf8Ptr = NativeMethods.GetContinuousActionModelId(this.DangerousGetHandle());

                GC.KeepAlive(this);
                return NativeMethods.StringMarshallingFunc(modelIdUtf8Ptr);
            }
        }

        public float ChosenAction
        {
            get
            {
                float result = NativeMethods.GetContinuousChosenAction(this.DangerousGetHandle());

                GC.KeepAlive(this);
                return result;
            }
        }

        public float ChosenActionPdfValue
        {
            get
            {
                float result = NativeMethods.GetContinuousChosenActionPdfValue(this.DangerousGetHandle());

                GC.KeepAlive(this);
                return result;
            }
        }
    }
}