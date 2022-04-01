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
            public static extern IntPtr CreateSlotResponse();

            [DllImport("rl.net.native.dll")]
            public static extern IntPtr GetSlotSlotId(IntPtr slotResponse, int slotIdSize);

            [DllImport("rl.net.native.dll")]
            public static extern int GetSlotActionId(IntPtr slotResponse);

            [DllImport("rl.net.native.dll")]
            public static extern float GetSlotProbability(IntPtr slotResponse);

            [DllImport("rl.net.native.dll")]
            public static extern IntPtr CreateDecisionResponse();

            [DllImport("rl.net.native.dll")]
            public static extern void DeleteDecisionResponse(IntPtr decisionResponse);

            [DllImport("rl.net.native.dll", EntryPoint = "GetDecisionModelId")]
            private static extern IntPtr GetDecisionModelIdNative(IntPtr decisionResponse);

            internal static Func<IntPtr, IntPtr, int> GetDecisionModelIdOverride { get; set; }

            public static IntPtr GetDecisionModelId(IntPtr decisionResponse, out int modelIdSize)
            {
                if (GetDecisionModelIdOverride != null)
                {
                    return GetDecisionModelIdOverride(decisionResponse, modelIdSize);
                }

                return GetDecisionModelIdNative(decisionResponse, modelIdSize);
            }

            // TODO: CLS-compliance requires that we not publically expose unsigned types.
            // Probably not a big issue ("9e18 actions ought to be enough for anyone...")
            [DllImport("rl.net.native.dll")]
            public static extern UIntPtr GetDecisionSize(IntPtr decisionResponse);
        }
    }

    public sealed class SlotResponse : NativeObject<SlotResponse>
    {
        internal SlotResponse(IntPtr sharedRankingResponseHandle) : base(sharedRankingResponseHandle, ownsHandle: false)
        {
        }

        public string SlotId
        {
            get
            {
                int slotIdSize;
                IntPtr modelIdUtf8Ptr = NativeMethods.GetSlotSlotId(this.DangerousGetHandle(), slotIdSize);

                GC.KeepAlive(this);
                return NativeMethods.StringMarshallingFunc(modelIdUtf8Ptr, slotIdSize);
            }
        }

        public int ActionId
        {
            get
            {
                int result = NativeMethods.GetSlotActionId(this.DangerousGetHandle());

                GC.KeepAlive(this);
                return result;
            }
        }

        public float Probability
        {
            get
            {
                float result = NativeMethods.GetSlotProbability(this.DangerousGetHandle());

                GC.KeepAlive(this);
                return result;
            }
        }
    }

    public sealed class DecisionResponse : NativeObject<DecisionResponse>, IEnumerable<SlotResponse>
    {
        public DecisionResponse() : base(new New<DecisionResponse>(NativeMethods.CreateDecisionResponse), new Delete<DecisionResponse>(NativeMethods.DeleteDecisionResponse))
        {
        }

        public string ModelId
        {
            get
            {
                int modelIdSize;
                IntPtr modelIdUtf8Ptr = NativeMethods.GetDecisionModelId(this.DangerousGetHandle(), modelIdSize);

                GC.KeepAlive(this);
                return NativeMethods.StringMarshallingFunc(modelIdUtf8Ptr, modelIdSize);
            }
        }

        public long Count
        {
            get
            {
                ulong unsignedSize = NativeMethods.GetDecisionSize(this.DangerousGetHandle()).ToUInt64();
                Debug.Assert(unsignedSize < Int64.MaxValue, "We do not support collections with size larger than _I64_MAX/Int64.MaxValue");
    
                GC.KeepAlive(this);
                return (long)unsignedSize;
            }
        }

        public IEnumerator<SlotResponse> GetEnumerator()
        {
            return new DecisionResponseEnumerator(this);
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return this.GetEnumerator();
        }

        private class DecisionResponseEnumerator : NativeObject<DecisionResponseEnumerator>, IEnumerator<SlotResponse>
        {
            [DllImport("rl.net.native.dll")]
            private static extern IntPtr CreateDecisionEnumeratorAdapter(IntPtr decisionResponse);

            private static New<DecisionResponseEnumerator> BindConstructorArguments(DecisionResponse decisionResponse)
            {
                return new New<DecisionResponseEnumerator>(() => 
                {
                    IntPtr result = CreateDecisionEnumeratorAdapter(decisionResponse.DangerousGetHandle());

                    GC.KeepAlive(decisionResponse); // Extend the lifetime of this handle because the delegate (and its data) is not stored on the heap.
                    return result;
                    
                });
            }

            [DllImport("rl.net.native.dll")]
            private static extern void DeleteDecisionEnumeratorAdapter(IntPtr decisionEnumeratorAdapter);

            [DllImport("rl.net.native.dll")]
            private static extern int DecisionEnumeratorInit(IntPtr decisionEnumeratorAdapter);

            [DllImport("rl.net.native.dll")]
            private static extern int DecisionEnumeratorMoveNext(IntPtr decisionEnumeratorAdapter);
        
            [DllImport("rl.net.native.dll")]
            private static extern IntPtr GetDecisionEnumeratorCurrent(IntPtr decisionEnumeratorAdapter);

            private bool initialState = true;

            public DecisionResponseEnumerator(DecisionResponse decisionResponse) : base(BindConstructorArguments(decisionResponse), new Delete<DecisionResponseEnumerator>(DeleteDecisionEnumeratorAdapter))
            {
            }

            public SlotResponse Current
            {
                get
                {
                    IntPtr sharedRankingResponseHandle = GetDecisionEnumeratorCurrent(this.DangerousGetHandle());

                    GC.KeepAlive(this);
                    return new SlotResponse(sharedRankingResponseHandle);
                }
            }

            object System.Collections.IEnumerator.Current => this.Current;

            public bool MoveNext()
            {
                int result;
                if (this.initialState)
                {
                    this.initialState = false;
                    result = DecisionEnumeratorInit(this.DangerousGetHandle());
                }
                else
                {
                    result = DecisionEnumeratorMoveNext(this.DangerousGetHandle());
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