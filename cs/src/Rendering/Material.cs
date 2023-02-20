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
        CullMode CullingMode;
        PrimitiveMode PrimitiveMode;
        public byte Flags;
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

    public enum CullMode : ushort
    {
        None = 0,
        Front = 1,
        Back = 2,
        Both = 3
    };
    public enum PrimitiveMode : ushort
    {
        Triangles = 0,
        TriangleStrip = 1
    };

    internal enum InternalRenderProgram : ushort
    {
        DirectionalLight = 0
    }

    public struct ShaderBufferInput
    {   
        public uint Slot;
        public ShaderBufferType BufferType;
        public ShaderSlot ShaderSlot;
    };

    public class Material : IDisposable
    {
        public static Material DirectionalLightMaterial = null;

        MaterialDef   m_def = null;

        bool          m_disposed = false;

        readonly uint m_bufferAddr;

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateInternalProgram(InternalRenderProgram a_renderProgram);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateProgram(uint a_vertexShader, uint a_pixelShader, ushort a_vertexStride, VertexInputAttribute[] a_attributes, ShaderBufferInput[] a_shaderInputs, uint a_cullMode, uint a_primitiveMode); 
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static RenderProgram GetProgramBuffer(uint a_addr); 
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetProgramBuffer(uint a_addr, RenderProgram a_program);
        [MethodImpl(MethodImplOptions.InternalCall)]
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

        internal static void Init()
        {
            DirectionalLightMaterial = new Material(InternalRenderProgram.DirectionalLight);
        }
        internal static void Destroy()
        {
            DirectionalLightMaterial.Dispose();
        }

        Material(InternalRenderProgram a_program)
        {
            m_bufferAddr = GenerateInternalProgram(a_program);

            RenderLayer = 0b1;
        }
        public Material(VertexShader a_vertexShader, PixelShader a_pixelShader, ushort a_vertexStride, VertexInputAttribute[] a_attributes, ShaderBufferInput[] a_shaderInputs, CullMode a_cullMode = CullMode.Back, PrimitiveMode a_primitiveMode = PrimitiveMode.Triangles)
        {
            m_bufferAddr = GenerateProgram(a_vertexShader.InternalAddr, a_pixelShader.InternalAddr, a_vertexStride, a_attributes, a_shaderInputs, (uint)a_cullMode, (uint)a_primitiveMode);

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

            return new Material(AssetLibrary.LoadVertexShader(a_def.VertexShaderPath), AssetLibrary.LoadPixelShader(a_def.PixelShaderPath), (ushort)Marshal.SizeOf(a_def.VertexType), vertexInputAttributes, shaderInput, a_def.CullingMode, a_def.PrimitiveMode)
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
                    Logger.FlareWarning("Material Failed to Dispose");
                }

                m_disposed = true;
            }
            else
            {
                Logger.FlareError("Multiple Material Dispose");
            }
        }

        ~Material()
        {
            Dispose(false);
        }
    };
}