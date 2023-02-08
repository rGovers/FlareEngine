using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace FlareEngine.Rendering
{
    public static class RenderTextureCmd
    {
        static Dictionary<uint, IRenderTexture> RenderTextureTable;

        internal static void PushRenderTexture(uint a_addr, IRenderTexture a_renderTexture)
        {
            if (RenderTextureTable == null)
            {
                RenderTextureTable = new Dictionary<uint, IRenderTexture>();
            }   

            if (!RenderTextureTable.ContainsKey(a_addr))
            {
                RenderTextureTable.Add(a_addr, a_renderTexture);
            }
            else
            {
                Logger.FlareWarning($"RenderTexture exists at {a_addr}");

                RenderTextureTable[a_addr] = a_renderTexture;
            }
        } 
        internal static void DestroyRenderTexture(uint a_addr)
        {
            RenderTextureTable.Remove(a_addr);
        }

        internal static IRenderTexture GetRenderTexture(uint a_addr)
        {
            return RenderTextureTable[a_addr];
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern uint GetWidth(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern uint GetHeight(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Resize(uint a_addr, uint a_width, uint a_height);
    }
}