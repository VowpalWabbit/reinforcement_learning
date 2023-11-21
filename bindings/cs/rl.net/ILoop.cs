using System;

namespace Rl.Net
{
    public interface ILoop
    {
        bool TryInit(ApiStatus apiStatus = null);
        void Init();
        bool TryRefreshModel(ApiStatus apiStatus = null);
        void RefreshModel();
        event EventHandler<ApiStatus> BackgroundError;
        event EventHandler<TraceLogEventArgs> TraceLoggerEvent;
    }
}