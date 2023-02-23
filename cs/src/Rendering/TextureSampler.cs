using System;
using System.Runtime.CompilerServices;

namespace FlareEngine.Rendering
{
    public enum TextureFilter : ushort
    {
        Nearest = 0,
        Linear = 1
    }
    public enum TextureAddress : ushort
    {
        Repeat = 0,
        MirroredRepeat = 1,
        ClampToEdge = 2
    };

    public class TextureSampler : IDisposable
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateRenderTextureSampler(uint a_renderTexture, uint a_textureIndex, uint a_filter, uint a_addressMode);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroySampler(uint a_addr);

        bool          m_disposed = false;

        readonly uint m_bufferAddr;

        internal uint BufferAddr
        {
            get
            {
                return m_bufferAddr;
            }
        }

        TextureSampler(uint a_bufferAddr)
        {
            m_bufferAddr = a_bufferAddr;
        }

        public static TextureSampler GenerateRenderTextureSampler(RenderTexture a_renderTexture, TextureFilter a_filter = TextureFilter.Linear, TextureAddress a_addressMode = TextureAddress.Repeat)
        {
            if (a_renderTexture == null)
            {
                Logger.FlareWarning("GenerateRenderTextureSampler null RenderTexture");

                return null;
            }

            return new TextureSampler(GenerateRenderTextureSampler(a_renderTexture.BufferAddr, 0, (uint)a_filter, (uint)a_addressMode));
        }
        public static TextureSampler GenerateRenderTextureSampler(MultiRenderTexture a_renderTexture, uint a_index, TextureFilter a_filter = TextureFilter.Linear, TextureAddress a_addressMode = TextureAddress.Repeat)
        {
            if (a_renderTexture == null)
            {
                Logger.FlareWarning("GenerateRenderTextureSampler null RenderTexture");

                return null;
            }

            return new TextureSampler(GenerateRenderTextureSampler(a_renderTexture.BufferAddr, a_index, (uint)a_filter, (uint)a_addressMode));
        }

        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool a_disposing)
        {
            if(!m_disposed)
            {
                if(a_disposing)
                {
                    DestroySampler(m_bufferAddr);
                }
                else
                {
                    Logger.FlareWarning("TextureSampler Failed to Dispose");
                }

                m_disposed = true;
            }
            else
            {
                Logger.FlareError("Multiple TextureSampler Dispose");
            }
        }

        ~TextureSampler()
        {
            Dispose(false);
        }
    }
}