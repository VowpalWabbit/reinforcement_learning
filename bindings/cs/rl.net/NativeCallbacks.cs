using System;

namespace Rl.Net
{
    namespace Native
    {
        internal static partial class NativeMethods
        {
            public delegate void managed_background_error_callback_t(IntPtr apiStatus);
            public delegate void managed_trace_callback_t(int logLevel, IntPtr msgUtf8Ptr);
        }
    }
}