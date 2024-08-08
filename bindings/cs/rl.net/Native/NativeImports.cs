namespace Rl.Net.Native {
   internal static class NativeImports {
      #if DEBUG
      internal const string RLNETNATIVE = "rlnetnatived";
      #else
      internal const string RLNETNATIVE = "rlnetnative";
      #endif
   }
}
