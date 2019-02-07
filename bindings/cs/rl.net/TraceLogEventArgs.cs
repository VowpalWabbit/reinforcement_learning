namespace Rl.Net
{
    public class TraceLogEventArgs
    {
        public TraceLogEventArgs(RLLogLevel logLevel, string msg)
        {
            LogLevel = logLevel;
            Message = msg;
        }

        public RLLogLevel LogLevel { get; }
        public string Message { get; }
    }
}
