using System;
using System.Collections.Generic;
using System.Reflection;
using FlareEngine.Rendering;

namespace FlareEngine.Definitions
{
    public class MaterialDef : Def
    {
        [EditorTooltip("Path relative to the project for the vertex shader file to be used.")]
        public string VertexShaderPath;
        [EditorTooltip("Path relative to the project for the pixel shader file to be used.")]
        public string PixelShaderPath;
        [EditorTooltip("Used to determine if it will be rendered by a camera in a matching layer. Binary bit based.")]
        public uint RenderLayer = 0b1;

        public Type VertexType = typeof(Vertex);

        [EditorTooltip("Deterimine vertex data the shader uses for input.")]
        public List<VertexInputAttribute> VertexAttributes = null;
        
        [EditorTooltip("Used to determine input values for shaders.")]
        public List<ShaderBufferInput> ShaderBuffers = null;

        [EditorTooltip("Which faces to show when rendering.")]
        public CullMode CullingMode = CullMode.Back;

        public PrimitiveMode PrimitiveMode = PrimitiveMode.Triangles;

        [EditorTooltip("Enables color blending")]
        public bool EnableColorBlending = false;

        public override void PostResolve()
        {
            base.PostResolve();

            if (VertexType == null)
            {
                Logger.Error("FlareCS: Material Def Invalid VertexType");

                return;
            }

            if (VertexAttributes != null && VertexAttributes.Count > 0)
            {
                return;
            }

            MethodInfo methodInfo = VertexType.GetMethod("GetAttributes", BindingFlags.Public | BindingFlags.Static);
            if (methodInfo == null)
            {
                Logger.Error("FlareCS: Material Def no VertexAttributes");
            }

            VertexAttributes = new List<VertexInputAttribute>(methodInfo.Invoke(null, null) as VertexInputAttribute[]);
        }
    }
}