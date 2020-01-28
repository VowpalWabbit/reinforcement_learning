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
            public static extern IntPtr GetSlotSlotId(IntPtr slotResponse);

            [DllImport("rl.net.native.dll")]
            public static extern int GetSlotActionId(IntPtr slotResponse);

            [DllImport("rl.net.native.dll")]
            public static extern void PushSlotActionProbability(IntPtr slot, int action, float prob);

            [DllImport("rl.net.native.dll")]
            public static extern float GetSlotProbability(IntPtr slotResponse);

            [DllImport("rl.net.native.dll")]
            public static extern IntPtr CreateDecisionResponse();

            [DllImport("rl.net.native.dll")]
            public static extern void DeleteDecisionResponse(IntPtr decisionResponse);

            [DllImport("rl.net.native.dll", EntryPoint = "GetDecisionModelId")]
            private static extern IntPtr GetDecisionModelIdNative(IntPtr decisionResponse);

            internal static Func<IntPtr, IntPtr> GetDecisionModelIdOverride { get; set; }

            [DllImport("rl.net.native.dll")]
            public static extern void SetDecisionModelId(IntPtr decision, IntPtr modelId);


            [DllImport("rl.net.native.dll")]
            public static extern IntPtr PushSlotResponse(IntPtr decisionResponse, IntPtr slotId);


            public static IntPtr GetDecisionModelId(IntPtr decisionResponse)
            {
                if (GetDecisionModelIdOverride != null)
                {
                    return GetDecisionModelIdOverride(decisionResponse);
                }

                return GetDecisionModelIdNative(decisionResponse);
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
                IntPtr modelIdUtf8Ptr = NativeMethods.GetSlotSlotId(this.DangerousGetHandle());

                GC.KeepAlive(this);
                return NativeMethods.StringMarshallingFunc(modelIdUtf8Ptr);
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

        public static DecisionResponse Create(string modelId)
        {
            var result = new DecisionResponse();
            DecisionResponseSetModelId(result.DangerousGetHandle(), modelId);
            return result;
        }


        public DecisionResponse AddSlot(string eventId, IList<int> actions, IList<float> probabilities)
        {
            return this;
        }

        public string ModelId
        {
            get
            {
                IntPtr modelIdUtf8Ptr = NativeMethods.GetDecisionModelId(this.DangerousGetHandle());

                GC.KeepAlive(this);
                return NativeMethods.StringMarshallingFunc(modelIdUtf8Ptr);
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

        unsafe private static void DecisionResponseSetModelId(IntPtr decisionResponse, string modelId)
        {
            fixed (byte* modelIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(modelId))
            {
                NativeMethods.SetDecisionModelId(decisionResponse, new IntPtr(modelIdUtf8Bytes));
            }
        }

        unsafe private static void DecisionResponsePushSlot(IntPtr decisionResponse, string slotId, IList<int> actions, IList<float> probabilities)
        {
            fixed (byte* slotIdUtf8Bytes = NativeMethods.StringEncoding.GetBytes(slotId))
            {
                IntPtr slot = NativeMethods.PushSlotResponse(decisionResponse, new IntPtr(slotIdUtf8Bytes));
                for (int i = 0; i < actions.Count; ++i)
                {
                    NativeMethods.PushSlotActionProbability(slot, actions[i], probabilities[i]);
                }
            }
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