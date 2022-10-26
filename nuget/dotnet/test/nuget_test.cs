using System;
using System.Diagnostics;
using Rl.Net;
using System;
using System.IO;

Console.WriteLine("Running RL C# test...");
var test = new RLTest();
test.RLTestSimple();
Console.WriteLine("Done");

public class RLTest
{
    public void RLTestSimple()
    {
      ApiStatus apiStatus = new ApiStatus();
      Configuration config;
        if (!Configuration.TryLoadConfigurationFromJson("{}", out config, apiStatus))
        {
            Console.Error.WriteLine(apiStatus.ErrorMessage);
            Environment.Exit(1);
        }
    RankingResponse rankingResponse = new RankingResponse();
   }
}

