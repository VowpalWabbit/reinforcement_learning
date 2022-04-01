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
            private static extern IntPtr GetRankingEventIdNative(IntPtr rankingResponse, out int eventIdSize);

            internal static Func<IntPtr, IntPtr> GetRankingEventIdOverride { get; set; }

            public static IntPtr GetRankingEventId(IntPtr rankingResponse, out int eventIdSize)
            {
                eventIdSize = 0;
                if (GetRankingEventIdOverride != null)
                {
                    IntPtr eventId = GetRankingEventIdOverride(rankingResponse);
                    string marshalledBack = NativeMethods.StringMarshallingFunc(eventId);
                    eventIdSize = NativeMethods.StringEncoding.GetByteCount(marshalledBack);
                    return eventId;
                }

                return GetRankingEventIdNative(rankingResponse, out eventIdSize);
            }

            [DllImport("rl.net.native.dll", EntryPoint = "GetRankingModelId")]
            private static extern IntPtr GetRankingModelIdNative(IntPtr rankingResponse, out int modelIdSize);

            internal static Func<IntPtr, IntPtr> GetRankingModelIdOverride { get; set; }

            public static IntPtr GetRankingModelId(IntPtr rankingResponse, out int modelIdSize)
            {
                modelIdSize = 0;
                if (GetRankingModelIdOverride != null)
                {
                    IntPtr modelId = GetRankingModelIdOverride(rankingResponse);
                    string marshalledBack = NativeMethods.StringMarshallingFunc(modelId);
                    modelIdSize = NativeMethods.StringEncoding.GetByteCount(marshalledBack);
                    return modelId;
                }

                return GetRankingModelIdNative(rankingResponse, out modelIdSize);
            }

            // TODO: CLS-compliance requires that we not publically expose unsigned types.
            // Probably not a big issue ("9e18 actions ought to be enough for anyone...")
            [DllImport("rl.net.native.dll")]
            public static extern UIntPtr GetRankingActionCount(IntPtr rankingResponse);

            [DllImport("rl.net.native.dll")]
            public static extern int GetRankingChosenAction(IntPtr rankingResponse, out UIntPtr action_id, IntPtr status);
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

        internal RankingResponse(IntPtr sharedRankingResponseHandle) : base(sharedRankingResponseHandle, ownsHandle: false)
        {
        }

        public string EventId
        {
            get
            {
                int eventIdSize = 0;
                IntPtr eventIdUtf8Ptr = NativeMethods.GetRankingEventId(this.DangerousGetHandle(), out eventIdSize);

                string result = NativeMethods.StringMarshallingFuncWithSize(eventIdUtf8Ptr, eventIdSize);

                GC.KeepAlive(this);
                return result;
            }
        }

        public string ModelId
        {
            get
            {
                int modelIdSize = 0;
                IntPtr modelIdUtf8Ptr = NativeMethods.GetRankingModelId(this.DangerousGetHandle(), out modelIdSize);

                string result = NativeMethods.StringMarshallingFuncWithSize(modelIdUtf8Ptr, modelIdSize);

                GC.KeepAlive(this);
                return result;
            }
        }

        public long Count
        {
            get
            {
                ulong unsignedSize = NativeMethods.GetRankingActionCount(this.DangerousGetHandle()).ToUInt64();
                Debug.Assert(unsignedSize < Int64.MaxValue, "We do not support collections with size larger than _I64_MAX/Int64.MaxValue");
    
                GC.KeepAlive(this);
                return (long)unsignedSize;
            }
        }

        // TODO: Why does this method call, which seems like a "get" of a value, have an API status?
        public bool TryGetChosenAction(out long actionIndex, ApiStatus status = null)
        {
            actionIndex = -1;
            UIntPtr chosenAction;
            int result = NativeMethods.GetRankingChosenAction(this.DangerousGetHandle(), out chosenAction, status.ToNativeHandleOrNullptrDangerous());

            bool success = (result == NativeMethods.SuccessStatus);
            if (success)
            {
                actionIndex = (long)(chosenAction.ToUInt64());
            }

            GC.KeepAlive(this);
            return success;
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

        private class RankingResponseEnumerator : NativeObject<RankingResponseEnumerator>, IEnumerator<ActionProbability>
        {
            [DllImport("rl.net.native.dll")]
            private static extern IntPtr CreateRankingEnumeratorAdapter(IntPtr rankingResponse);

            private static New<RankingResponseEnumerator> BindConstructorArguments(RankingResponse rankingResponse)
            {
                return new New<RankingResponseEnumerator>(() => 
                {
                    IntPtr result = CreateRankingEnumeratorAdapter(rankingResponse.DangerousGetHandle());

                    GC.KeepAlive(rankingResponse); // Extend the lifetime of this handle because the delegate (and its data) is not stored on the heap.
                    return result;
                });
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
                    ActionProbability result = GetRankingEnumeratorCurrent(this.DangerousGetHandle());

                    GC.KeepAlive(this);
                    return result;
                }
            }

            object System.Collections.IEnumerator.Current => this.Current;

            public bool MoveNext()
            {
                int result;
                if (this.initialState)
                {
                    this.initialState = false;
                    result = RankingEnumeratorInit(this.DangerousGetHandle());
                }
                else
                {
                    result = RankingEnumeratorMoveNext(this.DangerousGetHandle());
                }

                // The contract of result is to return 1 if true, 0 if false.
                GC.KeepAlive(this);
                return result == 1;
            }

            public void Reset()
            {
                throw new NotSupportedException();
            }
        }
    }
}