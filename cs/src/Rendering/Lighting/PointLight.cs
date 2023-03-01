using FlareEngine.Definitions;
using FlareEngine.Maths;
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FlareEngine.Rendering.Lighting
{
    [StructLayout(LayoutKind.Sequential, Pack = 0)]
    internal struct PointLightBuffer
    {
        public uint TransformAddr;
        public uint RenderLayer;
        public Vector4 Color;
        public float Intensity;
        public float Radius;
    }

    public class PointLight : Light, IDisposable
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateBuffer(uint a_transformAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static PointLightBuffer GetBuffer(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetBuffer(uint a_addr, PointLightBuffer a_buffer);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyBuffer(uint a_addr);

        bool m_disposed = false;
        uint m_bufferAddr;

        public override LightType LightType
        {
            get
            {
                return LightType.Spot;
            }
        }

        public PointLightDef PointLightDef
        {
            get
            {
                return Def as PointLightDef;
            }
        }

        public override uint RenderLayer 
        { 
            get
            {
                PointLightBuffer buffer = GetBuffer(m_bufferAddr);

                return buffer.RenderLayer;   
            }
            set
            {
                PointLightBuffer buffer = GetBuffer(m_bufferAddr);

                buffer.RenderLayer = value;

                SetBuffer(m_bufferAddr, buffer);
            } 
        }

        public override Vector4 Color 
        {
            get
            {
                PointLightBuffer buffer = GetBuffer(m_bufferAddr);

                return buffer.Color;
            }
            set
            {
                PointLightBuffer buffer = GetBuffer(m_bufferAddr);

                buffer.Color = value;

                SetBuffer(m_bufferAddr, buffer);
            }
        }

        public override float Intensity 
        {
            get
            {
                PointLightBuffer buffer = GetBuffer(m_bufferAddr);
                
                return buffer.Intensity;
            }
            set
            {
                PointLightBuffer buffer = GetBuffer(m_bufferAddr);

                buffer.Intensity = value;

                SetBuffer(m_bufferAddr, buffer);
            }
        }

        public override void Init()
        {
            base.Init();

            m_bufferAddr = GenerateBuffer(Transform.InternalAddr);

            PointLightDef pointDef = PointLightDef;
            if (pointDef != null)
            {
                PointLightBuffer buffer = GetBuffer(m_bufferAddr);

                buffer.RenderLayer = pointDef.RenderLayer;
                buffer.Color = pointDef.Color;
                buffer.Intensity = pointDef.Intensity;
                buffer.Radius = pointDef.Radius;

                SetBuffer(m_bufferAddr, buffer);
            }
            else
            {
                LightDef lightDef = LightDef;
                if (lightDef != null)
                {
                    PointLightBuffer buffer = GetBuffer(m_bufferAddr);

                    buffer.RenderLayer = lightDef.RenderLayer;
                    buffer.Color = lightDef.Color;
                    buffer.Intensity = lightDef.Intensity;

                    SetBuffer(m_bufferAddr, buffer);
                }
            }
        }

        ~PointLight()
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
                    Logger.FlareWarning("PointLight Failed to Dispose");
                }

                m_disposed = true;
            }
            else
            {
                Logger.FlareError("Multiple PointLight Dispose");
            }
        }
    }
}