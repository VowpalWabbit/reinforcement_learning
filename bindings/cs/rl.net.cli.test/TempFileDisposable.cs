using System;
using System.IO;

namespace Rl.Net.Cli.Test
{
    internal sealed class TempFileDisposable : IDisposable
    {
        public TempFileDisposable()
        {
            this.Path = System.IO.Path.GetTempFileName();
        }

        public string Path
        {
            get;
            private set;
        }

        public void Dispose()
        {
            try
            {
                if (File.Exists(this.Path))
                {
                    File.Delete(this.Path);
                }

                if (Directory.Exists(this.Path))
                {
                    Directory.Delete(this.Path, recursive: true);
                }
            }
            catch
            {
                // TestCleanup is best-efforts
            }
        }
    }
}
