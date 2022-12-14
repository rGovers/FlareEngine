using FlareEngine.Maths;
using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FlareEngine
{
    [StructLayout(LayoutKind.Sequential, Pack = 0)]
    internal struct TransformBuffer
    {
        public uint ParentIndex;

        public Vector3 Translation;
        public Quaternion Rotation;
        public Vector3 Scale;
    }

    public class Transform : IDisposable
    {
        bool            m_disposed = false;
           
        uint            m_bufferAddr;
      
        Object          m_object;

        Transform       m_parent;
        List<Transform> m_children;

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static uint GenerateTransformBuffer();
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static TransformBuffer GetTransformBuffer(uint a_addr);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static void SetTransformBuffer(uint a_addr, TransformBuffer a_buffer);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static void DestroyTransformBuffer(uint a_addr); 

        internal uint InternalAddr
        {
            get
            {
                return m_bufferAddr;
            }
        }

        public Transform Parent
        {
            get
            {
                return m_parent;
            }
            set
            {
                if (m_parent != null)
                {
                    m_parent.m_children.Remove(this);
                }

                m_parent = value;

                TransformBuffer buffer = GetTransformBuffer(m_bufferAddr);

                if (m_parent != null)
                {
                    m_parent.m_children.Add(this);

                    buffer.ParentIndex = m_parent.m_bufferAddr;
                }
                else
                {
                    buffer.ParentIndex = uint.MaxValue;
                }

                SetTransformBuffer(m_bufferAddr, buffer);
            }
        }

        public IEnumerable<Transform> Children
        {
            get
            {
                return m_children;
            }
        }

        public Object Object
        {
            get
            {
                return m_object;
            }
        }

        public Vector3 Translation
        {
            get
            {
                return GetTransformBuffer(m_bufferAddr).Translation;
            }
            set
            {
                TransformBuffer buffer = GetTransformBuffer(m_bufferAddr);
                buffer.Translation = value;
                SetTransformBuffer(m_bufferAddr, buffer);
            }
        }

        public Quaternion Rotation
        {
            get
            {
                return GetTransformBuffer(m_bufferAddr).Rotation;
            }
            set
            {
                TransformBuffer buffer = GetTransformBuffer(m_bufferAddr);
                buffer.Rotation = value;
                SetTransformBuffer(m_bufferAddr, buffer);
            }
        }

        public Vector3 Scale
        {
            get
            {
                return GetTransformBuffer(m_bufferAddr).Scale;
            }
            set
            {
                TransformBuffer buffer = GetTransformBuffer(m_bufferAddr);
                buffer.Scale = value;
                SetTransformBuffer(m_bufferAddr, buffer);
            }
        }

        internal Transform(Object a_object)
        {
            m_object = a_object;

            m_bufferAddr = GenerateTransformBuffer();

            Translation = Vector3.Zero;
            Rotation = Quaternion.Identity;
            Scale = Vector3.One;

            m_parent = null;
            m_children = new List<Transform>();
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
                    DestroyTransformBuffer(m_bufferAddr);
                }
                else
                {
                    Logger.Error("FlareCS: Transform Failed to Dispose");
                }

                m_disposed = true;
            }
            else
            {
                Logger.Error("FlareCS: Multiple Transform Dispose");
            }
        }

        ~Transform()
        {
            Dispose(false);
        }
    }
}