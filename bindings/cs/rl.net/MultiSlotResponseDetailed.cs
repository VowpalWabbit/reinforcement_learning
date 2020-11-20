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
            public static extern IntPtr CreateMultiSlotResponseDetailed();

            [DllImport("rl.net.native.dll")]
            public static extern void DeleteMultiSlotResponseDetailed(IntPtr multiSlotResponseDetailed);

            [DllImport("rl.net.native.dll", EntryPoint = "GetMultiSlotDetailedModelId")]
            private static extern IntPtr GetMultiSlotDetailedModelIdNative(IntPtr multiSlotResponseDetailed);

            internal static Func<IntPtr, IntPtr> GetMultiSlotDetailedModelIdOverride { get; set; }

            public static IntPtr GetMultiSlotDetailedModelId(IntPtr multiSlotResponseDetailed)
            {
                if (GetMultiSlotDetailedModelIdOverride != null)
                {
                    return GetMultiSlotDetailedModelIdOverride(multiSlotResponseDetailed);
                }

                return GetMultiSlotDetailedModelIdNative(multiSlotResponseDetailed);
            }

            [DllImport("rl.net.native.dll", EntryPoint = "GetMultiSlotDetailedEventID")]
            private static extern IntPtr GetMultiSlotDetailedEventIDNative(IntPtr multiSlotResponseDetailed);

            internal static Func<IntPtr, IntPtr> GetMultiSlotDetailedEventIdOverride { get; set; }

            public static IntPtr GetMultiSlotDetailedEventId(IntPtr multiSlotResponseDetailed)
            {
                if (GetMultiSlotDetailedEventIdOverride != null)
                {
                    return GetMultiSlotDetailedEventIdOverride(multiSlotResponseDetailed);
                }

                return GetMultiSlotEventIdNative(multiSlotResponseDetailed);
            }

            // TODO: CLS-compliance requires that we not publically expose unsigned types.
            // Probably not a big issue ("9e18 actions ought to be enough for anyone...")
            [DllImport("rl.net.native.dll")]
            public static extern UIntPtr GetMultiSlotDetailedSize(IntPtr multiSlot);
        }
    }

    public sealed class MultiSlotResponseDetailed : NativeObject<MultiSlotResponseDetailed>, IEnumerable<SlotRanking>
    {
        public MultiSlotResponseDetailed() : base(new New<MultiSlotResponseDetailed>(NativeMethods.CreateMultiSlotResponseDetailed), new Delete<MultiSlotResponseDetailed>(NativeMethods.DeleteMultiSlotResponseDetailed))
        {
        }

        public string ModelId
        {
            get
            {
                IntPtr modelIdUtf8Ptr = NativeMethods.GetMultiSlotDetailedModelId(this.DangerousGetHandle());

                GC.KeepAlive(this);
                return NativeMethods.StringMarshallingFunc(modelIdUtf8Ptr);
            }
        }


        public string EventId
        {
            get
            {
                IntPtr eventIdUtf8Ptr = NativeMethods.GetMultiSlotDetailedEventId(this.DangerousGetHandle());

                GC.KeepAlive(this);
                return NativeMethods.StringMarshallingFunc(eventIdUtf8Ptr);
            }
        }

        public long Count
        {
            get
            {
                ulong unsignedSize = NativeMethods.GetMultiSlotDetailedSize(this.DangerousGetHandle()).ToUInt64();
                Debug.Assert(unsignedSize < Int64.MaxValue, "We do not support collections with size larger than _I64_MAX/Int64.MaxValue");

                GC.KeepAlive(this);
                return (long)unsignedSize;
            }
        }

        public IEnumerator<SlotRanking> GetEnumerator()
        {
            return new MultiSlotResponseDetailedEnumerator(this);
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return this.GetEnumerator();
        }

        private class MultiSlotResponseDetailedEnumerator : NativeObject<MultiSlotResponseDetailedEnumerator>, IEnumerator<SlotRanking>
        {
            [DllImport("rl.net.native.dll")]
            private static extern IntPtr CreateMultiSlotDetailedEnumeratorAdapter(IntPtr multiSlotResponseDetailed);

            private static New<MultiSlotResponseDetailedEnumerator> BindConstructorArguments(MultiSlotResponseDetailed multiSlotResponseDetailed)
            {
                return new New<MultiSlotResponseDetailedEnumerator>(() =>
                {
                    IntPtr result = CreateMultiSlotDetailedEnumeratorAdapter(multiSlotResponseDetailed.DangerousGetHandle());

                    GC.KeepAlive(multiSlotResponseDetailed); // Extend the lifetime of this handle because the delegate (and its data) is not stored on the heap.
                    return result;

                });
            }

            [DllImport("rl.net.native.dll")]
            private static extern void DeleteMultiSlotDetailedEnumeratorAdapter(IntPtr multiSlotDetailedEnumeratorAdapter);

            [DllImport("rl.net.native.dll")]
            private static extern int MultiSlotDetailedEnumeratorInit(IntPtr multiSlotDetailedEnumeratorAdapter);

            [DllImport("rl.net.native.dll")]
            private static extern int MultiSlotDetailedEnumeratorMoveNext(IntPtr multiSlotDetailedEnumeratorAdapter);

            [DllImport("rl.net.native.dll")]
            private static extern IntPtr GetMultiSlotDetailedEnumeratorCurrent(IntPtr multiSlotDetailedEnumeratorAdapter);

            private bool initialState = true;

            public MultiSlotResponseDetailedEnumerator(MultiSlotResponseDetailed multiSlotResponseDetailed) : base(BindConstructorArguments(multiSlotResponseDetailed), new Delete<MultiSlotResponseDetailedEnumerator>(DeleteMultiSlotDetailedEnumeratorAdapter))
            {
            }

            public SlotRanking Current
            {
                get
                {
                    IntPtr sharedmultiSlotResponseHandle = GetMultiSlotDetailedEnumeratorCurrent(this.DangerousGetHandle());

                    GC.KeepAlive(this);
                    return new SlotRanking(sharedmultiSlotResponseHandle);
                }
            }

            object System.Collections.IEnumerator.Current => this.Current;

            public bool MoveNext()
            {
                int result;
                if (this.initialState)
                {
                    this.initialState = false;
                    result = MultiSlotDetailedEnumeratorInit(this.DangerousGetHandle());
                }
                else
                {
                    result = MultiSlotDetailedEnumeratorMoveNext(this.DangerousGetHandle());
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