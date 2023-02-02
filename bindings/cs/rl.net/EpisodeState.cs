using System;
using System.Runtime.InteropServices;

using Rl.Net.Native;

namespace Rl.Net
{
    namespace Native
    {
        internal partial class NativeMethods
        {
            [DllImport("rlnetnative")]
            public static extern IntPtr CreateEpisodeState(IntPtr episodeId);

            [DllImport("rlnetnative")]
            public static extern void DeleteEpisodeState(IntPtr episodeState);

            [DllImport("rlnetnative")]
            public static extern IntPtr GetEpisodeId(IntPtr episodeState);

            [DllImport("rlnetnative")]
            public static extern int UpdateEpisodeHistory(IntPtr episodeState, IntPtr eventId, IntPtr previousEventId, IntPtr context, IntPtr rankingResponse, IntPtr apiStatus);
        }
    }

    public sealed class EpisodeState : NativeObject<EpisodeState>
    {
        public EpisodeState(string episodeId) : base(BindConstructorArguments(episodeId), new Delete<EpisodeState>(NativeMethods.DeleteEpisodeState))
        {
        }

        internal EpisodeState(IntPtr sharedEpisodeStateHandle) : base(sharedEpisodeStateHandle, ownsHandle: false)
        {
        }

        private static New<EpisodeState> BindConstructorArguments(string episodeId)
        {
            return new New<EpisodeState>(() =>
            {
                unsafe
                {
                    fixed (byte* episodeIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(episodeId))
                    {
                        IntPtr contextJsonUtf8Ptr = new IntPtr(episodeIdUtf8Bytes);
                        IntPtr result = NativeMethods.CreateEpisodeState(contextJsonUtf8Ptr);
                        GC.KeepAlive(episodeId);
                        return result;
                    }
                }
            });
        }

        public int Update(string eventId, string previousEventId, string context, RankingResponse resp, ApiStatus status = null)
        {
            if (string.IsNullOrEmpty(eventId))
            {
                throw new ArgumentException("EventId cannot be null or empty", "eventId");
            }

            unsafe
            {
                fixed (byte* contextJsonUtf8Bytes = NativeMethods.StringEncoding.GetBytes(context))
                fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
                {
                    if (previousEventId == null)
                    {
                        return NativeMethods.UpdateEpisodeHistory(this.DangerousGetHandle(), new IntPtr(eventIdUtf8Bytes), IntPtr.Zero, new IntPtr(contextJsonUtf8Bytes), resp.DangerousGetHandle(),
                                status == null ? IntPtr.Zero : status.DangerousGetHandle());
                    }
                    else
                    {
                        fixed (byte* previousEventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(previousEventId))
                        {
                            return NativeMethods.UpdateEpisodeHistory(this.DangerousGetHandle(), new IntPtr(eventIdUtf8Bytes), new IntPtr(previousEventIdUtf8Bytes), new IntPtr(contextJsonUtf8Bytes), resp.DangerousGetHandle(),
                                status == null ? IntPtr.Zero : status.DangerousGetHandle());
                        }
                    }
                }
            }
        }

        public string EpisodeId
        {
            get
            {
                IntPtr episodeIdUtf8Ptr = NativeMethods.GetEpisodeId(this.DangerousGetHandle());
                GC.KeepAlive(this);
                return NativeMethods.StringMarshallingFunc(episodeIdUtf8Ptr);
            }
        }
    }
}