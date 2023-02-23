using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using FlareEngine.Definitions;
using FlareEngine.Maths;

namespace FlareEngine.Rendering.Lighting
{
    [StructLayout(LayoutKind.Sequential, Pack = 0)]
    internal struct DirectionalLightBuffer
    {
        public uint RenderLayer;
        public uint TransformAddr;
        public Vector4 Color;
        public float Intensity;
    }

    public class DirectionalLight : Light, IDisposable
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateBuffer(uint a_transformAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static DirectionalLightBuffer GetBuffer(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetBuffer(uint a_addr, DirectionalLightBuffer a_buffer);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyBuffer(uint a_addr); 

        bool m_disposed = false;
        uint m_bufferAddr;

        public override LightType LightType
        {
            get
            {
                return LightType.Directional;
            }
        }

        public DirectionalLightDef DirectionalLightDef
        {
            get
            {
                return Def as DirectionalLightDef;
            }
        }

        public uint RenderLayer
        {
            get
            {
                DirectionalLightBuffer buffer = GetBuffer(m_bufferAddr);

                return buffer.RenderLayer;
            }
            set
            {
                DirectionalLightBuffer buffer = GetBuffer(m_bufferAddr);

                buffer.RenderLayer = value;

                SetBuffer(m_bufferAddr, buffer);
            }
        }

        public Vector4 Color
        {
            get
            {
                DirectionalLightBuffer buffer = GetBuffer(m_bufferAddr);

                return buffer.Color;
            }
            set
            {
                DirectionalLightBuffer buffer = GetBuffer(m_bufferAddr);

                buffer.Color = value;

                SetBuffer(m_bufferAddr, buffer);
            }
        }

        public float Intensity
        {
            get
            {
                DirectionalLightBuffer buffer = GetBuffer(m_bufferAddr);

                return buffer.Intensity;
            }
            set
            {
                DirectionalLightBuffer buffer = GetBuffer(m_bufferAddr);

                buffer.Intensity = value;

                SetBuffer(m_bufferAddr, buffer);
            }
        }

        public override void Init()
        {
            base.Init();

            m_bufferAddr = GenerateBuffer(Transform.InternalAddr);

            DirectionalLightBuffer buffer = GetBuffer(m_bufferAddr);

            LightDef def = LightDef;
            if (def != null)
            {
                buffer.RenderLayer = def.RenderLayer;
                buffer.Color = def.Color;
                buffer.Intensity = def.Intensity;
            }
            else
            {
                buffer.RenderLayer = 0b1;
                buffer.Color = Vector4.One;
                buffer.Intensity = 10.0f;
            }

            SetBuffer(m_bufferAddr, buffer);
        }

        ~DirectionalLight()
        {
            Dispose(false);
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
                    DestroyBuffer(m_bufferAddr);
                }
                else
                {
                    Logger.FlareWarning("DirectionalLight Failed to Dispose");
                }

                m_disposed = true;
            }
            else
            {
                Logger.FlareError("Multiple DirectionalLight Dispose");
            }
        }
    }
}