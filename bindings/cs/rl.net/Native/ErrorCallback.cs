using System;
using System.Threading;
using System.Runtime.InteropServices;
using System.Text;

namespace Rl.Net.Native
{
    internal delegate void error_fn(IntPtr error_context, IntPtr status);

    public class ErrorCallback
    {
        private IntPtr error_context;
        private error_fn callback;

        internal ErrorCallback(error_fn callback, IntPtr error_context)
        {
            this.callback = callback;
            this.error_context = error_context;
        }

        public void Invoke(ApiStatus status)
        {
            if (status != null)
            {
                this.callback(this.error_context, status.DangerousGetHandle());
                GC.KeepAlive(status);
            }
        }
    }
}
