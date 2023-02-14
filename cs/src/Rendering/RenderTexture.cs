using System;
using System.Runtime.CompilerServices;

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

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateRenderTexture(uint a_width, uint a_height, uint a_depth, uint a_hdr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyRenderTexture(uint a_addr);

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

            m_bufferAddr = GenerateRenderTexture(a_width, a_height, depthVal, hdrVal);

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
                    RenderTextureCmd.DestroyRenderTexture(m_bufferAddr);

                    DestroyRenderTexture(m_bufferAddr);
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