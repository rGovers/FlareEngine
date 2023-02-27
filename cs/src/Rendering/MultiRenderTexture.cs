using System;
using System.Runtime.CompilerServices;

namespace FlareEngine.Rendering
{
    public class MultiRenderTexture : IRenderTexture
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

        public uint TextureCount
        {
            get
            {
                return GetTextureCount(m_bufferAddr);
            }
        }

        public bool HasDepth
        {
            get
            {
                return RenderTextureCmd.HasDepth(m_bufferAddr) != 0;
            }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetTextureCount(uint a_addr);

        public MultiRenderTexture(uint a_count, uint a_width, uint a_height, bool a_depth = false, bool a_hdr = false)
        {
            uint hdrVal = 0;
            if (a_hdr)
            {
                hdrVal = 1;
            }
            
            uint depthVal = 0;
            if (a_depth)
            {
                depthVal = 1;
            }

            m_bufferAddr = RenderTextureCmd.GenerateRenderTexture(a_count, a_width, a_height, depthVal, hdrVal);

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
                    Logger.Error("FlareCS: MultiRenderTexture Failed to Dispose");
                }

                m_disposed = true;
            }
            else
            {
                Logger.Error("FlareCS: Multiple MultiRenderTexture Dispose");
            }
        }

        ~MultiRenderTexture()
        {
            Dispose(false);
        }

        public void Resize(uint a_width, uint a_height)
        {
            RenderTextureCmd.Resize(m_bufferAddr, a_width, a_height);
        }
    }
}