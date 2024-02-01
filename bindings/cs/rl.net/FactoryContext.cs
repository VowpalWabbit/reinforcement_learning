using System;
using System.Threading;
using System.Runtime.InteropServices;
using System.Collections.Generic;

using Rl.Net.Native;

namespace Rl.Net {
    public sealed class FactoryContext : NativeObject<FactoryContext>
    {
        [DllImport("rlnetnative")]
        private static extern IntPtr CreateFactoryContext();

        [DllImport("rlnetnative")]
        private static extern void DeleteFactoryContext(IntPtr context);

        [DllImport("rlnetnative")]
        private static extern IntPtr SetFactoryContextBindingSenderFactory(IntPtr context, sender_create_fn create_Fn, sender_vtable vtable);

        public List<byte> Weights { get; private set; }

        public FactoryContext(List<byte> weights = null) : base(new New<FactoryContext>(CreateFactoryContext), new Delete<FactoryContext>(DeleteFactoryContext))
        {
            Weights = weights ?? new List<byte>();
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
