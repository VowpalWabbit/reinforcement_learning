using System;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Rl.Net;
using Rl.Net.Native;

namespace Rl.Net.Cli.Test
{
    using SenderFactory = Func<IReadOnlyConfiguration, ErrorCallback, ISender>;
    using BackgroundErrorCallback = Action<ApiStatus>;
    internal class MockSender : ISender
    {
        public Action<ApiStatus> Init
        {
            get;
            set;
        }

        public Action<SharedBuffer, ApiStatus> Send
        {
            get;
            set;
        }

        void ISender.Init(ApiStatus status)
        {
            if (this.Init != null)
            {
                this.Init(status);
            }
        }

        void ISender.Send(SharedBuffer buffer, ApiStatus status)
        {
            if (this.Send != null)
            {
                this.Send(buffer, status);
            }
        }
    }


    internal class MockAsyncSender : AsyncSender
    {
        public MockAsyncSender(ErrorCallback callback) : base(callback)
        {
        }

        public new Func<SharedBuffer, BackgroundErrorCallback, Task> Send
        {
            get;
            set;
        }

        protected override Task SendAsync(SharedBuffer buffer)
        {
            if (this.Send != null)
            {
                return this.Send(buffer, this.RaiseBackgroundError);
            }

            return Task.CompletedTask;
        }
    }

    [TestClass]
    public class SenderExtensibilityTest : TestBase
    {
        const string CustomSenderConfigJson =
@"{
    ""ApplicationID"": ""ßïϱTèƨƭÂƥƥℓïçáƭïôñNá₥è-ℓôř"",
    ""IsExplorationEnabled"": true,
    ""InitialExplorationEpsilon"": 1.0,
    ""model.source"": ""NO_MODEL_DATA"",
    ""model.implementation"": ""PASSTHROUGH_PDF"",
    ""model.backgroundrefresh"": false,
    ""observation.sender.implementation"": ""BINDING_SENDER"",
    ""interaction.sender.implementation"": ""BINDING_SENDER"",
    ""observation.send.batchintervalms"": 10,
    ""interaction.send.batchintervalms"": 10
}
";

    const string FileSenderConfigJson =
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

        private LiveModel CreateLiveModel(FactoryContext factoryContext = null)
        {
            Configuration config;
            ApiStatus apiStatus = new ApiStatus();
            if (!Configuration.TryLoadConfigurationFromJson(CustomSenderConfigJson, out config, apiStatus))
            {
                Assert.Fail("Failed to parse pseudolocalized configuration JSON: " + apiStatus.ErrorMessage);
            }

            LiveModel liveModel = factoryContext == null ? new LiveModel(config) : new LiveModel(config, factoryContext);

            return liveModel;
        }

        [TestMethod]
        public void Test_CustomSender_FailsWhenNotRegistered()
        {
            const int TypeNotRegisteredError = 10; // see errors_data.h

            ApiStatus apiStatus = new ApiStatus();
            LiveModel liveModel = CreateLiveModel();

            Assert.IsFalse(liveModel.TryInit(apiStatus), "Should not be able to configure a model with BINDING_SENDER if custom factory is not set.");

            Assert.AreEqual(TypeNotRegisteredError, apiStatus.ErrorCode);
        }

        [TestMethod]
        public void Test_CustomSender_FactoryNotCalled_WhenNotRequested()
        {
            ApiStatus apiStatus = new ApiStatus();

            FactoryContext factoryContext = new FactoryContext();

            bool factoryCalled = false;
            Func<IReadOnlyConfiguration, ErrorCallback, MockSender> customFactory = 
            (IReadOnlyConfiguration readOnlyConfig, ErrorCallback callback) =>
            {
                factoryCalled = true;

                return new MockSender();
            };

            Configuration config = new Configuration();
            if (!Configuration.TryLoadConfigurationFromJson(FileSenderConfigJson, out config, apiStatus))
            {
                Assert.Fail("Failed to parse pseudolocalized configuration JSON: " + apiStatus.ErrorMessage);
            }

            TempFileDisposable interactionDisposable = new TempFileDisposable();
            this.TestCleanup.Add(interactionDisposable);

            TempFileDisposable observationDisposable = new TempFileDisposable();
            this.TestCleanup.Add(observationDisposable);

            config["interaction.file.name"] = interactionDisposable.Path;
            config["observation.file.name"] = observationDisposable.Path;

            factoryContext.SetSenderFactory(customFactory);

            LiveModel liveModel = new LiveModel(config, factoryContext);
            liveModel.Init();

            Assert.IsFalse(factoryCalled, "Custom factory should not be called unless BINDING_SENDER is selected in configuration.");
        }

        private FactoryContext CreateFactoryContext(SenderFactory customFactory)
        {
            FactoryContext factoryContext = new FactoryContext();
            factoryContext.SetSenderFactory(customFactory);

            return factoryContext;
        }

        private FactoryContext CreateFactoryContext(Action<ApiStatus> initAction = null, Action<SharedBuffer, ApiStatus> sendAction = null)
        {
            return CreateFactoryContext(
                (config, callback) => 
                {
                    return new MockSender
                    {
                        Init = initAction,
                        Send = sendAction
                    };
                }
            );
        }

        private FactoryContext CreateFactoryContext(Func<SharedBuffer, BackgroundErrorCallback, Task> asyncSendFunc = null)
        {
            return CreateFactoryContext(
                (config, callback) =>
                {
                    return new MockAsyncSender(callback)
                    {
                        Send = asyncSendFunc
                    };
                }
            );
        }

        [TestMethod]
        public void Test_CustomSender_FactoryCalled_WhenRequested()
        {
            ApiStatus apiStatus = new ApiStatus();

            bool factoryCalled = false;
            FactoryContext factoryContext = CreateFactoryContext(
                (IReadOnlyConfiguration config, ErrorCallback callback) =>
                {
                    factoryCalled = true;

                    return new MockSender();
                });

            
            LiveModel liveModel = CreateLiveModel(factoryContext);
            liveModel.Init();

            Assert.IsTrue(factoryCalled, "Custom factory must be called when BINDING_SENDER is selected in configuration.");
        }

        [TestMethod]
        public void Test_CustomSender_InitSuccess()
        {
            bool initCalled = false;
            void SenderInit(ApiStatus status)
            {
                initCalled = true;
            }

            FactoryContext factoryContext = CreateFactoryContext(initAction: SenderInit);

            LiveModel liveModel = CreateLiveModel(factoryContext);
            liveModel.Init();

            Assert.IsTrue(initCalled, "MockSender.Init should be called and succeed, which means LiveModel.Init should succeed.");
        }

        const string OpaqueErrorMessage = "Opaque error in external code. §ô₥è Tèжƭ Fřô₥ ÐôƭNèƭ ℓôřè₥";

        private void Run_TestCustomSender_InitFailure(Action<ApiStatus> senderInit, string expectedString, bool expectPrefix = false)
        {
            FactoryContext factoryContext = CreateFactoryContext(initAction: senderInit);

            ApiStatus apiStatus = new ApiStatus();
            LiveModel liveModel = CreateLiveModel(factoryContext);
            Assert.IsFalse(liveModel.TryInit(apiStatus), "MockSender.Init should be called and fail, which means LiveModel.Init should fail.");

            Assert.AreEqual(NativeMethods.OpaqueBindingError, apiStatus.ErrorCode);

            if (!expectPrefix)
            {
                Assert.AreEqual(expectedString, apiStatus.ErrorMessage);
            }
            else
            {
                Assert.IsTrue(apiStatus.ErrorMessage.StartsWith(expectedString));
            }
        }

        [TestMethod]
        public void Test_CustomSender_InitFailure_ViaApiStatusBuilder()
        {
            const string ExpectedString = OpaqueErrorMessage;
            void SenderInit(ApiStatus status)
            {
                new ApiStatusBuilder(NativeMethods.OpaqueBindingError)
                    .Append("§ô₥è Tèжƭ Fřô₥ ÐôƭNèƭ ℓôřè₥")
                    .UpdateApiStatus(status);
            }

            this.Run_TestCustomSender_InitFailure(SenderInit, ExpectedString);
        }

        [TestMethod]
        public void Test_CustomSender_InitFailure_ViaRLException()
        {
            const string ExpectedString = OpaqueErrorMessage;
            void SenderInit(ApiStatus status)
            {
                throw new RLException("§ô₥è Tèжƭ Fřô₥ ÐôƭNèƭ ℓôřè₥");
            }

            this.Run_TestCustomSender_InitFailure(SenderInit, ExpectedString);
        }

        [TestMethod]
        public void Test_CustomSender_InitFailure_ViaNonRLException()
        {
            string expectedStringPrefix = OpaqueErrorMessage;
            void SenderInit(ApiStatus status)
            {
                try
                {
                    throw new Exception("§ô₥è Tèжƭ Fřô₥ ÐôƭNèƭ ℓôřè₥");
                }
                catch (Exception e)
                {
                    expectedStringPrefix += "\n" + e.StackTrace;
                    throw;
                }
            }

            this.Run_TestCustomSender_InitFailure(SenderInit, expectedStringPrefix, expectPrefix: true);
        }

        const string EventId = "É1ß9ÐÇ83-8ÐF5-45É5-8Ð59-29F05ßÉ89ß96";
        const string ContextJsonWithPdf =
@"{
    ""共有"": {""δèƒ"": ""ℓƙωèř áωèℓ ωèJƙř""},
    ""_multi"": [{
        ""ωôω"": 1
    },{
        ""ωôω"": 2
    }],
    ""p"": [0.3, 0.7]
}
";

        [TestMethod]
        public void Test_CustomSender_SendSuccess()
        {
            ManualResetEventSlim senderCalledWaiter = new ManualResetEventSlim(initialState: false);

            bool sendCalled = false;
            void SenderSend(SharedBuffer buffer, ApiStatus status)
            {
                sendCalled = true;
                senderCalledWaiter.Set();
            }

            FactoryContext factoryContext = CreateFactoryContext(sendAction: SenderSend);

            LiveModel liveModel = CreateLiveModel(factoryContext);
            liveModel.Init();
            RankingResponse response = liveModel.ChooseRank(EventId, ContextJsonWithPdf);

            senderCalledWaiter.Wait(TimeSpan.FromSeconds(1));

            Assert.IsTrue(sendCalled);
        }

        private void Run_TestCustomSender_SendFailure(FactoryContext factoryContext, string expectedString, bool expectPrefix = false)
        {
            ManualResetEventSlim backgroundMessageWaiter = new ManualResetEventSlim(initialState: false);

            int backgroundErrorCount = 0;
            int backgroundErrorCode = 0;
            string backgroundErrorMessage = null;
            void OnBackgroundError(object sender, ApiStatus args)
            {
                Assert.AreEqual(0, backgroundErrorCount++, "Do not duplicate background errors.");
                backgroundErrorCode = args.ErrorCode;
                backgroundErrorMessage = args.ErrorMessage;

                backgroundMessageWaiter.Set();
            }

            LiveModel liveModel = CreateLiveModel(factoryContext);
            liveModel.BackgroundError += OnBackgroundError;

            liveModel.Init();

            ApiStatus apiStatus = new ApiStatus();
            RankingResponse response;
            Assert.IsTrue(liveModel.TryChooseRank(EventId, ContextJsonWithPdf, out response, apiStatus));
            Assert.AreEqual(NativeMethods.SuccessStatus, apiStatus.ErrorCode, "Errors from ISender.Send should be background errors.");

            backgroundMessageWaiter.Wait(TimeSpan.FromSeconds(1));

            Assert.AreEqual(NativeMethods.OpaqueBindingError, backgroundErrorCode, "Error from ISender did not get raised.");

            if (!expectPrefix)
            {
                Assert.AreEqual(OpaqueErrorMessage, backgroundErrorMessage);
            }
            else
            {
                Assert.IsTrue(backgroundErrorMessage.StartsWith(OpaqueErrorMessage));
            }
        }

        private void Run_TestCustomSender_SendFailure(Action<SharedBuffer, ApiStatus> senderSend, string expectedString, bool expectPrefix = false)
        {
            FactoryContext factoryContext = CreateFactoryContext(sendAction: senderSend);

            Run_TestCustomSender_SendFailure(factoryContext, expectedString, expectPrefix);
        }

        [TestMethod]
        public void Test_CustomSender_SendFailure_ViaApiStatusBuilder()
        {
            void SenderSend(SharedBuffer buffer, ApiStatus status)
            {
                new ApiStatusBuilder(NativeMethods.OpaqueBindingError)
                    .Append("§ô₥è Tèжƭ Fřô₥ ÐôƭNèƭ ℓôřè₥")
                    .UpdateApiStatus(status);
            }

            this.Run_TestCustomSender_SendFailure(SenderSend, OpaqueErrorMessage);
        }

        [TestMethod]
        public void Test_CustomSender_SendFailure_ViaRLException()
        {
            void SenderSend(SharedBuffer buffer, ApiStatus status)
            {
                throw new RLException("§ô₥è Tèжƭ Fřô₥ ÐôƭNèƭ ℓôřè₥");
            }

            this.Run_TestCustomSender_SendFailure(SenderSend, OpaqueErrorMessage);
        }

        [TestMethod]
        public void Test_CustomSender_SendFailure_ViaNonRLException()
        {
            string expectedStringPrefix = OpaqueErrorMessage;
            void SenderSend(SharedBuffer buffer, ApiStatus status)
            {
                try
                {
                    throw new Exception("§ô₥è Tèжƭ Fřô₥ ÐôƭNèƭ ℓôřè₥");
                }
                catch (Exception e)
                {
                    expectedStringPrefix += "\n" + e.StackTrace;
                    throw;
                }
            }

            this.Run_TestCustomSender_SendFailure(SenderSend, expectedStringPrefix, expectPrefix: true);
        }

        [TestMethod]
        public void Test_AsyncSender_SendSuccess()
        {
            ManualResetEventSlim senderCalledWaiter = new ManualResetEventSlim(initialState: false);

            bool sendCalled = false;
            Task AsyncSenderSend(SharedBuffer buffer, BackgroundErrorCallback raiseBackgroundError)
            {
                sendCalled = true;
                senderCalledWaiter.Set();

                return Task.CompletedTask;
            }

            FactoryContext factoryContext = CreateFactoryContext(asyncSendFunc: AsyncSenderSend);

            LiveModel liveModel = CreateLiveModel(factoryContext);
            liveModel.Init();
            RankingResponse response = liveModel.ChooseRank(EventId, ContextJsonWithPdf);

            senderCalledWaiter.Wait(TimeSpan.FromSeconds(1));

            Assert.IsTrue(sendCalled);
        }

        private void Run_TestAsyncSender_SendFailure(Func<SharedBuffer, BackgroundErrorCallback, Task> asyncSenderSend, string expectedString, bool expectPrefix = false)
        {
            FactoryContext factoryContext = CreateFactoryContext(asyncSendFunc: asyncSenderSend);

            Run_TestCustomSender_SendFailure(factoryContext, expectedString, expectPrefix);
        }
    
        [TestMethod]
        public void Test_AsyncSender_SendFailure_ViaExplicitRaise()
        {
            Task AsyncSenderSend(SharedBuffer buffer, BackgroundErrorCallback raiseBackgroundError)
            {
                ApiStatusBuilder statusBuilder = new ApiStatusBuilder(NativeMethods.OpaqueBindingError)
                    .Append("§ô₥è Tèжƭ Fřô₥ ÐôƭNèƭ ℓôřè₥");

                raiseBackgroundError(statusBuilder.ToApiStatus());

                return Task.CompletedTask;
            }

            this.Run_TestAsyncSender_SendFailure(AsyncSenderSend, OpaqueErrorMessage);
        }

        [TestMethod]
        public void Test_AsyncSender_SendFailure_ViaRLException()
        {
            
            #pragma warning disable 1998
            async Task AsyncSenderSend(SharedBuffer buffer, BackgroundErrorCallback raiseBackgroundError)
            {
                throw new RLException("§ô₥è Tèжƭ Fřô₥ ÐôƭNèƭ ℓôřè₥");
            }
            #pragma warning restore 

            this.Run_TestAsyncSender_SendFailure(AsyncSenderSend, OpaqueErrorMessage);
        }

        [TestMethod]
        public void Test_AsyncSender_SendFailure_ViaRLExceptionInTask()
        {
            Task AsyncSenderSend(SharedBuffer buffer, BackgroundErrorCallback raiseBackgroundError)
            {
                return Task.FromException(new RLException("§ô₥è Tèжƭ Fřô₥ ÐôƭNèƭ ℓôřè₥"));
            }

            this.Run_TestAsyncSender_SendFailure(AsyncSenderSend, OpaqueErrorMessage);
        }

        [TestMethod]
        public void Test_AsyncSender_SendFailure_ViaNonRLException()
        {
            string expectedStringPrefix = OpaqueErrorMessage;

            #pragma warning disable 1998
            async Task AsyncSenderSend(SharedBuffer buffer, BackgroundErrorCallback raiseBackgroundError)
            {
                try
                {
                    throw new Exception("§ô₥è Tèжƭ Fřô₥ ÐôƭNèƭ ℓôřè₥");
                }
                catch (Exception e)
                {
                    expectedStringPrefix += "\n" + e.StackTrace;
                    throw e;
                }
            }
            #pragma warning restore

            this.Run_TestAsyncSender_SendFailure(AsyncSenderSend, OpaqueErrorMessage, expectPrefix: true);
        }

        [TestMethod]
        public void Test_AsyncSender_SendFailure_ViaNonRLExceptionInTask()
        {
            string expectedStringPrefix = OpaqueErrorMessage;
            Task AsyncSenderSend(SharedBuffer buffer, BackgroundErrorCallback raiseBackgroundError)
            {
                try
                {
                    throw new Exception("§ô₥è Tèжƭ Fřô₥ ÐôƭNèƭ ℓôřè₥");
                }
                catch (Exception e)
                {
                    expectedStringPrefix += "\n" + e.StackTrace;
                    return Task.FromException(e);
                }
            }

            this.Run_TestAsyncSender_SendFailure(AsyncSenderSend, expectedStringPrefix, expectPrefix: true);
        }
    }

    
}
