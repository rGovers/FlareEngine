using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FlareEngine.Rendering
{
    [StructLayout(LayoutKind.Sequential, Pack = 0)]
    internal struct CameraBuffer
    {
        public Viewport Viewport;
        public uint RenderLayer;
    };

    public class Camera : IDisposable
    {
        bool m_disposed = false;
        uint m_bufferAddr;

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static uint GenerateBuffer();
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static void DestroyBuffer(uint a_addr);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static CameraBuffer GetBuffer(uint a_addr);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static void SetBuffer(uint a_addr, CameraBuffer a_buffer);

        public Viewport Viewport
        {
            get
            {
                return GetBuffer(m_bufferAddr).Viewport;
            }
            set
            {
                CameraBuffer val = GetBuffer(m_bufferAddr);

                val.Viewport = value;

                SetBuffer(m_bufferAddr, val);
            }
        }

        public uint RenderLayer
        {
            get
            {
                return GetBuffer(m_bufferAddr).RenderLayer;
            }
            set
            {
                CameraBuffer val = GetBuffer(m_bufferAddr);

                val.RenderLayer = value;

                SetBuffer(m_bufferAddr, val);
            }
        }

        public Camera()
        {
            m_bufferAddr = GenerateBuffer();
            
            Viewport = new Viewport()
            {
                Position = Vector2.Zero,
                Size = Vector2.One * 55,
                MinDepth = 0.0f,
                MaxDepth = 1.0f
            };
            RenderLayer = 0b1;
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
                    Console.Error.WriteLine("FlareCS: Camera Failed to Dispose");
                }

                m_disposed = true;
            }
            else
            {
                Console.Error.WriteLine("FlareCS: Multiple Camera Dispose");
            }
        }

        ~Camera()
        {
            Dispose(false);
        }
    };
}