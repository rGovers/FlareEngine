using System.Runtime.CompilerServices;

namespace FlareEngine
{
    public static class Profiler
    {   
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void StartFrame(string a_frameName);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void StopFrame();
    };
}