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

        internal static void Init()
        {
            Assemblies = new List<FlareAssembly>();

            bool working = !string.IsNullOrWhiteSpace(Application.WorkingDirectory);

            if (working)
            {
                CoreAssembly = FlareAssembly.GetFlareAssembly(Path.Combine(Application.WorkingDirectory, "Core"));
            }

            if (CoreAssembly == null)
            {
                CoreAssembly = FlareAssembly.GetFlareAssembly(Path.Combine(Directory.GetCurrentDirectory(), "Core"));       
            }

            if (CoreAssembly == null)
            {
                Logger.FlareError("Failed to load core assembly");
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

            for (int i = Assemblies.Count - 1; i >= 0; --i)
            {
                string mPath = Path.Combine(Assemblies[i].AssemblyInfo.Path, "Assets", a_path);
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
        public static string GetScenePath(string a_path, string a_modID)
        {
            if (a_modID == CoreAssembly.AssemblyInfo.ID)
            {
                return Path.Combine(CoreAssembly.AssemblyInfo.Path, "Scenes", a_path);
            }

            foreach (FlareAssembly asm in Assemblies)
            {
                if (asm.AssemblyInfo.ID == a_modID)
                {
                    return Path.Combine(asm.AssemblyInfo.ID, "Scenes", a_path);
                }
            }

            return null;
        }
        public static string GetScenePath(string a_path)
        {
            if (File.Exists(a_path))
            {
                return a_path;
            }

            for (int i = Assemblies.Count - 1; i >= 0; --i)
            {
                string mPath = Path.Combine(Assemblies[i].AssemblyInfo.Path, "Scenes", a_path);
                if (File.Exists(mPath))
                {
                    return mPath;
                }
            }

            string cPath = Path.Combine(CoreAssembly.AssemblyInfo.Path, "Scenes", a_path);
            if (File.Exists(cPath))
            {
                return cPath;
            }

            return null;
        }
    }
}