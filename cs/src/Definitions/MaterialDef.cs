using FlareEngine.Rendering;

namespace FlareEngine.Definitions
{
    public class MaterialDef : Def
    {
        public VertexShader VertexShader;
        public PixelShader PixelShader;
        public uint RenderLayer = 0b1;
    }
}