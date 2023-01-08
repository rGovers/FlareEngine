using FlareEngine.Definitions;
using FlareEngine.Maths;
using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FlareEngine.Rendering
{
    [StructLayout(LayoutKind.Sequential, Pack = 0)]
    internal struct CameraBuffer
    {
        public uint TransformBuffer;
        public Viewport Viewport;
        public uint RenderLayer;
        public float FOV;
        public float Near;
        public float Far;
    };

    public class Camera : Object
    {
        static Dictionary<uint, Camera> BufferLookup = new Dictionary<uint, Camera>();

        bool          m_disposed = false;
        readonly uint m_bufferAddr;

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateBuffer(uint a_transformAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyBuffer(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static CameraBuffer GetBuffer(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
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

        public float FOV
        {
            get
            {
                return GetBuffer(m_bufferAddr).FOV;
            }
            set
            {
                CameraBuffer val = GetBuffer(m_bufferAddr);

                val.FOV = value;

                SetBuffer(m_bufferAddr, val);
            }
        }
        public float Near
        {
            get
            {
                return GetBuffer(m_bufferAddr).Near;
            }
            set
            {
                CameraBuffer val = GetBuffer(m_bufferAddr);

                val.Near = value;

                SetBuffer(m_bufferAddr, val);
            }
        }
        public float Far
        {
            get
            {
                return GetBuffer(m_bufferAddr).Far;
            }
            set
            {
                CameraBuffer val = GetBuffer(m_bufferAddr);

                val.Far = value;

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
            m_bufferAddr = GenerateBuffer(Transform.InternalAddr);
            
            CameraBuffer val = new CameraBuffer()
            {
                Viewport = new Viewport()
                {
                    Position = Vector2.Zero,
                    Size = Vector2.One,
                    MinDepth = 0.0f,
                    MaxDepth = 1.0f
                },
                FOV = (float)(Math.PI * 0.45),
                Near = 0.1f,
                Far = 100.0f,
                RenderLayer = 0b1
            };

            SetBuffer(m_bufferAddr, val);

            BufferLookup.Add(m_bufferAddr, this);
        }

        public override void Init()
        {
            base.Init();

            if (Def is CameraDef def)
            {
                CameraBuffer val = new CameraBuffer()
                {
                    Viewport = def.Viewport,
                    FOV = def.FOV,
                    Near = def.Near,
                    Far = def.Far,
                    RenderLayer = def.RenderLayer
                };

                SetBuffer(m_bufferAddr, val);
            }
        }

        internal static Camera GetCamera(uint a_buffer)
        {
            if (BufferLookup.ContainsKey(a_buffer))
            {
                return BufferLookup[a_buffer];
            }

            return null;
        }

        protected override void Dispose(bool a_disposing)
        {
            base.Dispose(a_disposing);

            if(!m_disposed)
            {
                if(a_disposing)
                {
                    DestroyBuffer(m_bufferAddr);

                    BufferLookup.Remove(m_bufferAddr);
                }
                else
                {
                    Logger.Error("FlareCS: Camera Failed to Dispose");
                }

                m_disposed = true;
            }
            else
            {
                Logger.Error("FlareCS: Multiple Camera Dispose");
            }
        }
    }}