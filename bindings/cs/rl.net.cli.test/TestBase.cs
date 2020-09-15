using Microsoft.VisualStudio.TestTools.UnitTesting;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Rl.Net;

namespace Rl.Net.Cli.Test
{
    public abstract class TestBase
    {
        protected CleanupContainer TestCleanup
        {
            get;
            private set;
        } = new CleanupContainer();

        [TestCleanup]
        public void CleanupTest()
        {
            this.TestCleanup.Dispose();
        }
    }
}