using System;
using System.Threading;
using System.Runtime.InteropServices;

using Rl.Net.Native;
using System.Text;

namespace Rl.Net {

    using SenderFactory = Func<IReadOnlyConfiguration, ErrorCallback, ISender>;

    namespace Native
    {
        // The publics in this class are just a verbose, but jittably-efficient way of enabling overriding a native invocation
        internal static partial class NativeMethods
        {
            [DllImport("rl.net.native")]
            public static extern IntPtr CreateConfig();

            [DllImport("rl.net.native")]
            public static extern void DeleteConfig(IntPtr config);

            [DllImport("rl.net.native", EntryPoint = "LoadConfigurationFromJson")]
            private static extern int LoadConfigurationFromJsonNative(int jsonLength, IntPtr json, IntPtr config, IntPtr apiStatus);

            internal static Func<int, IntPtr, IntPtr, IntPtr, int> LoadConfigurationFromJsonOverride { get; set; }

            public static int LoadConfigurationFromJson(int jsonLength, IntPtr json, IntPtr config, IntPtr apiStatus)
            {
                if (LoadConfigurationFromJsonOverride != null)
                {
                    return LoadConfigurationFromJsonOverride(jsonLength, json, config, apiStatus);
                }

                return LoadConfigurationFromJsonNative(jsonLength, json, config, apiStatus);
            }

            [DllImport("rl.net.native", EntryPoint = "ConfigurationSet")]
            private static extern void ConfigurationSetNative(IntPtr config, IntPtr name, IntPtr value);

            internal static Action<IntPtr, IntPtr, IntPtr> ConfigurationSetOverride { get; set; }

            public static void ConfigurationSet(IntPtr config, IntPtr name, IntPtr value)
            {
                if (ConfigurationSetOverride != null)
                {
                    ConfigurationSetOverride(config, name, value);
                    return;
                }

                ConfigurationSetNative(config, name, value);
            }

            [DllImport("rl.net.native", EntryPoint = "ConfigurationGet")]
            private static extern IntPtr ConfigurationGetNative(IntPtr config, IntPtr name, IntPtr defVal);

            internal static Func<IntPtr, IntPtr, IntPtr, IntPtr> ConfigurationGetOverride { get; set; }

            public static IntPtr ConfigurationGet(IntPtr config, IntPtr name, IntPtr defVal)
            {
                if (ConfigurationGetOverride != null)
                {
                    return ConfigurationGetOverride(config, name, defVal);
                }

                return ConfigurationGetNative(config, name, defVal);
            }
        }
    }

    public interface IReadOnlyConfiguration
    {
        string this[string key] { get; }
    }

    public sealed class Configuration: NativeObject<Configuration>, IReadOnlyConfiguration
    {


        public Configuration() : base(new New<Configuration>(NativeMethods.CreateConfig), new Delete<Configuration>(NativeMethods.DeleteConfig))
        {
        }

        internal Configuration(IntPtr configuration): base(configuration, ownsHandle: false)
        {}

        unsafe internal static void ConfigurationSet(IntPtr config, string name, string value)
        {
            if (name == null)
            {
                throw new ArgumentNullException("name", "Configuration entry name cannot be null.");
            }

            fixed (byte* nameUtf8Bytes = NativeMethods.StringEncoding.GetBytes(name))
            {
                IntPtr nameUtf8Ptr = new IntPtr(nameUtf8Bytes);

                if (value == null)
                {
                    NativeMethods.ConfigurationSet(config, nameUtf8Ptr, IntPtr.Zero);
                }
                else
                {
                    fixed (byte* valueUtf8Bytes = NativeMethods.StringEncoding.GetBytes(value))
                    {
                        NativeMethods.ConfigurationSet(config, nameUtf8Ptr, new IntPtr(valueUtf8Bytes));
                    }
                }
            }
        }

        unsafe internal static string ConfigurationGet(IntPtr config, string name, string defVal)
        {
            if (name == null)
            {
                throw new ArgumentNullException("name", "Configuration entry name cannot be null.");
            }

            defVal = defVal ?? String.Empty;

            fixed (byte* nameUtf8Bytes = NativeMethods.StringEncoding.GetBytes(name))
            fixed (byte* defValUtf8Bytes = NativeMethods.StringEncoding.GetBytes(defVal))
            {
                IntPtr nameUtf8Ptr = new IntPtr(nameUtf8Bytes);
                IntPtr defValUtf8Ptr = new IntPtr(defValUtf8Bytes);

                IntPtr result = NativeMethods.ConfigurationGet(config, nameUtf8Ptr, defValUtf8Ptr);

                if (result == IntPtr.Zero)
                {
                    return string.Empty;
                }

                return NativeMethods.StringMarshallingFunc(result);
            }
        }

        unsafe internal static int LoadConfigurationFromJson(string json, IntPtr config, IntPtr apiStatus)
        {
            if (String.IsNullOrWhiteSpace(json))
            {
                throw new ArgumentException("Configuration json is empty", "json");
            }

            byte[] unpinnedJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(json);
            fixed (byte* jsonUtf8Bytes = unpinnedJsonUtf8Bytes)
            {
                return NativeMethods.LoadConfigurationFromJson(unpinnedJsonUtf8Bytes.Length, new IntPtr(jsonUtf8Bytes), config, apiStatus);
            }
        }

        public static bool TryLoadConfigurationFromJson(string json, out Configuration config, ApiStatus apiStatus = null)
        {
            config = new Configuration();

            int result = LoadConfigurationFromJson(json, config.DangerousGetHandle(), apiStatus.ToNativeHandleOrNullptrDangerous());
            return result == NativeMethods.SuccessStatus;
        }

        public static Configuration LoadConfigurationFromJson(string json)
        {
            using (ApiStatus apiStatus = new ApiStatus())
            {
                if (TryLoadConfigurationFromJson(json, out Configuration config, apiStatus))
                {
                    return config;
                }

                throw new RLException(apiStatus);
            }
        }

        public string this[string key]
        {
            get
            {
                string result = ConfigurationGet(this.DangerousGetHandle(), key, string.Empty);

                GC.KeepAlive(this);
                return result;
            }
            set
            {
                ConfigurationSet(this.DangerousGetHandle(), key, value ?? string.Empty);
                if (key == "")

                GC.KeepAlive(this);
            }
        }
    }
}
