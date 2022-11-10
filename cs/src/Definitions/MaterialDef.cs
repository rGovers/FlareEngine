using System;
using System.Collections.Generic;
using System.Reflection;
using FlareEngine.Rendering;

namespace FlareEngine.Definitions
{
    public class MaterialDef : Def
    {
        public string VertexShaderPath;
        public string PixelShaderPath;
        public uint RenderLayer = 0b1;

        public Type VertexType = typeof(Vertex);

        public List<VertexInputAttribute> VertexAttributes = null;

        public List<ShaderBufferInput> ShaderBuffers = null;

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