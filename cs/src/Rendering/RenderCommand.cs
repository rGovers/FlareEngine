using System.Runtime.CompilerServices;

namespace FlareEngine.Rendering
{
    public static class RenderCommand
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void BindMaterial(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void BindRenderTexture(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void RTRTBlit(uint a_srcAddr, uint a_dstAddr);

        public static void BindMaterial(Material a_material)
        {
            if (a_material != null)
            {
                BindMaterial(a_material.InternalAddr);
            }
            else
            {
                BindMaterial(uint.MaxValue);
            }
        }

        public static void BindRenderTexture(IRenderTexture a_renderTexture)
        {
            BindRenderTexture(RenderTextureCmd.GetTextureAddr(a_renderTexture));
        }
        public static void Blit(IRenderTexture a_srcTexture, IRenderTexture a_dstTexture)
        {
            RTRTBlit(RenderTextureCmd.GetTextureAddr(a_srcTexture), RenderTextureCmd.GetTextureAddr(a_dstTexture));
        }
    }
}