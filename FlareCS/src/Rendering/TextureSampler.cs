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

    // Type exists for Vulkan in the engine
    // May not work properly in the editor due to editor using OpenGL
    public class TextureSampler : IDestroy
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateTextureSampler(uint a_texture, uint a_filter, uint a_addressMode);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateRenderTextureSampler(uint a_renderTexture, uint a_textureIndex, uint a_filter, uint a_addressMode);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateRenderTextureDepthSampler(uint a_renderTexture, uint a_filter, uint a_addressMode);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroySampler(uint a_addr);

        uint m_bufferAddr = uint.MaxValue;

        public bool IsDisposed
        {
            get
            {
                return m_bufferAddr == uint.MaxValue;
            }
        }

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

        public static TextureSampler GeneretateTextureSampler(Texture a_texture, TextureFilter a_filter = TextureFilter.Linear, TextureAddress a_addressMode = TextureAddress.Repeat)
        {
            if (a_texture == null)
            {
                Logger.FlareWarning("GeneretateTextureSampler null Texture");

                return null;
            }

            return new TextureSampler(GenerateTextureSampler(a_texture.BufferAddr, (uint)a_filter, (uint) a_addressMode));
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

        public static TextureSampler GenerateRenderTextureDepthSampler(IRenderTexture a_renderTexture, TextureFilter a_filter = TextureFilter.Linear, TextureAddress a_addressMode = TextureAddress.Repeat)
        {
            if (a_renderTexture == null)
            {
                Logger.FlareWarning("GenerateRenderTextureDepthSampler null RenderTexture");

                return null;
            }
            
            if (!a_renderTexture.HasDepth)
            {
                Logger.FlareWarning("GenerateRenderTextureDepthSampler no depth on render texture");

                return null;
            }

            return new TextureSampler(GenerateRenderTextureDepthSampler(RenderTextureCmd.GetTextureAddr(a_renderTexture), (uint)a_filter, (uint)a_addressMode));
        }

        public override bool Equals(object a_obj)
        {
            if (a_obj == null && m_bufferAddr == uint.MaxValue)
            {
                return true;
            }

            return base.Equals(a_obj);
        }
        public override int GetHashCode()
        {
            return base.GetHashCode();
        }

        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool a_disposing)
        {
            if(m_bufferAddr != uint.MaxValue)
            {
                if(a_disposing)
                {
                    DestroySampler(m_bufferAddr);
                }
                else
                {
                    Logger.FlareWarning("TextureSampler Failed to Dispose");
                }

                m_bufferAddr = uint.MaxValue;
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