using System;

namespace FlareEngine.Rendering
{
    public interface IRenderTexture : IDisposable
    {
        uint Width
        {
            get;
        }
        uint Height
        {
            get;
        }

        bool HasDepth
        {
            get;
        }

        void Resize(uint a_width, uint a_height);
    }
}