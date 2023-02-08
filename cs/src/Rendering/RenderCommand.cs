using System.Runtime.CompilerServices;

namespace FlareEngine.Rendering
{
    public static class RenderCommand
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void BindRenderTexture(uint a_addr);

        public static void BindRenderTexture(IRenderTexture a_renderTexture)
        {
            if (a_renderTexture == null)
            {
                BindRenderTexture(uint.MaxValue);
            }
            else if (a_renderTexture is RenderTexture rVal)
            {
                BindRenderTexture(rVal.BufferAddr);
            }
            else if (a_renderTexture is MultiRenderTexture mVal)
            {
                BindRenderTexture(mVal.BufferAddr);
            }
        }
    }
}