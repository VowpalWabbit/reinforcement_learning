using System;
using System.Threading;
using System.Runtime.InteropServices;

using Rl.Net.Native;
using System.Text;

namespace Rl.Net {
    public sealed class ApiStatus : NativeObject<ApiStatus>
    {
        [DllImport("rlnetnative")]
        private static extern IntPtr CreateApiStatus();

        [DllImport("rlnetnative")]
        private static extern void DeleteApiStatus(IntPtr config);

        [DllImport("rlnetnative")]
        private static extern IntPtr GetApiStatusErrorMessage(IntPtr status);

        [DllImport("rlnetnative")]
        private static extern int GetApiStatusErrorCode(IntPtr status);

        [DllImport("rlnetnative")]
        private static extern void UpdateApiStatusSafe(IntPtr status, int error_code, IntPtr message);

        [DllImport("rlnetnative")]
        private static extern void ClearApiStatusSafe(IntPtr status);

        public ApiStatus() : base(new New<ApiStatus>(CreateApiStatus), new Delete<ApiStatus>(DeleteApiStatus))
        {
        }

        internal ApiStatus(IntPtr sharedApiStatusHandle) : base(sharedApiStatusHandle, ownsHandle: false)
        {
        }

        public int ErrorCode
        {
            get
            {
                int result = GetApiStatusErrorCode(this.DangerousGetHandle());

                GC.KeepAlive(this);
                return result;
            }
        }

        public string ErrorMessage
        {
            get
            {
                IntPtr errorMessagePtr = GetApiStatusErrorMessage(this.DangerousGetHandle());

                // We cannot rely on P/Invoke's marshalling here, because it assumes that it can deallocate the string
                // it receives, after converting it to a managed string. We cannot do this, in this case.
                string result = NativeMethods.StringMarshallingFunc(errorMessagePtr);

                GC.KeepAlive(this);
                return result;
            }
        }

        internal static void Update(ApiStatus status, int errorCode, string message)
        {
            unsafe
            {
                fixed (byte* messageBytes = NativeMethods.StringEncoding.GetBytes(message))
                {
                    IntPtr messagePtr = new IntPtr(messageBytes);

                    // Under the hood, api_status will take a copy of the incoming string, so it only needs
                    // to live until after UpdateApiStatusSafe returns. After that it is safe to let the
                    // buffer be unpinned and collected.
                    UpdateApiStatusSafe(status.ToNativeHandleOrNullptrDangerous(), errorCode, messagePtr);

                    GC.KeepAlive(status);
                }
            }
        }
    }

    public sealed class ApiStatusBuilder
    {
        private int errorCode;
        private StringBuilder messageBuilder;

        public ApiStatusBuilder(int errorCode)
        {
            this.errorCode = errorCode;
            this.messageBuilder = new StringBuilder(NativeMethods.MarshalMessageForErrorCode(errorCode));
        }

        public ApiStatusBuilder Append(string message)
        {
            this.messageBuilder.Append(message);

            return this;
        }

        public ApiStatusBuilder AppendLine(string message)
        {
            this.messageBuilder.AppendLine(message);

            return this;
        }

        public int UpdateApiStatus(ApiStatus target)
        {
            ApiStatus.Update(target, this.errorCode, this.messageBuilder.ToString());
            return this.errorCode;
        }

        public ApiStatus ToApiStatus()
        {
            ApiStatus result = new ApiStatus();
            ApiStatus.Update(result, this.errorCode, this.messageBuilder.ToString());
            return result;
        }
    }
}
