namespace Rl.Net
{
    public class TraceLogEventArgs
    {
        public TraceLogEventArgs(int logLevel, string msg)
        {
            LogLevel = logLevel;
            Message = msg;
        }

        public int LogLevel { get; }
        public string Message { get; }
    }
}
