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
            public static extern int GetSlotEntryActionId(IntPtr slotEntryResponse);

            [DllImport("rl.net.native.dll")]
            public static extern float GetSlotEntryProbability(IntPtr slotEntryResponse);

            [DllImport("rl.net.native.dll")]
            public static extern IntPtr CreateMultiSlotResponse();

            [DllImport("rl.net.native.dll")]
            public static extern void DeleteMultiSlotResponse(IntPtr multiSlotResponse);

            [DllImport("rl.net.native.dll", EntryPoint = "GetMultiSlotModelId")]
            private static extern IntPtr GetMultiSlotModelIdNative(IntPtr multiSlotResponse, int modelIdSize);

            internal static Func<IntPtr, IntPtr, int> GetMultiSlotModelIdOverride { get; set; }

            public static IntPtr GetMultiSlotModelId(IntPtr multiSlotResponse, out int modelIdSize)
            {
                if (GetMultiSlotModelIdOverride != null)
                {
                    return GetMultiSlotModelIdOverride(multiSlotResponse, modelIdSize);
                }

                return GetMultiSlotModelIdNative(multiSlotResponse, modelIdSize);
            }


            [DllImport("rl.net.native.dll", EntryPoint = "GetMultiSlotEventId")]
            private static extern IntPtr GetMultiSlotEventIdNative(IntPtr multiSlotResponse, int eventIdSize);

            internal static Func<IntPtr, IntPtr, int> GetMultiSlotEventIdOverride { get; set; }

            public static IntPtr GetMultiSlotEventId(IntPtr multiSlotResponse, out int eventIdSize)
            {
                if (GetMultiSlotEventIdOverride != null)
                {
                    return GetMultiSlotEventIdOverride(multiSlotResponse, eventIdSize);
                }

                return GetMultiSlotEventIdNative(multiSlotResponse, eventIdSize);
            }

            // TODO: CLS-compliance requires that we not publically expose unsigned types.
            // Probably not a big issue ("9e18 actions ought to be enough for anyone...")
            [DllImport("rl.net.native.dll")]
            public static extern UIntPtr GetMultiSlotSize(IntPtr decisionResponse);
        }
    }

    public sealed class SlotEntryResponse : NativeObject<SlotEntryResponse>
    {
        internal SlotEntryResponse(IntPtr sharedmultiSlotResponseHandle) : base(sharedmultiSlotResponseHandle, ownsHandle: false)
        {
        }

        public int ActionId
        {
            get
            {
                int result = NativeMethods.GetSlotEntryActionId(this.DangerousGetHandle());

                GC.KeepAlive(this);
                return result;
            }
        }

        public float Probability
        {
            get
            {
                float result = NativeMethods.GetSlotEntryProbability(this.DangerousGetHandle());

                GC.KeepAlive(this);
                return result;
            }
        }
    }

    public sealed class MultiSlotResponse : NativeObject<MultiSlotResponse>, IEnumerable<SlotEntryResponse>
    {
        public MultiSlotResponse() : base(new New<MultiSlotResponse>(NativeMethods.CreateMultiSlotResponse), new Delete<MultiSlotResponse>(NativeMethods.DeleteMultiSlotResponse))
        {
        }

        public string ModelId
        {
            get
            {
                int modelIdSize;
                IntPtr modelIdUtf8Ptr = NativeMethods.GetMultiSlotModelId(this.DangerousGetHandle(), modelIdSize);

                GC.KeepAlive(this);
                return NativeMethods.StringMarshallingFunc(modelIdUtf8Ptr, modelIdSize);
            }
        }


        public string EventId
        {
            get
            {
                int eventIdSize;
                IntPtr eventIdUtf8Ptr = NativeMethods.GetMultiSlotEventId(this.DangerousGetHandle(), eventIdSize);

                GC.KeepAlive(this);
                return NativeMethods.StringMarshallingFunc(eventIdUtf8Ptr, eventIdSize);
            }
        }

        public long Count
        {
            get
            {
                ulong unsignedSize = NativeMethods.GetMultiSlotSize(this.DangerousGetHandle()).ToUInt64();
                Debug.Assert(unsignedSize < Int64.MaxValue, "We do not support collections with size larger than _I64_MAX/Int64.MaxValue");

                GC.KeepAlive(this);
                return (long)unsignedSize;
            }
        }

        public IEnumerator<SlotEntryResponse> GetEnumerator()
        {
            return new MultiSlotResponseEnumerator(this);
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return this.GetEnumerator();
        }

        private class MultiSlotResponseEnumerator : NativeObject<MultiSlotResponseEnumerator>, IEnumerator<SlotEntryResponse>
        {
            [DllImport("rl.net.native.dll")]
            private static extern IntPtr CreateMultiSlotEnumeratorAdapter(IntPtr multiSlotResponse);

            private static New<MultiSlotResponseEnumerator> BindConstructorArguments(MultiSlotResponse multiSlotResponse)
            {
                return new New<MultiSlotResponseEnumerator>(() =>
                {
                    IntPtr result = CreateMultiSlotEnumeratorAdapter(multiSlotResponse.DangerousGetHandle());

                    GC.KeepAlive(multiSlotResponse); // Extend the lifetime of this handle because the delegate (and its data) is not stored on the heap.
                    return result;
                });
            }

            [DllImport("rl.net.native.dll")]
            private static extern void DeleteMultiSlotEnumeratorAdapter(IntPtr multiSlotEnumeratorAdapter);

            [DllImport("rl.net.native.dll")]
            private static extern int MultiSlotEnumeratorInit(IntPtr multiSlotEnumeratorAdapter);

            [DllImport("rl.net.native.dll")]
            private static extern int MultiSlotEnumeratorMoveNext(IntPtr multiSlotEnumeratorAdapter);

            [DllImport("rl.net.native.dll")]
            private static extern IntPtr GetMultiSlotEnumeratorCurrent(IntPtr multiSlotEnumeratorAdapter);

            private bool initialState = true;

            public MultiSlotResponseEnumerator(MultiSlotResponse multiSlotResponse) : base(BindConstructorArguments(multiSlotResponse), new Delete<MultiSlotResponseEnumerator>(DeleteMultiSlotEnumeratorAdapter))
            {
            }

            public SlotEntryResponse Current
            {
                get
                {
                    IntPtr sharedmultiSlotResponseHandle = GetMultiSlotEnumeratorCurrent(this.DangerousGetHandle());

                    GC.KeepAlive(this);
                    return new SlotEntryResponse(sharedmultiSlotResponseHandle);
                }
            }

            object System.Collections.IEnumerator.Current => this.Current;

            public bool MoveNext()
            {
                int result;
                if (this.initialState)
                {
                    this.initialState = false;
                    result = MultiSlotEnumeratorInit(this.DangerousGetHandle());
                }
                else
                {
                    result = MultiSlotEnumeratorMoveNext(this.DangerousGetHandle());
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