using System;
using System.Collections.Generic;
using System.Runtime.ConstrainedExecution;
using System.Runtime.InteropServices;
using System.Text;

namespace Rl.Net.Native
{
    public sealed class GCHandleLifetime : CriticalFinalizerObject, IDisposable
    {
        private bool disposedValue;
        private GCHandle gcHandle;

        public GCHandleLifetime(object target, GCHandleType type)
        {
            this.gcHandle = GCHandle.Alloc(target, type);
        }

        public IntPtr Pointer
        {
            get
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(GCHandleLifetime));
                }

                return GCHandle.ToIntPtr(gcHandle);
            }
        }

        ~GCHandleLifetime()
        {
            this.Dispose(disposing: false);
        }

        private void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                gcHandle.Free();
                disposedValue = true;
            }
        }

        void IDisposable.Dispose()
        {
            // Do not change this code. Put cleanup code in 'Dispose(bool disposing)' method
            this.Dispose(disposing: true);
            GC.SuppressFinalize(this);
        }
    }
}
