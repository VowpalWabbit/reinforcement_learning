using System;
using System.Runtime.InteropServices;
using Rl.Net.Native;

namespace Rl.Net {
    public sealed class SharedBuffer : NativeObject<SharedBuffer>
    {
        [DllImport("rl.net.native")]
        private static extern IntPtr CloneBufferSharedPointer(IntPtr original);

        [DllImport("rl.net.native")]
        private static extern void ReleaseBufferSharedPointer(IntPtr shared_buffer);

        [DllImport("rl.net.native")]
        private static extern IntPtr GetSharedBufferBegin(IntPtr shared_buffer);

        [DllImport("rl.net.native")]
        private static extern UIntPtr GetSharedBufferLength(IntPtr status);

        private static New<SharedBuffer> BindConstructorArguments(SharedBuffer original)
        {
            return new New<SharedBuffer>(() =>
            {
                IntPtr result = CloneBufferSharedPointer(original.DangerousGetHandle());

                GC.KeepAlive(original);
                return result;
            });
        }

        internal SharedBuffer(IntPtr sharedBufferHandle) : base(sharedBufferHandle, ownsHandle:false)
        { }

        /// <remarks>This is a copy constructor</remarks>
        public SharedBuffer(SharedBuffer original) : base(BindConstructorArguments(original), new Delete<SharedBuffer>(ReleaseBufferSharedPointer))
        { }

        public ReadOnlySpan<byte> AsSpanDangerous()
        {
            unsafe {
                IntPtr handle = this.DangerousGetHandle();
                ReadOnlySpan<byte> result = new ReadOnlySpan<byte>(GetSharedBufferBegin(handle).ToPointer(), (int)GetSharedBufferLength(handle).ToUInt32());

                GC.KeepAlive(this); // Technically, this will not really help - the caller is responsible for making sure this stays alive
                                    // for the duration of the life of the ReadOnlySpan

                return result;
            }
        }
    }
}