using System;
using System.Runtime.InteropServices;

using Rl.Net.Native;

namespace Rl.Net
{
  public interface ISender
  {
    void Init(ApiStatus status);

    void Send(SharedBuffer buffer, ApiStatus status);
  }
}
