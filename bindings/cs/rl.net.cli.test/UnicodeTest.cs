using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Rl.Net.Native;
using System.Runtime.InteropServices;

namespace Rl.Net.Cli.Test
{

    [TestClass]
    public class UnicodeTest : TestBase
    {
        const string PseudoLocLoremIpsum = "£ôřè₥ ïƥƨú₥ δôℓôř ƨïƭ á₥èƭ, çôñƨèçƭèƭúř áδïƥïƨçïñϱ èℓïƭ, ƨèδ δô èïúƨ₥ôδ ƭè₥ƥôř ïñçïδïδúñƭ úƭ ℓáβôřè èƭ δôℓôřè ₥áϱñá áℓï9úá. Ûƭ èñï₥ áδ ₥ïñï₥ Ʋèñïá₥, 9úïƨ ñôƨƭřúδ èжèřçïƭáƭïôñ úℓℓá₥çô ℓáβôřïƨ ñïƨï úƭ áℓï9úïƥ èж èá çô₥₥ôδô çôñƨè9úáƭ. Ðúïƨ áúƭè ïřúřè δôℓôř ïñ řèƥřèλèñδèřïƭ ïñ Ʋôℓúƥƭáƭè Ʋèℓïƭ èƨƨè çïℓℓú₥ δôℓôřè èú ƒúϱïáƭ ñúℓℓá ƥářïáƭúř. Éжçèƥƭèúř ƨïñƭ ôççáèçáƭ çúƥïδáƭáƭ ñôñ ƥřôïδèñƭ, ƨúñƭ ïñ çúℓƥá 9úï ôƒƒïçïá δèƨèřúñƭ ₥ôℓℓïƭ áñï₥ ïδ èƨƭ ℓáβôřú₥. ℓôřè₥ ïƥƨú₥ δôℓôř ƨïƭ á₥èƭ, çôñƨèçƭèƭúř áδïƥïƨçïñϱ èℓïƭ. Núñç èϱèƭ úřñá ℓáôřèèƭ, áççú₥ƨáñ ƒèℓïƨ áƭ, δáƥïβúƨ èℓïƭ. Ìñ úƭ ƭè₥ƥúƨ ₥áúřïƨ";
        const string PseudoLocJson =
@"{
""áβç"": ""δèƒ"",
""ϱλï"": ""ℓƙωèř effective. Power لُلُصّبُلُلصّبُررً ॣ ॣh ॣ ॣ 冗 ωèJƙř"",
""ωôω"": 1,
""ℓôřú₥ ïƥƨú₥ δôℓôř"": ""áβç { δèƒ: ϱλï }""
}
";
        const string PseudoLocJsonKey1 = "áβç";
        const string PseudoLocJsonValue1 = "δèƒ";
        const string PseudoLocJsonKey2 = "ϱλï";
        const string PseudoLocJsonValue2 = "ℓƙωèř effective. Power لُلُصّبُلُلصّبُررً ॣ ॣh ॣ ॣ 冗 ωèJƙř"; // Includes the iOS Message crash text
        const string PseudoLocJsonKey3 = "ωôω";
        const string PseudoLocJsonValue3 = "1";
        const string PseudoLocJsonKey4 = "ℓôřú₥ ïƥƨú₥ δôℓôř";
        const string PseudoLocJsonValue4 = "áβç { δèƒ: ϱλï }";

        const string PseudoLocConfigJson =
@"{
    ""ApplicationID"": ""ßïϱTèƨƭÂƥƥℓïçáƭïôñNá₥è-ℓôř"",
    ""IsExplorationEnabled"": true,
    ""InitialExplorationEpsilon"": 1.0,
    ""model.source"": ""NO_MODEL_DATA"",
    ""model.implementation"": ""PASSTHROUGH_PDF"",
    ""model.backgroundrefresh"": false,
    ""observation.sender.implementation"": ""OBSERVATION_FILE_SENDER"",
    ""interaction.sender.implementation"": ""INTERACTION_FILE_SENDER""
}
";

        const string PseudoLocEventId = "É1ß9ÐÇ83-8ÐF5-45É5-8Ð59-29F05ßÉ89ß96";
        const string PseudoLocContextJsonWithPdf =
@"{
    ""共有"": {""δèƒ"": ""ℓƙωèř áωèℓ ωèJƙř""},
    ""_multi"": [{
        ""ωôω"": 1
    },{
        ""ωôω"": 2
    }],
    ""p"": [0.4, 0.6]
}
";

        const string PseudoLocOutcomeJson =
@"{
    ""Reward"": ""1"",
    ""共有"": {""δèƒ"": ""ℓƙωèř áωèℓ ωèJƙř""}
}
";

        static float [] ExpectedPdf = { 0.4f, 0.6f };
        const float Epsilon = float.Epsilon;

        private void Run_PtrToStringUtf8_RoundtripTest(string str, string message)
        {
            unsafe
            {
                fixed (byte* strUtf8Bytes = NativeMethods.StringEncoding.GetBytes(str))
                {
                    // Arrange
                    IntPtr strUtf8Ptr = new IntPtr(strUtf8Bytes);

                    // Act
                    string marshalledBack = NativeMethods.StringMarshallingFunc(strUtf8Ptr);

                    // Assert
                    Assert.AreEqual(str, marshalledBack, message);
                }
            }
        }

        [TestInitialize]
        public void SetupTest()
        {
            void CleanupPInvokeOverrides()
            {
                NativeMethods.ConfigurationGetOverride = null;
                NativeMethods.ConfigurationSetOverride = null;
                NativeMethods.LiveModelChooseRankOverride = null;
                NativeMethods.LiveModelChooseRankWithFlagsOverride = null;
                NativeMethods.LiveModelReportActionTakenOverride = null;
                NativeMethods.LiveModelReportOutcomeFOverride = null;
                NativeMethods.LiveModelReportOutcomeJsonOverride = null;
                NativeMethods.LoadConfigurationFromJsonOverride = null;
            }
            this.TestCleanup.Add(CleanupPInvokeOverrides);
        }

        [TestMethod]
        public void Test_StringExtensions_PtrToStringUtf8_NullPtr()
        {
            string result = StringExtensions.PtrToStringUtf8(IntPtr.Zero);
            Assert.AreEqual(String.Empty, result, "Failed to marshal nullptr to .NET empty string");
        }

        [TestMethod]
        public void Test_StringExtensions_EmptyCStringToStringUtf8_NullPtr()
        {
            byte[] stringBytes = new byte[1];
            stringBytes[0] = 0;

            unsafe
            {
                fixed (byte* stringBytesPtr = stringBytes)
                {
                    // Arrange
                    IntPtr strPtr = new IntPtr(stringBytesPtr);

                    // Act
                    string marshalledBack = StringExtensions.PtrToStringUtf8(strPtr);

                    // Assert
                    Assert.AreEqual(String.Empty, marshalledBack, "Unable to marshal back a non-null empty cstring.");
                }
            }
        }

        // If this test has failed, the rest of the tests are inconclusive
        [TestMethod]
        public void Test_StringExtensions_PtrToStringUtf8()
        {
            // This is a smoke test that our round-tripping should work in practice
            this.Run_PtrToStringUtf8_RoundtripTest(PseudoLocLoremIpsum, "Failed to round-trip pseudolocalized lorem ipsum");
            this.Run_PtrToStringUtf8_RoundtripTest(PseudoLocJson, "Failed to round-trip pseudolocalized json");
            this.Run_PtrToStringUtf8_RoundtripTest(String.Empty, "Failed to round-trip empty string");
        }

        [TestMethod]
        public void Test_Configuration_LoadConfigurationFromJson()
        {
            Configuration config;
            ApiStatus status = new ApiStatus();

            if (!Configuration.TryLoadConfigurationFromJson(PseudoLocJson, out config, status))
            {
                Assert.Fail("Could not deserialize configuration from pseudolocalized json: " + status.ErrorMessage);
            }

            Assert.AreEqual(PseudoLocJsonValue2, config[PseudoLocJsonKey2], "Failed to parse and retrieve values from pseudolocalized json");
            Assert.AreEqual(PseudoLocJsonValue3, config[PseudoLocJsonKey3], "Failed to parse and retrieve values from pseudolocalized json");
            Assert.AreEqual(PseudoLocJsonValue1, config[PseudoLocJsonKey1], "Failed to parse and retrieve values from pseudolocalized json");
            Assert.AreEqual(PseudoLocJsonValue4, config[PseudoLocJsonKey4], "Failed to parse and retrieve values from pseudolocalized json");
        }

        [TestMethod]
        public void Test_Configuration_SetAndGetRoundtrip()
        {
            Configuration config = new Configuration();

            config[PseudoLocJsonKey1] = PseudoLocJsonValue1;
            config[PseudoLocJsonKey2] = PseudoLocJsonValue2;
            config[PseudoLocJsonKey3] = PseudoLocJsonValue3;
            config[PseudoLocJsonKey4] = PseudoLocJsonValue4;

            Assert.AreEqual(PseudoLocJsonValue2, config[PseudoLocJsonKey2], "Failed to set and retrieve pseudolocalized keys/values");
            Assert.AreEqual(PseudoLocJsonValue3, config[PseudoLocJsonKey3], "Failed to set and retrieve pseudolocalized keys/values");
            Assert.AreEqual(PseudoLocJsonValue1, config[PseudoLocJsonKey1], "Failed to set and retrieve pseudolocalized keys/values");
            Assert.AreEqual(PseudoLocJsonValue4, config[PseudoLocJsonKey4], "Failed to set and retrieve pseudolocalized keys/values");
        }

        public void Run_ConfigurationSet_Test(string key, string value)
        {
            NativeMethods.ConfigurationSetOverride =
                (IntPtr config, IntPtr keyPtr, IntPtr valuePtr) =>
                {
                    Assert.AreEqual(key, StringExtensions.PtrToStringUtf8(keyPtr), "Failed to round-trip key in Configuration[].set");
                    Assert.AreEqual(value ?? String.Empty, StringExtensions.PtrToStringUtf8(valuePtr), "Failed to round-trip value in Configuration[].set");
                };

            Configuration.ConfigurationSet(IntPtr.Zero, key, value);
        }

        [TestMethod]
        public void Test_Configuration_Set()
        {
            Run_ConfigurationSet_Test(PseudoLocJsonKey1, PseudoLocJsonValue1);
            Run_ConfigurationSet_Test(PseudoLocJsonKey2, null);
            Run_ConfigurationSet_Test(PseudoLocJsonKey3, String.Empty);
        }

        public void Run_ConfigurationGet_Test(string key, string defValue = null, string valueToReturn = null)
        {
            GCHandle valueToReturnHandle = default(GCHandle);

            try
            {
                IntPtr valueToReturnPtr = IntPtr.Zero;
                if (valueToReturn != null)
                {
                    byte[] valueToReturnUtf8Bytes = NativeMethods.StringEncoding.GetBytes(valueToReturn);
                    valueToReturnHandle = GCHandle.Alloc(valueToReturnUtf8Bytes, GCHandleType.Pinned);
                    valueToReturnPtr = valueToReturnHandle.AddrOfPinnedObject();
                }

                NativeMethods.ConfigurationGetOverride =
                    (IntPtr config, IntPtr keyPtr, IntPtr defValuePtr) =>
                    {
                        string keyMarshalledBack = StringExtensions.PtrToStringUtf8(keyPtr);
                        Assert.AreEqual(key, keyMarshalledBack, "Marshalling key does not work properly in Configuration[].get");

                        string defValueMarshalledBack = StringExtensions.PtrToStringUtf8(defValuePtr);
                        Assert.AreEqual(defValue ?? String.Empty, defValueMarshalledBack, "Marshalling defValue does not work properly in ConfigurationGet");

                        return valueToReturnPtr;
                    };

                string getResult = Configuration.ConfigurationGet(IntPtr.Zero, key, defValue);
                Assert.AreEqual(valueToReturn ?? String.Empty, getResult, "Marshalling result does not work properly in ConfigurationGet");
            }
            finally
            {
                if (valueToReturnHandle != null && valueToReturnHandle.IsAllocated)
                {
                    valueToReturnHandle.Free();
                }
            }
        }

        [TestMethod]
        public void Test_Configuration_Get()
        {
            Run_ConfigurationGet_Test(PseudoLocJsonKey1, String.Empty, PseudoLocJsonValue1);
            Run_ConfigurationGet_Test(PseudoLocJsonKey1, null, PseudoLocJsonValue1);

            Run_ConfigurationGet_Test(PseudoLocJsonKey2, valueToReturn: null);
            Run_ConfigurationGet_Test(PseudoLocJsonKey3, valueToReturn: String.Empty);
        }

        private LiveModel ConfigureLiveModel()
        {
            Configuration config;
            ApiStatus apiStatus = new ApiStatus();
            if (!Configuration.TryLoadConfigurationFromJson(PseudoLocConfigJson, out config, apiStatus))
            {
                Assert.Fail("Failed to parse pseudolocalized configuration JSON: " + apiStatus.ErrorMessage);
            }

            TempFileDisposable interactionDisposable = new TempFileDisposable();
            this.TestCleanup.Add(interactionDisposable);

            TempFileDisposable observationDisposable = new TempFileDisposable();
            this.TestCleanup.Add(observationDisposable);

            config["interaction.file.name"] = interactionDisposable.Path;
            config["observation.file.name"] = observationDisposable.Path;

            LiveModel liveModel = new LiveModel(config);
            liveModel.Init();
            liveModel.RefreshModel();

            return liveModel;
        }

        private void ValidatePdf(RankingResponse rankingResponse)
        {
            List<ActionProbability> actionProbabilities = rankingResponse.ToList();

            Assert.AreEqual(ExpectedPdf.Length, actionProbabilities.Count, "Input PDF length does not match samping PDF length.");

            for (int i = 0; i < actionProbabilities.Count; i++)
            {
                int actionIndex = (int)actionProbabilities[i].ActionIndex;
                Assert.AreEqual(ExpectedPdf[actionIndex], actionProbabilities[i].Probability, float.Epsilon, "PDF score does not match input probability.");
            }
        }

        [TestMethod]
        public void Test_LiveModel_ChooseRankE2E()
        {
            LiveModel liveModel = this.ConfigureLiveModel();

            RankingResponse rankingResponse1 = liveModel.ChooseRank(PseudoLocEventId, PseudoLocContextJsonWithPdf);
            ValidatePdf(rankingResponse1);

            RankingResponse rankingResponse2 = liveModel.ChooseRank(PseudoLocEventId, PseudoLocContextJsonWithPdf, ActionFlags.Deferred);
            ValidatePdf(rankingResponse2);
        }

        private void Run_LiveModelChooseRank_Test(LiveModel liveModel, string eventId, string contextJson)
        {
            NativeMethods.LiveModelChooseRankOverride =
                (IntPtr liveModelPtr, IntPtr eventIdPtr, IntPtr contextJsonPtr, IntPtr rankingResponse, IntPtr apiStatus) =>
                {
                    string eventIdMarshalledBack = NativeMethods.StringMarshallingFunc(eventIdPtr);
                    Assert.AreEqual(eventId, eventIdMarshalledBack, "Marshalling eventId does not work properly in LiveModelChooseRank");

                    string contextJsonMarshalledBack = NativeMethods.StringMarshallingFunc(contextJsonPtr);
                    Assert.AreEqual(contextJson, contextJsonMarshalledBack, "Marshalling contextJson does not work properly in LiveModelChooseRank");

                    return NativeMethods.SuccessStatus;
                };

            liveModel.ChooseRank(eventId, contextJson);
        }

        private void Run_LiveModelChooseRankWithFlags_Test(LiveModel liveModel, string eventId, string contextJson)
        {
            NativeMethods.LiveModelChooseRankWithFlagsOverride =
                (IntPtr liveModelPtr, IntPtr eventIdPtr, IntPtr contextJsonPtr, uint flags, IntPtr rankingResponse, IntPtr apiStatus) =>
                {
                    string eventIdMarshalledBack = NativeMethods.StringMarshallingFunc(eventIdPtr);
                    Assert.AreEqual(eventId, eventIdMarshalledBack, "Marshalling eventId does not work properly in LiveModelChooseRank");

                    string contextJsonMarshalledBack = NativeMethods.StringMarshallingFunc(contextJsonPtr);
                    Assert.AreEqual(contextJson, contextJsonMarshalledBack, "Marshalling contextJson does not work properly in LiveModelChooseRank");

                    return NativeMethods.SuccessStatus;
                };

            liveModel.ChooseRank(eventId, contextJson, ActionFlags.Deferred);
        }

        [TestMethod]
        public void Test_LiveModel_ChooseRank()
        {
            LiveModel liveModel = this.ConfigureLiveModel();

            Run_LiveModelChooseRank_Test(liveModel, PseudoLocEventId, PseudoLocContextJsonWithPdf);
            Run_LiveModelChooseRankWithFlags_Test(liveModel, PseudoLocEventId, PseudoLocContextJsonWithPdf);
        }

        private void Run_LiveModelRequestDecision_Test(LiveModel liveModel, string contextJson)
        {
            NativeMethods.LiveModelRequestDecisionOverride =
                (IntPtr liveModelPtr, IntPtr contextJsonPtr, IntPtr rankingResponse, IntPtr ApiStatus) =>
                {
                    string contextJsonMarshalledBack = NativeMethods.StringMarshallingFunc(contextJsonPtr);
                    Assert.AreEqual(contextJson, contextJsonMarshalledBack, "Marshalling contextJson does not work properly in LiveModelRequestDecision");

                    return NativeMethods.SuccessStatus;
                };

            liveModel.RequestDecision(contextJson);
        }

        private void Run_LiveModelRequestDecisionWithFlags_Test(LiveModel liveModel, string contextJson)
        {
            NativeMethods.LiveModelRequestDecisionWithFlagsOverride =
                (IntPtr liveModelPtr, IntPtr contextJsonPtr, uint flags, IntPtr rankingResponse, IntPtr ApiStatus) =>
                {
                    string contextJsonMarshalledBack = NativeMethods.StringMarshallingFunc(contextJsonPtr);
                    Assert.AreEqual(contextJson, contextJsonMarshalledBack, "Marshalling contextJson does not work properly in LiveModelRequestDecisionWithFlags");

                    return NativeMethods.SuccessStatus;
                };

            liveModel.RequestDecision(contextJson, ActionFlags.Deferred);
        }

        [TestMethod]
        public void Test_LiveModel_RequestDecision()
        {
            LiveModel liveModel = this.ConfigureLiveModel();

            Run_LiveModelRequestDecision_Test(liveModel, PseudoLocContextJsonWithPdf);
            Run_LiveModelRequestDecisionWithFlags_Test(liveModel, PseudoLocContextJsonWithPdf);
        }

        private void Run_LiveModelRequestContinuousAction_Test(LiveModel liveModel, string contextJson)
        {
            NativeMethods.LiveModelRequestContinuousActionOverride =
                (IntPtr liveModelPtr, IntPtr eventIdPtr, IntPtr contextJsonPtr, IntPtr continuousActionResponse, IntPtr ApiStatus) =>
                {
                    string contextJsonMarshalledBack = NativeMethods.StringMarshallingFunc(contextJsonPtr);
                    Assert.AreEqual(contextJson, contextJsonMarshalledBack, "Marshalling contextJson does not work properly in LiveModelRequestContinuousAction");

                    return NativeMethods.SuccessStatus;
                };

            liveModel.RequestContinuousAction(PseudoLocEventId, contextJson);
        }

        [TestMethod]
        public void Test_LiveModel_RequestContinuousAction()
        {
            LiveModel liveModel = this.ConfigureLiveModel();

            Run_LiveModelRequestContinuousAction_Test(liveModel, PseudoLocContextJsonWithPdf);
        }

        // TODO: Create a real CCB context json and add an E2E test (pending CCB E2E and
        // clarity around sample mode.)

        private void Run_LiveModelReportActionTaken_Test(LiveModel liveModel, string eventId)
        {
            NativeMethods.LiveModelReportActionTakenOverride =
                (IntPtr liveModelPtr, IntPtr eventIdPtr, IntPtr apiStatus) =>
                {
                    string eventIdMarshalledBack = NativeMethods.StringMarshallingFunc(eventIdPtr);
                    Assert.AreEqual(eventId, eventIdMarshalledBack, "Marshalling eventId does not work properly in LiveModelReportActionTaken");

                    return NativeMethods.SuccessStatus;
                };

            liveModel.QueueActionTakenEvent(eventId);
        }

        [TestMethod]
        public void Test_LiveModel_ReportActionTaken()
        {
            LiveModel liveModel = this.ConfigureLiveModel();

            Run_LiveModelReportActionTaken_Test(liveModel, PseudoLocEventId);
        }

        private void Run_LiveModelReportOutcomeF_Test(LiveModel liveModel, string eventId, float outcome)
        {
            NativeMethods.LiveModelReportOutcomeFOverride =
                (IntPtr liveModelPtr, IntPtr eventIdPtr, float o, IntPtr apiStatus) =>
                {
                    string eventIdMarshalledBack = NativeMethods.StringMarshallingFunc(eventIdPtr);
                    Assert.AreEqual(eventId, eventIdMarshalledBack, "Marshalling eventId does not work properly in LiveModelReportOutcomeF");

                    return NativeMethods.SuccessStatus;
                };

            liveModel.QueueOutcomeEvent(eventId, outcome);
        }

        private void Run_LiveModelReportOutcomeJson_Test(LiveModel liveModel, string eventId, string outcomeJson)
        {
            NativeMethods.LiveModelReportOutcomeJsonOverride =
                (IntPtr liveModelPtr, IntPtr eventIdPtr, IntPtr outcomeJsonPtr, IntPtr apiStatus) =>
                {
                    string eventIdMarshalledBack = NativeMethods.StringMarshallingFunc(eventIdPtr);
                    Assert.AreEqual(eventId, eventIdMarshalledBack, "Marshalling eventId does not work properly in LiveModelReportOutcomeJson");

                    string outcomeJsonMarshalledBack = NativeMethods.StringMarshallingFunc(outcomeJsonPtr);
                    Assert.AreEqual(outcomeJson, outcomeJsonMarshalledBack, "Marshalling eventId does not work properly in LiveModelReportOutcomeJson");

                    return NativeMethods.SuccessStatus;
                };

            liveModel.QueueOutcomeEvent(eventId, outcomeJson);
        }

        private void Run_LiveModelReportOutcomeSlotF_Test(LiveModel liveModel, string eventId, uint slotIndex, float outcome)
        {
            NativeMethods.LiveModelReportOutcomeSlotFOverride =
                (IntPtr liveModelPtr, IntPtr eventIdPtr, uint slotI, float o, IntPtr apiStatus) =>
                {
                    string eventIdMarshalledBack = NativeMethods.StringMarshallingFunc(eventIdPtr);
                    Assert.AreEqual(eventId, eventIdMarshalledBack, "Marshalling eventId does not work properly in LiveModelReportOutcomeF");

                    return NativeMethods.SuccessStatus;
                };

            liveModel.QueueOutcomeEvent(eventId, slotIndex, outcome);
        }

        private void Run_LiveModelReportOutcomeSlotJson_Test(LiveModel liveModel, string eventId, uint slotIndex, string outcomeJson)
        {
            NativeMethods.LiveModelReportOutcomeSlotJsonOverride =
                (IntPtr liveModelPtr, IntPtr eventIdPtr, uint slotI, IntPtr outcomeJsonPtr, IntPtr apiStatus) =>
                {
                    string eventIdMarshalledBack = NativeMethods.StringMarshallingFunc(eventIdPtr);
                    Assert.AreEqual(eventId, eventIdMarshalledBack, "Marshalling eventId does not work properly in LiveModelReportOutcomeJson");

                    string outcomeJsonMarshalledBack = NativeMethods.StringMarshallingFunc(outcomeJsonPtr);
                    Assert.AreEqual(outcomeJson, outcomeJsonMarshalledBack, "Marshalling outcomeJson does not work properly in LiveModelReportOutcomeJson");

                    return NativeMethods.SuccessStatus;
                };

            liveModel.QueueOutcomeEvent(eventId, slotIndex, outcomeJson);
        }

        [TestMethod]
        public void Test_LiveModel_ReportOutcome()
        {
            LiveModel liveModel = this.ConfigureLiveModel();

            Run_LiveModelReportOutcomeF_Test(liveModel, PseudoLocEventId, 1.0f);
            Run_LiveModelReportOutcomeJson_Test(liveModel, PseudoLocEventId, PseudoLocOutcomeJson);
            Run_LiveModelReportOutcomeSlotF_Test(liveModel, PseudoLocEventId, 1, 1.0f);
            Run_LiveModelReportOutcomeSlotJson_Test(liveModel, PseudoLocEventId, 1, PseudoLocOutcomeJson);
        }

        private void Run_StringReturnMarshallingTest<TNativeObject>(string valueToReturn, Action<Func<IntPtr, IntPtr>> registerNativeOverride, Func<TNativeObject, string> targetInvocation, string targetInvocationName)
            where TNativeObject : NativeObject<TNativeObject>, new()
        {
            TNativeObject targetObject = new TNativeObject();
            GCHandle valueToReturnHandle = default(GCHandle);

            try
            {
                IntPtr valueToReturnPtr = IntPtr.Zero;
                if (valueToReturn != null)
                {
                    byte[] valueToReturnUtf8Bytes = NativeMethods.StringEncoding.GetBytes(valueToReturn);
                    valueToReturnHandle = GCHandle.Alloc(valueToReturnUtf8Bytes, GCHandleType.Pinned);
                    valueToReturnPtr = valueToReturnHandle.AddrOfPinnedObject();
                }

                IntPtr ReturnTargetValue(IntPtr targetObjectPtr)
                {
                    return valueToReturnPtr;
                }

                registerNativeOverride(ReturnTargetValue);

                string getResult = targetInvocation(targetObject);
                Assert.AreEqual(valueToReturn ?? String.Empty, getResult, $"Marshalling result does not work properly in {targetInvocationName}");
            }
            finally
            {
                if (valueToReturnHandle != null && valueToReturnHandle.IsAllocated)
                {
                    valueToReturnHandle.Free();
                }

                targetObject?.Dispose();
                targetObject = null;
            }
        }

        private void Run_GetRankingModelId_Test(string modelIdToReturn)
        {
            void RegisterNativeOverride(Func<IntPtr, IntPtr> nativeOverrideCallback)
            {
                NativeMethods.GetRankingModelIdOverride = nativeOverrideCallback;
            }

            Run_StringReturnMarshallingTest<RankingResponse>(modelIdToReturn, RegisterNativeOverride, rankingResponse => rankingResponse.ModelId, nameof(NativeMethods.GetRankingModelId));
        }

        private void Run_GetRankingEventId_Test(string eventIdToReturn)
        {
            void RegisterNativeOverride(Func<IntPtr, IntPtr> nativeOverrideCallback)
            {
                NativeMethods.GetRankingEventIdOverride = nativeOverrideCallback;
            }

            Run_StringReturnMarshallingTest<RankingResponse>(eventIdToReturn, RegisterNativeOverride, rankingResponse => rankingResponse.EventId, nameof(NativeMethods.GetRankingEventId));
        }

        [TestMethod]
        public void Test_RankingResponseStringProperties()
        {
            Run_GetRankingEventId_Test(PseudoLocEventId);

            Run_GetRankingModelId_Test(String.Join("/", PseudoLocEventId, PseudoLocEventId));
            Run_GetRankingModelId_Test(String.Empty);
            Run_GetRankingModelId_Test(null);
        }

        private void Run_GetDecisionModelId_Test(string modelIdToReturn)
        {
            void RegisterNativeOverride(Func<IntPtr, IntPtr> nativeOverrideCallback)
            {
                NativeMethods.GetDecisionModelIdOverride = nativeOverrideCallback;
            }

            Run_StringReturnMarshallingTest<DecisionResponse>(modelIdToReturn, RegisterNativeOverride, decisionResponse => decisionResponse.ModelId, nameof(NativeMethods.GetDecisionModelId));
        }

        [TestMethod]
        public void Test_DecisionResponseStringProperties()
        {
            Run_GetDecisionModelId_Test(String.Join("/", PseudoLocEventId, PseudoLocEventId));
            Run_GetDecisionModelId_Test(String.Empty);
            Run_GetDecisionModelId_Test(null);
        }

        // TODO: Figure out how to project the mock objects from the C++ unit testing code to be
        // usable 
    }
}
