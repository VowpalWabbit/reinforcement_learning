using System;
using System.Collections.Generic;
using System.Threading;

namespace Rl.Net.Cli.Test
{
    public sealed class CleanupContainer : IDisposable
    {
        private Stack<Action> cleanupStack = new Stack<Action>();

        public void Dispose()
        {
            Stack<Action> localCleanupStack = Interlocked.Exchange(ref cleanupStack, new Stack<Action>());
            while (localCleanupStack.TryPop(out Action action))
            {
                try
                {
                    action?.Invoke();
                }
                catch
                {
                    // Suppress errors on TestCleanup. If we want to detect state errors on
                    // cleanup, it should be an explicit part of a test.
                }
            }
        }

        public void Add(IDisposable disposable)
        {
            this.cleanupStack.Push(disposable.Dispose);
        }

        public void Add(Action action)
        {
            this.cleanupStack.Push(action);
        }
    }
}
