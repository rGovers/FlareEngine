using FlareEngine.Definitions;
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FlareEngine.Rendering
{
    [StructLayout(LayoutKind.Sequential, Pack = 0)]
    internal struct RenderProgram
    {
        public uint VertexShader;
        public uint PixelShader;
        public uint RenderLayer;
    };

    public class Material : IDisposable
    {
        public MaterialDef Def = null;

        bool m_disposed = false;

        uint m_bufferAddr;

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static uint GenerateProgram(uint a_vertexShader, uint a_pixelShader); 
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static RenderProgram GetProgramBuffer(uint a_addr); 
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static void SetProgramBuffer(uint a_addr, RenderProgram a_program);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static void DestroyProgram(uint a_addr); 

        public uint RenderLayer
        {
            get
            {
                return GetProgramBuffer(m_bufferAddr).RenderLayer;
            }
            set
            {
                RenderProgram val = GetProgramBuffer(m_bufferAddr);

                val.RenderLayer = value;

                SetProgramBuffer(m_bufferAddr, val);
            }
        }

        public Material(VertexShader a_vertexShader, PixelShader a_pixelShader)
        {
            m_bufferAddr = GenerateProgram(a_vertexShader.InternalAddr, a_pixelShader.InternalAddr);

            RenderLayer = 0b1;
        }

        public static Material FromDef(MaterialDef a_def)
        {
            if (a_def.PixelShader == null || a_def.VertexShader == null)
            {
                return null;
            }

            return new Material(a_def.VertexShader, a_def.PixelShader)
            {
                Def = a_def,
                RenderLayer = a_def.RenderLayer
            };
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
                    DestroyProgram(m_bufferAddr);
                }
                else
                {
                    Console.Error.WriteLine("FlareCS: Material Failed to Dispose");
                }

                m_disposed = true;
            }
            else
            {
                Console.Error.WriteLine("FlareCS: Multiple Material Dispose");
            }
        }

        ~Material()
        {
            Dispose(false);
        }
    };
}