using System;
using System.Threading;
using System.Runtime.InteropServices;
using System.Text;
using System.Diagnostics;

namespace Rl.Net.Native {
    internal delegate IntPtr sender_create_fn(IntPtr configuration, [MarshalAs(UnmanagedType.FunctionPtr)]error_fn error_callback, IntPtr error_ctx);

    internal delegate int sender_init_fn(IntPtr managed_handle, IntPtr status);

    internal delegate int sender_send_fn(IntPtr managed_handle, IntPtr buffer, IntPtr status);

    internal delegate void sender_release_fn(IntPtr managed_handle);

    [StructLayout(LayoutKind.Sequential)]
    internal struct sender_vtable
    {
        [MarshalAs(UnmanagedType.FunctionPtr)] 
        public sender_init_fn init;

        [MarshalAs(UnmanagedType.FunctionPtr)] 
        public sender_send_fn send;

        [MarshalAs(UnmanagedType.FunctionPtr)] 
        public sender_release_fn release;
    }

    internal class SenderAdapter
    {
        private ISender senderImplementation;

        public SenderAdapter(ISender senderImplementation)
        {
            this.senderImplementation = senderImplementation;
        }

        public static sender_vtable VTable
        {
            get;
        } = CreateAdapterVTable();

        private static sender_vtable CreateAdapterVTable()
        {
            return new sender_vtable
            {
                init = (managed_handle, status) => InvokeInit(managed_handle, status),
                send = (managed_handle, buffer, status) => InvokeSend(managed_handle, buffer, status),
                release = (managed_handle) => InvokeRelease(managed_handle)
            };
        }

        private static SenderAdapter GetAdapterOrThrow(IntPtr managed_handle)
        {
            GCHandle gcHandle = GCHandle.FromIntPtr(managed_handle);
            SenderAdapter adapter = gcHandle.Target as SenderAdapter;

            // This should never happen, unless there is a bug in the binding implementation.
            if (adapter == null)
            {
                StringBuilder errorBuilder = new StringBuilder(NativeMethods.OpaqueBindingErrorMessage)
                    .Append("Invalid managed_handle in ISender binding adapter. Expecting non-null RL.Net.Native.SenderAdapter. Got ")
                    .AppendFormat("{0}.", gcHandle.Target == null ? "null" : gcHandle.Target.GetType().FullName);

                throw new RLException(errorBuilder.ToString());
            }

            return adapter;
        }

        private static int InvokeAndUnwrapExceptions(Func<ApiStatus, int> fn, IntPtr status)
        {
            using (ApiStatus apiStatus = new ApiStatus(status))
            {
                try
                {
                    return fn(apiStatus);
                }
                catch (RLException e)
                {
                    return e.UpdateApiStatus(apiStatus);
                }
                catch (Exception e)
                {
                    return new ApiStatusBuilder(NativeMethods.OpaqueBindingError)
                        .AppendLine(e.Message)
                        .AppendLine(e.StackTrace)
                        .UpdateApiStatus(apiStatus);
                }
            }
        }

        private static int InvokeInit(IntPtr managed_handle, IntPtr status)
        {
            return SenderAdapter.InvokeAndUnwrapExceptions(
                (ApiStatus apiStatus) =>
                {
                    SenderAdapter adapter = GetAdapterOrThrow(managed_handle);

                    adapter.senderImplementation.Init(apiStatus);
                    return apiStatus.ErrorCode;
                },
                status
            );
        }

        private static int InvokeSend(IntPtr managed_handle, IntPtr buffer, IntPtr status)
        {
            return SenderAdapter.InvokeAndUnwrapExceptions(
                (ApiStatus apiStatus) =>
                {
                    SenderAdapter adapter = GetAdapterOrThrow(managed_handle);
                    SharedBuffer sharedBuffer = new SharedBuffer(buffer);

                    // Buffer must live at least as long as the underlying call.
                    unsafe {
                        adapter.senderImplementation.Send(sharedBuffer, apiStatus);
                    }
                    
                    return apiStatus.ErrorCode;
                },
                status
            );
        }

        private static void InvokeRelease(IntPtr managed_handle)
        {
            GCHandle gcHandle = GCHandle.FromIntPtr(managed_handle);

            if (gcHandle.Target is SenderAdapter)
            {
                // Only release if we are getting the right type of object here
                gcHandle.Free();
            }
            else
            {
                // TODO: Raise Background Error - but it is unclear how, since we no longer have a LiveModel context...
                Debug.Fail("Getting wrong object in SenderAdapter's InvokeRelease.");

                // A failure here is a potential memory leak, but should not impact correctness, so may not be a
                // good idea to just throw arbitrarily here.
            }
        }
    }
}
