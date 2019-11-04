using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Runtime.InteropServices;

using Rl.Net.Native;
using System.Collections;

namespace Rl.Net {
    namespace Native
    {
        internal partial class NativeMethods
        {
            [DllImport("rl.net.native.dll")]
            public static extern IntPtr CreateRankingResponse();

            [DllImport("rl.net.native.dll")]
            public static extern void DeleteRankingResponse(IntPtr rankingResponse);

            [DllImport("rl.net.native.dll", EntryPoint = "GetRankingEventId")]
            private static extern IntPtr GetRankingEventIdNative(IntPtr rankingResponse);

            internal static Func<IntPtr, IntPtr> GetRankingEventIdOverride { get; set; }

            public static IntPtr GetRankingEventId(IntPtr rankingResponse)
            {
                if (GetRankingEventIdOverride != null)
                {
                    return GetRankingEventIdOverride(rankingResponse);
                }

                return GetRankingEventIdNative(rankingResponse);
            }

            [DllImport("rl.net.native.dll", EntryPoint = "GetRankingModelId")]
            private static extern IntPtr GetRankingModelIdNative(IntPtr rankingResponse);

            internal static Func<IntPtr, IntPtr> GetRankingModelIdOverride { get; set; }

            public static IntPtr GetRankingModelId(IntPtr rankingResponse)
            {
                if (GetRankingModelIdOverride != null)
                {
                    return GetRankingModelIdOverride(rankingResponse);
                }

                return GetRankingModelIdNative(rankingResponse);
            }

            // TODO: CLS-compliance requires that we not publically expose unsigned types.
            // Probably not a big issue ("9e18 actions ought to be enough for anyone...")
            [DllImport("rl.net.native.dll")]
            public static extern UIntPtr GetRankingActionCount(IntPtr rankingResponse);

            [DllImport("rl.net.native.dll")]
            public static extern int GetRankingChosenAction(IntPtr rankingResponse, out UIntPtr action_id, IntPtr status);

            [DllImport("rl.net.native.dll")]
            public static extern void SetRankingEventId(IntPtr rankingResponse, IntPtr eventId);

            [DllImport("rl.net.native.dll")]
            public static extern void SetRankingModelId(IntPtr rankingResponse, IntPtr modelId);

            [DllImport("rl.net.native.dll")]
            public static extern void SetRankingChosenAction(IntPtr rankingResponse, int chosenAction);

            [DllImport("rl.net.native.dll")]
            public static extern void PushActionProbability(IntPtr rankingResponse, int action, float prob);
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct ActionProbability
    {
        private UIntPtr actionIndex;
        private float probability;

        public long ActionIndex => (long)this.actionIndex.ToUInt64();

        public float Probability => this.probability;
    }

    public sealed class RankingResponse : NativeObject<RankingResponse>, IEnumerable<ActionProbability>
    {
        public RankingResponse() : base(new New<RankingResponse>(NativeMethods.CreateRankingResponse), new Delete<RankingResponse>(NativeMethods.DeleteRankingResponse))
        {
        }

        public static RankingResponse CreateRankingResponse(string eventId, string modelId, int chosenAction, int[] actions, float[] probabilities)
        {
            var result = new RankingResponse();
            RankingResponseSetEventId(result.NativeHandle, eventId);
            RankingResponseSetModelId(result.NativeHandle, modelId);
            NativeMethods.SetRankingChosenAction(result.NativeHandle, chosenAction);
            RankingResponseSetPmf(result.NativeHandle, actions, probabilities);
            return result;
        }

        public string EventId
        {
            get
            {
                IntPtr eventIdUtf8Ptr = NativeMethods.GetRankingEventId(this.NativeHandle);

                return NativeMethods.StringMarshallingFunc(eventIdUtf8Ptr);
            }
        }

        public string ModelId
        {
            get
            {
                IntPtr modelIdUtf8Ptr = NativeMethods.GetRankingModelId(this.NativeHandle);

                return NativeMethods.StringMarshallingFunc(modelIdUtf8Ptr);
            }
        }

        public long Count
        {
            get
            {
                ulong unsignedSize = NativeMethods.GetRankingActionCount(this.NativeHandle).ToUInt64();
                Debug.Assert(unsignedSize < Int64.MaxValue, "We do not support collections with size larger than _I64_MAX/Int64.MaxValue");
    
                return (long)unsignedSize;
            }
        }

        // TODO: Why does this method call, which seems like a "get" of a value, have an API status?
        public bool TryGetChosenAction(out long actionIndex, ApiStatus status = null)
        {
            actionIndex = -1;
            UIntPtr chosenAction;
            int result = NativeMethods.GetRankingChosenAction(this.NativeHandle, out chosenAction, status.ToNativeHandleOrNullptr());

            if (result != NativeMethods.SuccessStatus)
            {
                return false;
            }

            actionIndex = (long)(chosenAction.ToUInt64());
            return true;
        }

        public long ChosenAction
        {
            get
            {
                ApiStatus apiStatus = new ApiStatus();

                long result;
                if (!this.TryGetChosenAction(out result, apiStatus))
                {
                    throw new RLException(apiStatus);
                }

                return result;
            }
        }

        public IEnumerator<ActionProbability> GetEnumerator()
        {
            return new RankingResponseEnumerator(this);
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return this.GetEnumerator();
        }

        unsafe private static void RankingResponseSetEventId(IntPtr rankingResponse, string eventId)
        {
            fixed (byte* eventIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(eventId))
            {
                NativeMethods.SetRankingEventId(rankingResponse, new IntPtr(eventIdUtf8Bytes));
            }
        }

        unsafe private static void RankingResponseSetModelId(IntPtr rankingResponse, string modelId)
        {
            fixed (byte* modelIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(modelId))
            {
                NativeMethods.SetRankingModelId(rankingResponse, new IntPtr(modelIdUtf8Bytes));
            }
        }

        private static void RankingResponseSetPmf(IntPtr rankingResponse, int[] actions, float[] probabilities)
        {
            for (int i = 0; i < actions.Length; ++i)
            {
                NativeMethods.PushActionProbability(rankingResponse, actions[i], probabilities[i]);
            }
        }

        private class RankingResponseEnumerator : NativeObject<RankingResponseEnumerator>, IEnumerator<ActionProbability>
        {
            [DllImport("rl.net.native.dll")]
            private static extern IntPtr CreateRankingEnumeratorAdapter(IntPtr rankingResponse);

            private static New<RankingResponseEnumerator> BindConstructorArguments(RankingResponse rankingResponse)
            {
                return new New<RankingResponseEnumerator>(() => CreateRankingEnumeratorAdapter(rankingResponse.NativeHandle));
            }

            [DllImport("rl.net.native.dll")]
            private static extern void DeleteRankingEnumeratorAdapter(IntPtr rankingEnumeratorAdapter);

            [DllImport("rl.net.native.dll")]
            private static extern int RankingEnumeratorInit(IntPtr rankingEnumeratorAdapter);

            [DllImport("rl.net.native.dll")]
            private static extern int RankingEnumeratorMoveNext(IntPtr rankingEnumeratorAdapter);
        
            [DllImport("rl.net.native.dll")]
            private static extern ActionProbability GetRankingEnumeratorCurrent(IntPtr rankingEnumeratorAdapter);

            private bool initialState = true;

            public RankingResponseEnumerator(RankingResponse rankingResponse) : base(BindConstructorArguments(rankingResponse), new Delete<RankingResponseEnumerator>(DeleteRankingEnumeratorAdapter))
            {
            }

            public ActionProbability Current
            {
                get
                {
                    return GetRankingEnumeratorCurrent(this.NativeHandle);
                }
            }

            object System.Collections.IEnumerator.Current => this.Current;

            public bool MoveNext()
            {
                int result;
                if (this.initialState)
                {
                    this.initialState = false;
                    result = RankingEnumeratorInit(this.NativeHandle);
                }
                else
                {
                    result = RankingEnumeratorMoveNext(this.NativeHandle);
                }

                // The contract of result is to return 1 if true, 0 if false.
                return result == 1;
            }

            public void Reset()
            {
                throw new NotSupportedException();
            }
        }
    }
}