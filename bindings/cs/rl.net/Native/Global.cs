using System;
using System.Threading;
using System.Runtime.InteropServices;
using System.Text;

namespace Rl.Net.Native {
    internal static partial class NativeMethods
    {
        public static readonly Encoding StringEncoding = Encoding.UTF8;
        public static readonly Func<IntPtr, string> StringMarshallingFunc = StringExtensions.PtrToStringUtf8;

        public const int SuccessStatus = 0; // See err_constants.h

        public static IntPtr ToNativeHandleOrNullptr<TObject>(this NativeObject<TObject> nativeObject) where TObject : NativeObject<TObject>
        {
            if (nativeObject == null)
            {
                return IntPtr.Zero;
            }

            return nativeObject.NativeHandle;
        }
    }
}