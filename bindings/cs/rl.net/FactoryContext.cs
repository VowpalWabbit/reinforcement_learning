using System;
using System.Threading;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Linq;

using Rl.Net.Native;

namespace Rl.Net {
    public sealed class FactoryContext : NativeObject<FactoryContext>
    {
        [DllImport(NativeImports.RLNETNATIVE)]
        private static extern IntPtr CreateFactoryContext();

        [DllImport(NativeImports.RLNETNATIVE)]
        private static extern IntPtr CreateFactoryContextWithStaticModel(IntPtr vw_model, int len);

        [DllImport(NativeImports.RLNETNATIVE)]
        private static extern void DeleteFactoryContext(IntPtr context);

        [DllImport(NativeImports.RLNETNATIVE)]
        private static extern IntPtr SetFactoryContextBindingSenderFactory(IntPtr context, sender_create_fn create_Fn, sender_vtable vtable);

        public FactoryContext() : base(new New<FactoryContext>(CreateFactoryContext), new Delete<FactoryContext>(DeleteFactoryContext))
        {
        }

        public FactoryContext(IEnumerable<byte> vwModelEnumerable) : base(
            new New<FactoryContext>(() => {
                var vwModelArray = vwModelEnumerable.ToArray();
                GCHandle handle = GCHandle.Alloc(vwModelArray, GCHandleType.Pinned);
                try {
                    IntPtr ptr = handle.AddrOfPinnedObject();
                    return CreateFactoryContextWithStaticModel(ptr, vwModelArray.Length);
                }
                finally {
                    if (handle.IsAllocated)
                        handle.Free();
                }
            }),
            new Delete<FactoryContext>(DeleteFactoryContext))
        {
        }
 
        private GCHandleLifetime registeredSenderCreateHandle;

        public void SetSenderFactory<TSender>(Func<IReadOnlyConfiguration, ErrorCallback, TSender> createSender) where TSender : ISender
        {
            sender_create_fn create_fn =
                (IntPtr configuration, error_fn error_callback, IntPtr error_context) =>
                {
                    ErrorCallback errorCallback = new ErrorCallback(error_callback, error_context);
                    Configuration readableConfig = new Configuration(configuration);

                    TSender sender = createSender((IReadOnlyConfiguration)readableConfig, errorCallback);

                    SenderAdapter adapter = new SenderAdapter(sender);

                    // Unpinning this happens inside of SenderAdapter when Release is invoked
                    GCHandle adapterHandle = GCHandle.Alloc(adapter);

                    return GCHandle.ToIntPtr(adapterHandle);
                };

            GCHandleLifetime localCreateHandle = new GCHandleLifetime(create_fn, GCHandleType.Normal);

            SetFactoryContextBindingSenderFactory(this.DangerousGetHandle(), create_fn, SenderAdapter.VTable);

            Interlocked.Exchange(ref this.registeredSenderCreateHandle, localCreateHandle);

            // GCHandleLifetime cleans itself up, and does not need to be explicitly .Dispose()d.
        }
    }
}
