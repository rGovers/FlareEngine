using FlareEngine.Definitions;
using FlareEngine.Mod;
using FlareEngine.Rendering;

namespace FlareEngine
{
    class Program
    {
        const string WorkingDirArg = "--wDir";

        static void Main(string[] a_args)
        {
            Logger.Message("FlareCS: Started");

            string workingDir = null;

            foreach (string arg in a_args)
            {
                if (arg.StartsWith(WorkingDirArg))
                {
                    workingDir = arg.Substring(WorkingDirArg.Length + 1);
                }
            }

            Time.Init();

            Material.Init();

            RenderPipeline.Init(new DefaultRenderPipeline());

            AssetLibrary.Init(workingDir);
            DefLibrary.Init();

            ModControl.Init(workingDir);

            DefLibrary.ResolveDefs();
            Scribe.SetLanguage("en-us");

            ModControl.InitAssemblies();

            Logger.Message("FlareCS: Initialized");
        }

        static void Shutdown()
        {
            ModControl.Close();

            DefLibrary.Clear();
            AssetLibrary.ClearAssets();

            GameObject.DestroyObjects();

            RenderPipeline.Destroy();

            Material.Destroy();

            Logger.Message("FlareCS: Shutdown");
        }

        static void Update(double a_delta, double a_time)
        {
            Time.DDeltaTime = a_delta;
            Time.DTimePassed = a_time;

            ModControl.Update();

            GameObject.UpdateObjects();
            GameObject.UpdateScripts();
        }
    }
}
