using FlareEngine.Maths;
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FlareEngine.Rendering
{
    [StructLayout(LayoutKind.Sequential, Pack = 0)]
    public struct Vertex
    {
        public Vector4 Position;
        public Vector3 Normal;
        public Vector4 Color;
        public Vector2 TexCoords;

        public static VertexInputAttribute[] GetAttributes()
        {
            VertexInputAttribute[] attributes = new VertexInputAttribute[4];
            attributes[0].Location = 0;
            attributes[0].Type = VertexType.Float;
            attributes[0].Count = 4;
            attributes[0].Offset = (uint)Marshal.OffsetOf<Vertex>("Position");

            attributes[1].Location = 1;
            attributes[1].Type = VertexType.Float;
            attributes[1].Count = 3;
            attributes[1].Offset = (uint)Marshal.OffsetOf<Vertex>("Normal");

            attributes[2].Location = 2;
            attributes[2].Type = VertexType.Float;
            attributes[2].Count = 4;
            attributes[2].Offset = (uint)Marshal.OffsetOf<Vertex>("Color");

            attributes[3].Location = 3;
            attributes[3].Type = VertexType.Float;
            attributes[3].Count = 2;
            attributes[3].Offset = (uint)Marshal.OffsetOf<Vertex>("TexCoords");

            return attributes;
        }
    }

    public enum VertexType : ushort
    {
        Float = 0,
        Int = 1,
        UInt = 2
    };

    [StructLayout(LayoutKind.Sequential, Pack = 0)]
    public struct VertexInputAttribute
    {
        public uint Location;
        public VertexType Type;
        public uint Count;
        public uint Offset;
    };

    public class Model : IDisposable
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static uint GenerateModel(Array a_vertices, uint[] a_indices, ushort a_vertexSize); 
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static void DestroyModel(uint a_addr);

        bool m_disposed = false;
        uint m_bufferAddr;

        internal uint InternalAddr
        {
            get
            {
                return m_bufferAddr;
            }
        }

        private Model()
        {

        }

        public static Model CreateModel<T>(T[] a_vertices, uint[] a_indices) where T : struct 
        {
            Model model = new Model();

            model.m_bufferAddr = GenerateModel(a_vertices, a_indices, (ushort)Marshal.SizeOf<T>());

            return model;
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
                    DestroyModel(m_bufferAddr);
                }
                else
                {
                    Logger.Error("FlareCS: Model Failed to Dispose");
                }

                m_disposed = true;
            }
            else
            {
                Logger.Error("FlareCS: Multiple Model Dispose");
            }
        }

        ~Model()
        {
            Dispose(false);
        }
    }
}