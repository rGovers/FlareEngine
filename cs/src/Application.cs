using System.Runtime.CompilerServices;

namespace FlareEngine
{
    public static class Application
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetWidth();
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetHeight();
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void Close(); 

        public static uint Width
        {
            get
            {
                return GetWidth();
            }
        }
        public static uint Height
        {
            get
            {
                return GetHeight();
            }
        }
    }
}