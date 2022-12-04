using FlareEngine.Definitions;
using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;

namespace FlareEngine.Mod
{
    public class FlareAssembly : IDisposable
    {
        bool              m_disposed = false;

        AssemblyControl   m_assemblyControl = null;

        FlareAssemblyInfo m_assemblyInfo;

        List<Assembly>    m_assemblies = null;

        internal AssemblyControl AssemblyControl
        {
            get
            {
                return m_assemblyControl;
            }
        }

        public FlareAssemblyInfo AssemblyInfo
        {
            get
            {
                return m_assemblyInfo;
            }
        }

        public IEnumerable<Assembly> Assemblies
        {
            get
            {
                return m_assemblies;
            }
        }

        private FlareAssembly()
        {

        }

        internal static FlareAssembly GetFlareAssembly(string a_path)
        {
            if (Directory.Exists(a_path))
            {
                FlareAssembly asm = new FlareAssembly();

                string assemblyPath = Path.Combine(a_path, "Assemblies");
                string defPath = Path.Combine(a_path, "Defs");
                if (Directory.Exists(defPath))
                {
                    DefLibrary.LoadDefs(defPath);
                }

                if (Directory.Exists(assemblyPath))
                {
                    asm.m_assemblies = new List<Assembly>();

                    string[] paths = Directory.GetFiles(assemblyPath);
                    foreach (string str in paths)
                    {
                        if (Path.GetExtension(str) == ".dll")
                        {
                            // Already loaded because we are it so can skip
                            // Some compilers like to add to the output for some reason
                            if (Path.GetFileNameWithoutExtension(str) == "FlareCS")
                            {
                                continue;
                            }

                            asm.m_assemblies.Add(Assembly.LoadFile(str));
                        }
                    }

                    foreach (Assembly assembly in asm.m_assemblies)
                    {
                        Type[] types = assembly.GetTypes();

                        foreach (Type type in types)
                        {
                            if (type.IsSubclassOf(typeof(AssemblyControl)))
                            {
                                asm.m_assemblyControl = Activator.CreateInstance(type) as AssemblyControl;

                                return asm;
                            }
                        }
                    }
                }

                return asm;
            }

            return null;
        }

        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool a_disposing)
        {
            if(!m_disposed)
            {
                if(a_disposing)
                {
                    foreach (Assembly asm in m_assemblies)
                    {
                        
                    }
                }
                else
                {
                    Logger.Error("FlareCS: FlareAssembly Failed to Dispose");
                }

                m_disposed = true;
            }
            else
            {
                Logger.Error("FlareCS: FlareAssembly Multi Dispose");
            }
        }

        ~FlareAssembly()
        {
            Dispose(false);
        }
    }
}