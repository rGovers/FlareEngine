using System.Collections.Generic;

namespace FlareEngine.Mod
{
    public static class ModControl
    {
        static FlareAssembly CoreAssembly;

        static List<FlareAssembly> Assemblies;

        internal static void Init()
        {
            Assemblies = new List<FlareAssembly>();

            CoreAssembly = FlareAssembly.GetFlareAssembly("./Core/");       
            CoreAssembly.AssemblyControl.Init();

            foreach (FlareAssembly asm in Assemblies)
            {
                if (asm.AssemblyControl != null)
                {
                    asm.AssemblyControl.Init();
                }
            }
        }

        internal static void Update()
        {
            CoreAssembly.AssemblyControl.Update();

            foreach (FlareAssembly asm in Assemblies)
            {
                if (asm.AssemblyControl != null)
                {
                    asm.AssemblyControl.Update();
                }
            }
        }

        internal static void Close()
        {
            CoreAssembly.AssemblyControl.Close();

            foreach (FlareAssembly asm in Assemblies)
            {
                if (asm.AssemblyControl != null)
                {
                    asm.AssemblyControl.Close();
                }
            }

            Assemblies.Clear();
        }
    }
}