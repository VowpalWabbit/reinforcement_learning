using System;
using System.Runtime.CompilerServices;

[assembly: InternalsVisibleTo("rl.net.cli.test")]

namespace Rl.Net
{
    [Flags]
    public enum ActionFlags : uint
    {
        Default = 0,
        Deferred = 1,
    }
}
