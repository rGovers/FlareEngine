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
        public ushort VertexStride;
        public ushort VertexInputCount;
        public IntPtr VertexInputAttributes;
        public ushort ShaderBufferInputCount;
        public IntPtr ShaderBufferInputs;
    };

    public enum ShaderBufferType : ushort
    {
        Camera = 0,
        Model = 1
    };

    public enum ShaderSlot : ushort
    {
        Null = ushort.MaxValue,
        Vertex = 0,
        Pixel = 1,
        All = 2
    };

    public struct ShaderBufferInput
    {   
        public uint Slot;
        public ShaderBufferType BufferType;
        public ShaderSlot ShaderSlot;
    };

    public class Material : IDisposable
    {
        MaterialDef m_def;

        bool        m_disposed = false;

        uint        m_bufferAddr;

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static uint GenerateProgram(uint a_vertexShader, uint a_pixelShader, ushort a_vertexStride, VertexInputAttribute[] a_attributes, ShaderBufferInput[] a_shaderInputs); 
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static RenderProgram GetProgramBuffer(uint a_addr); 
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static void SetProgramBuffer(uint a_addr, RenderProgram a_program);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static void DestroyProgram(uint a_addr); 

        internal uint InternalAddr
        {
            get
            {
                return m_bufferAddr;
            }
        }

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
        
        public MaterialDef Def
        {
            get
            {
                return m_def;
            }
        }

        public Material(VertexShader a_vertexShader, PixelShader a_pixelShader, ushort a_vertexStride, VertexInputAttribute[] a_attributes, ShaderBufferInput[] a_shaderInputs)
        {
            m_bufferAddr = GenerateProgram(a_vertexShader.InternalAddr, a_pixelShader.InternalAddr, a_vertexStride, a_attributes, a_shaderInputs);

            RenderLayer = 0b1;
        }

        public static Material FromDef(MaterialDef a_def)
        {
            if (string.IsNullOrWhiteSpace(a_def.PixelShaderPath) || string.IsNullOrWhiteSpace(a_def.VertexShaderPath))
            {
                return null;
            }

            ShaderBufferInput[] shaderInput = null;
            if (a_def.ShaderBuffers != null)
            {
                shaderInput = a_def.ShaderBuffers.ToArray();
            }

            VertexInputAttribute[] vertexInputAttributes = null;
            if (a_def.VertexAttributes != null)
            {
                vertexInputAttributes = a_def.VertexAttributes.ToArray();
            }

            return new Material(AssetLibrary.LoadVertexShader(a_def.VertexShaderPath), AssetLibrary.LoadPixelShader(a_def.PixelShaderPath), (ushort)Marshal.SizeOf(a_def.VertexType), vertexInputAttributes, shaderInput)
            {
                m_def = a_def,
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
                    Logger.Error("FlareCS: Material Failed to Dispose");
                }

                m_disposed = true;
            }
            else
            {
                Logger.Error("FlareCS: Multiple Material Dispose");
            }
        }

        ~Material()
        {
            Dispose(false);
        }
    };
}