using System;
using System.Collections.Generic;
using System.Threading;

namespace Rl.Net.Cli.Test
{
    public sealed class CleanupContainer : IDisposable
    {
        private Stack<IDisposable> cleanupStack = new Stack<IDisposable>();

        public void Dispose()
        {
            Stack<IDisposable> localCleanupStack = Interlocked.Exchange(ref cleanupStack, new Stack<IDisposable>());
            while (localCleanupStack.TryPop(out IDisposable cleanupDisposable))
            {
                if (cleanupDisposable != null)
                {
                    cleanupDisposable.Dispose();
                }
            }
        }

        private sealed class ActionDisposable : IDisposable
        {
            private Action disposeAction;

            public ActionDisposable(Action action)
            {
                this.disposeAction = action;
            }

            public void Dispose()
            {
                Action localAction = Interlocked.Exchange(ref this.disposeAction, null);
                if (localAction != null)
                {
                    localAction();
                }
            }
        }

        public void Add(IDisposable disposable)
        {
            this.cleanupStack.Push(disposable);
        }

        public void Add(Action action)
        {
            this.cleanupStack.Push(new ActionDisposable(action));
        }
    }
}
