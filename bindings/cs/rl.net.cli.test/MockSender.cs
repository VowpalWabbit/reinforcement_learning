using System;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Rl.Net;
using Rl.Net.Native;

namespace Rl.Net.Cli.Test
{
    using SenderFactory = Func<IReadOnlyConfiguration, ErrorCallback, ISender>;
    using BackgroundErrorCallback = Action<ApiStatus>;

    internal class MockSender : ISender
    {
        public Action<ApiStatus> Init
        {
            get;
            set;
        }

        public Action<SharedBuffer, ApiStatus> Send
        {
            get;
            set;
        }

        void ISender.Init(ApiStatus status)
        {
            if (this.Init != null)
            {
                this.Init(status);
            }
        }

        void ISender.Send(SharedBuffer buffer, ApiStatus status)
        {
            if (this.Send != null)
            {
                this.Send(buffer, status);
            }
        }
    }


    internal class MockAsyncSender : AsyncSender
    {
        public MockAsyncSender(ErrorCallback callback) : base(callback)
        {
        }

        public new Func<SharedBuffer, BackgroundErrorCallback, Task> Send
        {
            get;
            set;
        }

        protected override Task SendAsync(SharedBuffer buffer)
        {
            if (this.Send != null)
            {
                return this.Send(buffer, this.RaiseBackgroundError);
            }

            return Task.CompletedTask;
        }
    }
}