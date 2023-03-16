using System.Collections.Generic;
using System.IO;

namespace FlareEngine.Mod
{
    public static class ModControl
    {
        public static FlareAssembly CoreAssembly
        {
            get;
            private set;
        }

        public static List<FlareAssembly> Assemblies
        {
            get;
            private set;
        }

        internal static void Init(string a_workingDir)
        {
            Assemblies = new List<FlareAssembly>();

            if (!string.IsNullOrWhiteSpace(a_workingDir))
            {
                CoreAssembly = FlareAssembly.GetFlareAssembly(Path.Combine(a_workingDir, "Core"));
            }
            else
            {
                CoreAssembly = FlareAssembly.GetFlareAssembly(Path.Combine(Directory.GetCurrentDirectory(), "Core"));       
            }
        }

        internal static void InitAssemblies()
        {
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

        public static string GetAssetPath(string a_path)
        {
            if (File.Exists(a_path))
            {
                return a_path;
            }

            foreach (FlareAssembly asm in Assemblies)
            {
                string mPath = Path.Combine(asm.AssemblyInfo.Path, "Assets", a_path);
                if (File.Exists(mPath))
                {
                    return mPath;
                }
            }

            string cPath = Path.Combine(CoreAssembly.AssemblyInfo.Path, "Assets", a_path);
            if (File.Exists(cPath))
            {
                return cPath;
            }

            return null;
        } 
    }
}