using System;
using System.ComponentModel;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Collections.Generic;
using Rl.Net.Native;

namespace Rl.Net
{
    namespace Native
    {
        internal static partial class NativeMethods
        {
            [DllImport(NativeImports.RLNETNATIVE)]
            public static extern void RegisterDefaultFactoriesCallback(
                [MarshalAs(UnmanagedType.FunctionPtr)] managed_oauth_callback_t callback,
                [MarshalAs(UnmanagedType.FunctionPtr)] managed_oauth_callback_t_complete_t completion);
        }
    }

    public class OAuthTokenRequestedEventArgs : EventArgs
    {
        public IList<string> Scopes { get; }

        public string Token { get; set; }

        public DateTime TokenExpirationTime { get; set; }

        public int ErrorCode { get; set; }

        public OAuthTokenRequestedEventArgs(IList<string> scopes)
        {
            Scopes = scopes;
        }
    }

    /// <summary>
    /// OAuthCredentialProvider is used to provide OAuth tokens to the underlying rlclientlib library.
    /// </summary>
    /// <remarks>
    /// OAuthCredentialProvider is currently static and should be sufficient for most applications.
    /// To control the OAuth token retrieval process, subscribe to the OAuthTokenRequested event.
    /// Use scopes if you need to request different tokens for different scopes which should be handled
    /// in the event handler.
    /// 
    /// Example Usage:
    /// 
    ///     static class EntryPoints
    ///     {
    ///         private static ManagedIdentityCredential managedIdentityCredential;
    /// 
    ///         private static void OnOAuthTokenRequest(object src, OAuthTokenRequestedEventArgs e)
    ///         {
    ///             try {
    ///                 var accessToken = managedIdentityCredential.GetToken(e.Scopes);
    ///                 e.Token = accessToken.Token
    ///                 e.TokenExpirationTime = accessToken.ExpiresOn;
    ///                 e.ErrorCode = 0;
    ///             }
    ///             catch (AuthenticationFailedException) {
    ///                 e.ErrorCode = 4; // http_bad_status_code
    ///             }
    ///         }
    /// 
    ///         public static void Main(string[] args)
    ///         {
    ///             // some startup code is here; do not use anything that would invoke an OAuth token request
    ///             var my_client_id = args[1];
    ///             managedIdentityCredential = new ManagedIdentityCredential(my_client_id);
    ///             // ...
    ///             // setup the OAuthCredentialProvider
    ///             OAuthCredentialProvider.OAuthTokenRequested += OnOAuthTokenRequest;
    ///             // now OAuth requests are ready to be handled
    ///         }
    ///     }
    /// 
    /// </remarks>
    public static class OAuthCredentialProvider
    {
        private static readonly NativeMethods.managed_oauth_callback_t oauthCredentialCallback;
        private static readonly NativeMethods.managed_oauth_callback_t_complete_t oauthCredentialCallbackCompletion;

        static OAuthCredentialProvider()
        {
            oauthCredentialCallback = new NativeMethods.managed_oauth_callback_t(WrapOAuthCredentialCallback);
            oauthCredentialCallbackCompletion = new NativeMethods.managed_oauth_callback_t_complete_t(WrapOAuthCredentialCallbackCompletion);
            NativeMethods.RegisterDefaultFactoriesCallback(oauthCredentialCallback, oauthCredentialCallbackCompletion);
        }

        public static event EventHandler<OAuthTokenRequestedEventArgs> OAuthTokenRequested;

        private static int WrapOAuthCredentialCallback(IntPtr scopes, IntPtr tokenOutPtr, IntPtr expiryUnixTime)
        {
            var scopesArray = StringArrayFromNativeUtf8Strings(scopes);
            var e = new OAuthTokenRequestedEventArgs(scopesArray);
            OAuthTokenRequested?.Invoke(null, e);
            if (e.ErrorCode == 0)
            {
                var outPtr = CreateUnmanagedString(e.Token ?? "");
                Marshal.WriteIntPtr(tokenOutPtr, outPtr);
                Marshal.WriteInt64(expiryUnixTime, (new DateTimeOffset(e.TokenExpirationTime)).ToUnixTimeSeconds());
            }
            return e.ErrorCode;
        }

        private static void WrapOAuthCredentialCallbackCompletion(IntPtr tokenStringToFree, int _)
        {
            if (tokenStringToFree != IntPtr.Zero)
            {
                Marshal.FreeHGlobal(tokenStringToFree);
            }
        }

        private static IList<string> StringArrayFromNativeUtf8Strings(IntPtr nativeStrings)
        {
            if (nativeStrings == IntPtr.Zero)
            {
                throw new ArgumentNullException(nameof(nativeStrings));
            }
            var list = new List<string>();
            IntPtr strPtr = Marshal.ReadIntPtr(nativeStrings);
            for (int i = 1; strPtr != IntPtr.Zero; ++i)
            {
                list.Add(Marshal.PtrToStringAnsi(strPtr));
                strPtr = Marshal.ReadIntPtr(nativeStrings, i * IntPtr.Size);
            }
            return list;
        }

        private static IntPtr CreateUnmanagedString(string str)
        {
            if (str == null)
            {
                throw new ArgumentNullException(nameof(str));
            }
            byte[] bytes = System.Text.Encoding.UTF8.GetBytes(str);
            IntPtr unmanagedString = Marshal.AllocHGlobal(bytes.Length + 1);
            Marshal.Copy(bytes, 0, unmanagedString, bytes.Length);
            Marshal.WriteByte(unmanagedString, bytes.Length, 0);
            return unmanagedString;
        }
    }
}