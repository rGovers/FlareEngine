using System.Collections.Generic;
using System.IO;

namespace FlareEngine.Mod
{
    public static class ModControl
    {
        static FlareAssembly CoreAssembly;

        static List<FlareAssembly> Assemblies;

        internal static void Init(string a_workingDir)
        {
            Assemblies = new List<FlareAssembly>();

            if (!string.IsNullOrWhiteSpace(a_workingDir))
            {
                CoreAssembly = FlareAssembly.GetFlareAssembly(Path.Combine(a_workingDir, "Core"));
            }
            else
            {
                CoreAssembly = FlareAssembly.GetFlareAssembly("./Core/");       
            }

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
            Profiler.StartFrame("Assembly Update");

            CoreAssembly.AssemblyControl.Update();

            foreach (FlareAssembly asm in Assemblies)
            {
                if (asm.AssemblyControl != null)
                {
                    asm.AssemblyControl.Update();
                }
            }

            Profiler.StopFrame();
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