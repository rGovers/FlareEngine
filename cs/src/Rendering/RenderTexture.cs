using System;

namespace FlareEngine.Rendering
{
    public class RenderTexture : IRenderTexture
    {
        bool          m_disposed = false;
        readonly uint m_bufferAddr;

        internal uint BufferAddr
        {
            get
            {
                return m_bufferAddr;
            }
        }

        public uint Width
        {
            get
            {
                return RenderTextureCmd.GetWidth(m_bufferAddr);
            }
        }

        public uint Height
        {
            get
            {
                return RenderTextureCmd.GetHeight(m_bufferAddr);
            }
        }

        public RenderTexture(uint a_width, uint a_height, bool a_depth = false, bool a_hdr = false)
        {
            uint depthVal = 0;
            if (a_depth)
            {
                depthVal = 1;
            }
            
            uint hdrVal = 0;
            if (a_hdr)
            {
                hdrVal = 1;
            }

            m_bufferAddr = RenderTextureCmd.GenerateRenderTexture(1, a_width, a_height, depthVal, hdrVal);

            RenderTextureCmd.PushRenderTexture(m_bufferAddr, this);
        }

        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool a_disposing)
        {
            if (!m_disposed)
            {
                if (a_disposing)
                {
                    RenderTextureCmd.RemoveRenderTexture(m_bufferAddr);

                    RenderTextureCmd.DestroyRenderTexture(m_bufferAddr);
                }
                else
                {
                    Logger.FlareError("RenderTexture Failed to Dispose");
                }

                m_disposed = true;
            }
            else
            {
                Logger.FlareError("Multiple RenderTexture Dispose");
            }
        }

        public void Resize(uint a_width, uint a_height)
        {
            RenderTextureCmd.Resize(m_bufferAddr, a_width, a_height);
        }

        ~RenderTexture()
        {
            Dispose(false);
        }
    }
}