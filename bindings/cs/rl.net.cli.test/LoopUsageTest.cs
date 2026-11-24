using System;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Rl.Net;
using Rl.Net.Native;
using Rl.Net.Cli.Test;

namespace Rl.Net.Cli.Test
{
    using SenderFactory = Func<IReadOnlyConfiguration, ErrorCallback, ISender>;
    using BackgroundErrorCallback = Action<ApiStatus>;

    [TestClass]
    public class LoopTest : TestBase
    {

        string configJson =
        @"{
            ""ApplicationID"": ""testclient"",
            ""interaction.sender.implementation"": ""INTERACTION_FILE_SENDER"",
            ""observation.sender.implementation"": ""OBSERVATION_FILE_SENDER"",
            ""interaction.file.name"": ""interaction.fbs"",
            ""observation.file.name"": ""observation.fbs"",
            ""IsExplorationEnabled"": true,
            ""InitialExplorationEpsilon"": 1.0,
            ""LearningMode"": ""Online"",
            ""model.source"": ""FILE_MODEL_DATA"",
            ""protocol.version"":""2""
        }";

        const string contextJsonCA = 
        @"{
            ""id"": ""j1"",
            ""20.3"": 1,
            ""102.4"": 1,
            ""-10.2"": 1
        }";

        const string contextJsonCB = 
        @"{
            ""GUser"": {
                ""id"": ""a"",
                ""major"": ""eng"",
                ""hobby"": ""hiking""
            },
            ""_multi"": [
                { ""TAction"": { ""a1"": ""f1"" } },
                { ""TAction"": { ""a2"": ""f2"" } }
            ]
        }";

        const string contextJsonCCB = @"
        {
        ""GUser"": {
            ""f1"": 82
        },
        ""_multi"": [
            {
            ""a1"": {
                ""af1"": 82
            }
            },
            {
            ""a2"": {
                ""af1"": 82
            }
            }
        ],
        ""_slots"": [
            {
            ""_id"": ""slot0""
            },
            {
            ""_id"": ""slot1""
            }
        ]
        }";

        const string contextJsonSlates = 
        @"{
            ""shared_feature"": 1.0,
            ""_multi"": [
                {""_slot_id"": 0, ""feature"": 1.0},
                {""_slot_id"": 0, ""feature"": 1.0},
                {""_slot_id"": 0, ""feature"": 1.0},
                {""_slot_id"": 1, ""feature"": 1.0},
                {""_slot_id"": 1, ""feature"": 1.0}
            ],
            ""_slots"": [
                {""feature"": 1.0},
                {""feature"": 1.0}
            ]
        }";


        [TestMethod]
        public void Test_CALoop()
        {
            const float outcome = 1.0f;
            const string eventId = "event_id";
            string configJsonCA = configJson.TrimEnd('}', '\r', '\n', ' ', '\t') +
                                @", ""model.vw.initial_command_line"": ""--quiet --cats 4 --min_value=185 --max_value=23959 --bandwidth 3000"" }";

            CALoop ca_loop = Helpers.CreateLoopOrExit<CALoop>(configJsonCA, config => new CALoop(config), true);

            ApiStatus apiStatus = new ApiStatus();

            ContinuousActionResponse continuousResponse = new ContinuousActionResponse();
            if (!ca_loop.TryRequestContinuousAction(eventId, contextJsonCA, continuousResponse, apiStatus))
            {
                Helpers.WriteStatusAndExit(apiStatus);
            }

            Assert.AreEqual(188.5713, continuousResponse.ChosenAction, .001);
            Assert.AreEqual(0, continuousResponse.ChosenActionPdfValue, .001);

            if (!ca_loop.TryQueueOutcomeEvent(eventId, outcome, apiStatus))
            {
                Helpers.WriteStatusAndExit(apiStatus);
            }
        }

        [TestMethod]
        public void Test_CBLoop()
        {
            const float outcome = 1.0f;
            const string eventId = "event_id";
            string configJsonCB = configJson.TrimEnd('}', '\r', '\n', ' ', '\t') +
                                @", ""model.vw.initial_command_line"": ""--quiet --cb_explore_adf"" }";

            CBLoop cb_loop = Helpers.CreateLoopOrExit<CBLoop>(configJsonCB, config => new CBLoop(config), true);
            ApiStatus apiStatus = new ApiStatus();

            RankingResponse rankingResponse = new RankingResponse();
            if (!cb_loop.TryChooseRank(eventId, contextJsonCB, rankingResponse, apiStatus))
            {
                Helpers.WriteStatusAndExit(apiStatus);
            }

            long actionId;
            if (!rankingResponse.TryGetChosenAction(out actionId, apiStatus))
            {
                Helpers.WriteStatusAndExit(apiStatus);
            }

            Assert.AreEqual(0, actionId);

            if (!cb_loop.TryQueueOutcomeEvent(eventId, outcome, apiStatus))
            {
                Helpers.WriteStatusAndExit(apiStatus);
            }
        }

        [TestMethod]
        public void Test_CCBLoop()
        {
            const float outcome = 1.0f;
            const string eventId = "event_id";
            string configJsonCCB = configJson.TrimEnd('}', '\r', '\n', ' ', '\t') +
                                @", ""model.vw.initial_command_line"": ""--quiet --ccb_explore_adf"" }";

            CCBLoop ccb_loop = Helpers.CreateLoopOrExit<CCBLoop>(configJsonCCB, config => new CCBLoop(config), true);

            ApiStatus apiStatus = new ApiStatus();

            MultiSlotResponseDetailed multiResponse = new MultiSlotResponseDetailed();
            if (!ccb_loop.TryRequestMultiSlotDecisionDetailed(eventId, contextJsonCCB, multiResponse, apiStatus))
            {
                Helpers.WriteStatusAndExit(apiStatus);
            }

            // TODO: Populate actionProbs. Currently GetOutcome() just returns a fixed outcome value, so the values of actionProbs don't matter.
            ActionProbability[] actionProbs = new ActionProbability[multiResponse.Count];
            // Define the expected sequences
            string[] expectedSlotIds = new string[] { "slot0", "slot1" };
            int[] expectedChosenActions = new int[] { 1, 0 };

            int index = 0;

            foreach (var slot in multiResponse)
            {
                if (index >= expectedSlotIds.Length)
                {
                    Assert.Fail("The response contains more items than expected.");
                }

                // Assert the expected values
                Assert.AreEqual(expectedSlotIds[index], slot.SlotId);
                Assert.AreEqual(expectedChosenActions[index], slot.ChosenAction);

                if (!ccb_loop.TryQueueOutcomeEvent(eventId, slot.SlotId, outcome, apiStatus))
                {
                    Helpers.WriteStatusAndExit(apiStatus);
                }
                index++;
            }
        }

        [TestMethod]
        public void Test_SlatesLoop()
        {
            const float outcome = 1.0f;
            const string eventId = "event_id";
            string configJsonSlates = configJson.TrimEnd('}', '\r', '\n', ' ', '\t') +
                                @", ""model.vw.initial_command_line"": ""--quiet --slates"" }";

            SlatesLoop slates_loop = Helpers.CreateLoopOrExit<SlatesLoop>(configJsonSlates, config => new SlatesLoop(config), true);

            ApiStatus apiStatus = new ApiStatus();

            // TODO: Fix VW slates json parser to test usage
            /*MultiSlotResponseDetailed multiResponse = new MultiSlotResponseDetailed();
            if (!slates_loop.TryRequestMultiSlotDecisionDetailed(eventId, contextJsonSlates, multiResponse, apiStatus))
            {
                Helpers.WriteStatusAndExit(apiStatus);
            }*/
        }
    }
}
