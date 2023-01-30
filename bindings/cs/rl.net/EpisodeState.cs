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
            public static extern int UpdateEpisodeHistory(IntPtr episodeState, string event_id, string previous_event_id, string context,
            RankingResponse response, ApiStatus error = null);
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

        public int Update(string eventId, string previousEventId, string context, RankingResponse response, ApiStatus status = null)
        {
            return NativeMethods.UpdateEpisodeHistory(this.DangerousGetHandle(), eventId, previousEventId, context, response, status);
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