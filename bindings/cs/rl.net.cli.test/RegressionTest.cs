/*


        [TestMethod]
        public void Test_PPTDesigner()
        {
            
        }

*/

using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Rl.Net.Native;
using System.Runtime.InteropServices;

namespace Rl.Net.Cli.Test
{

    [TestClass]
    public class RegressionTest : TestBase
    {
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

        private const string CustomSenderConfigJson = 
@"{
    ""ApplicationID"": ""id1"",
    ""IsExplorationEnabled"": true,
    ""InitialExplorationEpsilon"": 1.0,
    ""model.source"": ""FILE_MODEL_DATA"",
    ""model.implementation"": ""VW"",
    ""model.backgroundrefresh"": false,
    ""observation.sender.implementation"": ""BINDING_SENDER"",
    ""interaction.sender.implementation"": ""BINDING_SENDER"",
    ""observation.send.batchintervalms"": 10,
    ""interaction.send.batchintervalms"": 10,
    ""model_file_loader.file_must_exist"": true
}
";
        private LiveModel CreateLiveModel( FactoryContext factoryContext )
        {
            Configuration config;
            ApiStatus apiStatus = new ApiStatus();
            if (!Configuration.TryLoadConfigurationFromJson( CustomSenderConfigJson, out config, apiStatus ) )
                Assert.Fail($"Failed to TryLoadConfigurationFromJson={apiStatus.ErrorMessage}");

            string a = config[ "model.source" ];

            // Adding the initial command for live model to config.
            config[ "vw.commandline" ] = "--cb_explore_adf --softmax --lambda 50 -q UA -c --coin --clip_p 0.5";
            config[ "model_file_loader.file_name" ] = Path.Combine( "test_data", "model_not_found.vw" );
       
            LiveModel liveModel = new LiveModel( config, factoryContext );
            return liveModel;
        }

        [TestMethod]
        public void Test_NoRegress_FileModelDataProvider()
        {
            if (!File.Exists("test_data/model.vw"))
            {
                Assert.Inconclusive("Could not find test data - unable to run test.");
            }

            FactoryContext factoryContext = new FactoryContext();
            factoryContext.SetSenderFactory((config, callback) => new MockSender() );

            LiveModel model = CreateLiveModel(factoryContext);
            
            model.Init();
        }
    }
}
