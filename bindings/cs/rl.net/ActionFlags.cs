using System;

namespace Rl.Net
{
    [Flags]
    public enum ActionFlags : uint
    {
        Default = 0,
        Deferred = 1,
    }
}
