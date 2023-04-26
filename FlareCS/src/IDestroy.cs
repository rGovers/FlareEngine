using System;

namespace FlareEngine
{
    public interface IDestroy : IDisposable
    {
        bool IsDisposed
        {
            get;
        }
    }
}