using FlareEngine.Definitions;
using FlareEngine.Rendering;
using System;

namespace FlareEngine
{
    class Program
    {
        static Material Mat;
        static Camera Cam;

        static void Main(string[] a_args)
        {
            Console.WriteLine("FlareCS: Started");

            AssetLibrary.Init();
            DefLibrary.Init();

            DefLibrary.LoadDefs("Defs");

            MaterialDef def = DefLibrary.GetDef<MaterialDef>("Test");

            Mat = Material.FromDef(def);

            Cam = new Camera();

            Console.WriteLine("FlareCS: Initialized");
        }

        static void Shutdown()
        {
            Mat.Dispose();

            Cam.Dispose();

            DefLibrary.Clear();
            AssetLibrary.ClearAssets();

            Console.WriteLine("FlareCS: Shutdown");
        }

        static void Update(double a_delta, double a_time)
        {
            
        }
    }
}
