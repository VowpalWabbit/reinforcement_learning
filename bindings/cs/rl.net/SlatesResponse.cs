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
            public static extern int GetSlatesSlotSlotId(IntPtr slatesSlotResponse);

            [DllImport("rl.net.native.dll")]
            public static extern int GetSlatesSlotActionId(IntPtr slatesSlotResponse);

            [DllImport("rl.net.native.dll")]
            public static extern float GetSlatesSlotProbability(IntPtr slatesSlotResponse);

            [DllImport("rl.net.native.dll")]
            public static extern IntPtr CreateSlatesResponse();

            [DllImport("rl.net.native.dll")]
            public static extern void DeleteSlatesResponse(IntPtr slatesResponse);

            [DllImport("rl.net.native.dll", EntryPoint = "GetSlatesModelId")]
            private static extern IntPtr GetSlatesModelIdNative(IntPtr slatesResponse);

            internal static Func<IntPtr, IntPtr> GetSlatesModelIdOverride { get; set; }

            public static IntPtr GetSlatesModelId(IntPtr slatesResponse)
            {
                if (GetSlatesModelIdOverride != null)
                {
                    return GetSlatesModelIdOverride(slatesResponse);
                }

                return GetSlatesModelIdNative(slatesResponse);
            }


            [DllImport("rl.net.native.dll", EntryPoint = "GetSlatesEventId")]
            private static extern IntPtr GetSlatesEventIdNative(IntPtr slatesResponse);

            internal static Func<IntPtr, IntPtr> GetSlatesEventIdOverride { get; set; }

            public static IntPtr GetSlatesEventId(IntPtr slatesResponse)
            {
                if (GetSlatesEventIdOverride != null)
                {
                    return GetSlatesEventIdOverride(slatesResponse);
                }

                return GetSlatesEventIdNative(slatesResponse);
            }

            // TODO: CLS-compliance requires that we not publically expose unsigned types.
            // Probably not a big issue ("9e18 actions ought to be enough for anyone...")
            [DllImport("rl.net.native.dll")]
            public static extern UIntPtr GetSlatesSize(IntPtr decisionResponse);
        }
    }

    public sealed class SlatesSlotResponse : NativeObject<SlatesSlotResponse>
    {
        internal SlatesSlotResponse(IntPtr sharedSlatesResponseHandle) : base(sharedSlatesResponseHandle, ownsHandle: false)
        {
        }

        public int SlotId
        {
            get
            {
                int result = NativeMethods.GetSlatesSlotSlotId(this.DangerousGetHandle());

                GC.KeepAlive(this);
                return result;
            }
        }

        public int ActionId
        {
            get
            {
                int result = NativeMethods.GetSlatesSlotActionId(this.DangerousGetHandle());

                GC.KeepAlive(this);
                return result;
            }
        }

        public float Probability
        {
            get
            {
                float result = NativeMethods.GetSlatesSlotProbability(this.DangerousGetHandle());

                GC.KeepAlive(this);
                return result;
            }
        }
    }

    public sealed class SlatesResponse : NativeObject<SlatesResponse>, IEnumerable<SlatesSlotResponse>
    {
        public SlatesResponse() : base(new New<SlatesResponse>(NativeMethods.CreateSlatesResponse), new Delete<SlatesResponse>(NativeMethods.DeleteSlatesResponse))
        {
        }

        public string ModelId
        {
            get
            {
                IntPtr modelIdUtf8Ptr = NativeMethods.GetSlatesModelId(this.DangerousGetHandle());

                GC.KeepAlive(this);
                return NativeMethods.StringMarshallingFunc(modelIdUtf8Ptr);
            }
        }


        public string EventId
        {
            get
            {
                IntPtr eventIdUtf8Ptr = NativeMethods.GetSlatesEventId(this.DangerousGetHandle());

                GC.KeepAlive(this);
                return NativeMethods.StringMarshallingFunc(eventIdUtf8Ptr);
            }
        }

        public long Count
        {
            get
            {
                ulong unsignedSize = NativeMethods.GetSlatesSize(this.DangerousGetHandle()).ToUInt64();
                Debug.Assert(unsignedSize < Int64.MaxValue, "We do not support collections with size larger than _I64_MAX/Int64.MaxValue");
    
                GC.KeepAlive(this);
                return (long)unsignedSize;
            }
        }

        public IEnumerator<SlatesSlotResponse> GetEnumerator()
        {
            return new SlatesResponseEnumerator(this);
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return this.GetEnumerator();
        }

        private class SlatesResponseEnumerator : NativeObject<SlatesResponseEnumerator>, IEnumerator<SlatesSlotResponse>
        {
            [DllImport("rl.net.native.dll")]
            private static extern IntPtr CreateSlatesEnumeratorAdapter(IntPtr slatesResponse);

            private static New<SlatesResponseEnumerator> BindConstructorArguments(SlatesResponse slatesResponse)
            {
                return new New<SlatesResponseEnumerator>(() => 
                {
                    IntPtr result = CreateSlatesEnumeratorAdapter(slatesResponse.DangerousGetHandle());

                    GC.KeepAlive(slatesResponse); // Extend the lifetime of this handle because the delegate (and its data) is not stored on the heap.
                    return result;
                    
                });
            }

            [DllImport("rl.net.native.dll")]
            private static extern void DeleteSlatesEnumeratorAdapter(IntPtr slatesEnumeratorAdapter);

            [DllImport("rl.net.native.dll")]
            private static extern int SlatesEnumeratorInit(IntPtr slatesEnumeratorAdapter);

            [DllImport("rl.net.native.dll")]
            private static extern int SlatesEnumeratorMoveNext(IntPtr slatesEnumeratorAdapter);
        
            [DllImport("rl.net.native.dll")]
            private static extern IntPtr GetSlatesEnumeratorCurrent(IntPtr slatesEnumeratorAdapter);

            private bool initialState = true;

            public SlatesResponseEnumerator(SlatesResponse slatesResponse) : base(BindConstructorArguments(slatesResponse), new Delete<SlatesResponseEnumerator>(DeleteSlatesEnumeratorAdapter))
            {
            }

            public SlatesSlotResponse Current
            {
                get
                {
                    IntPtr sharedSlatesResponseHandle = GetSlatesEnumeratorCurrent(this.DangerousGetHandle());

                    GC.KeepAlive(this);
                    return new SlatesSlotResponse(sharedSlatesResponseHandle);
                }
            }

            object System.Collections.IEnumerator.Current => this.Current;

            public bool MoveNext()
            {
                int result;
                if (this.initialState)
                {
                    this.initialState = false;
                    result = SlatesEnumeratorInit(this.DangerousGetHandle());
                }
                else
                {
                    result = SlatesEnumeratorMoveNext(this.DangerousGetHandle());
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