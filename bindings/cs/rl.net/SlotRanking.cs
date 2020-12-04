using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Runtime.InteropServices;

using Rl.Net.Native;
using System.Collections;

namespace Rl.Net
{
    namespace Native
    {
        internal partial class NativeMethods
        {
            [DllImport("rl.net.native.dll")]
            public static extern IntPtr CreateSlotRanking();

            [DllImport("rl.net.native.dll")]
            public static extern void DeleteSlotRanking(IntPtr slot);

            [DllImport("rl.net.native.dll", EntryPoint = "GetSlotId")]
            private static extern IntPtr GetSlotIdNative(IntPtr slotRanking);

            internal static Func<IntPtr, IntPtr> GetSlotIdOverride { get; set; }

            public static IntPtr GetSlotId(IntPtr slotRanking)
            {
                if (GetSlotIdOverride != null)
                {
                    return GetSlotIdOverride(slotRanking);
                }

                return GetSlotIdNative(slotRanking);
            }

            [DllImport("rl.net.native.dll")]
            public static extern UIntPtr GetSlotActionCount(IntPtr slotRanking);

            [DllImport("rl.net.native.dll")]
            public static extern int GetSlotChosenAction(IntPtr slotRanking, out UIntPtr action_id, IntPtr status);
        }
    }

    public sealed class SlotRanking : NativeObject<SlotRanking>, IEnumerable<ActionProbability>
    {
        public SlotRanking() : base(new New<SlotRanking>(NativeMethods.CreateSlotRanking), new Delete<SlotRanking>(NativeMethods.DeleteSlotRanking))
        {
        }

        internal SlotRanking(IntPtr sharedSlotRankingHandle) : base(sharedSlotRankingHandle, ownsHandle: false)
        {
        }

        public string EventId
        {
            get
            {
                IntPtr eventIdUtf8Ptr = NativeMethods.GetSlotId(this.DangerousGetHandle());

                string result = NativeMethods.StringMarshallingFunc(eventIdUtf8Ptr);

                GC.KeepAlive(this);
                return result;
            }
        }

        public long Count
        {
            get
            {
                ulong unsignedSize = NativeMethods.GetSlotActionCount(this.DangerousGetHandle()).ToUInt64();
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
            int result = NativeMethods.GetSlotChosenAction(this.DangerousGetHandle(), out chosenAction, status.ToNativeHandleOrNullptrDangerous());

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
                if (!TryGetChosenAction(out result, apiStatus))
                {
                    throw new RLException(apiStatus);
                }

                return result;
            }
        }

        public IEnumerator<ActionProbability> GetEnumerator()
        {
            return new SlotRankingEnumerator(this);
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return this.GetEnumerator();
        }

        private class SlotRankingEnumerator : NativeObject<SlotRankingEnumerator>, IEnumerator<ActionProbability>
        {
            [DllImport("rl.net.native.dll")]
            private static extern IntPtr CreateSlotEnumeratorAdapter(IntPtr slotResponse);

            private static New<SlotRankingEnumerator> BindConstructorArguments(SlotRanking slotRanking)
            {
                return new New<SlotRankingEnumerator>(() =>
                {
                    IntPtr result = CreateSlotEnumeratorAdapter(slotRanking.DangerousGetHandle());

                    GC.KeepAlive(slotRanking); // Extend the lifetime of this handle because the delegate (and its data) is not stored on the heap.
                    return result;
                });
            }

            [DllImport("rl.net.native.dll")]
            private static extern void DeleteSlotEnumeratorAdapter(IntPtr slotEnumeratorAdapter);

            [DllImport("rl.net.native.dll")]
            private static extern int SlotEnumeratorInit(IntPtr slotEnumeratorAdapter);

            [DllImport("rl.net.native.dll")]
            private static extern int SlotEnumeratorMoveNext(IntPtr slotEnumeratorAdapter);

            [DllImport("rl.net.native.dll")]
            private static extern ActionProbability GetSlotEnumeratorCurrent(IntPtr slotEnumeratorAdapter);

            private bool initialState = true;

            public SlotRankingEnumerator(SlotRanking slotRanking) : base(BindConstructorArguments(slotRanking), new Delete<SlotRankingEnumerator>(DeleteSlotEnumeratorAdapter))
            {
            }

            public ActionProbability Current
            {
                get
                {
                    ActionProbability result = GetSlotEnumeratorCurrent(this.DangerousGetHandle());

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
                    result = SlotEnumeratorInit(this.DangerousGetHandle());
                }
                else
                {
                    result = SlotEnumeratorMoveNext(this.DangerousGetHandle());
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

