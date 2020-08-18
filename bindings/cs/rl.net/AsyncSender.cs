using System;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using Rl.Net.Native;

namespace Rl.Net
{
    public abstract class AsyncSender : ISender
    {
        private ErrorCallback errorCallback;

        public AsyncSender(ErrorCallback callback)
        {
            this.errorCallback = callback;
        }

        public virtual void Init(ApiStatus status)
        {
        }

        public void Send(SharedBuffer buffer, ApiStatus status)
        {
            // Create a cloned handle so that we have an our own copy of the shared pointer to the buffer 
            SharedBuffer ownedHandle = new SharedBuffer(buffer);
            GC.KeepAlive(buffer);

            Task backgroundTask = SendAsyncAndUnwrapExceptions(ownedHandle);
        }

        private async Task SendAsyncAndUnwrapExceptions(SharedBuffer buffer)
        {
            using (ApiStatus status = new ApiStatus())
            {
                try
                {
                    await this.SendAsync(buffer);
                }
                catch (RLException e)
                {
                    e.UpdateApiStatus(status);
                    this.RaiseBackgroundError(status);
                }
                catch (Exception e)
                {
                    ApiStatusBuilder builder = new ApiStatusBuilder(NativeMethods.OpaqueBindingError)
                        .AppendLine(e.Message)
                        .AppendLine(e.StackTrace);
                    builder.UpdateApiStatus(status);

                    this.RaiseBackgroundError(status);
                }
            }

            GC.KeepAlive(buffer);
        }

        protected void RaiseBackgroundError(ApiStatus status)
        {
            this.errorCallback.Invoke(status);
        }

        protected abstract Task SendAsync(SharedBuffer buffer);
    }
}
