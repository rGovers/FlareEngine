using FlareEngine.Definitions;
using System;

namespace FlareEngine
{
    class Program
    {
        static void Main(string[] a_args)
        {
            Console.WriteLine("FlareCS: Started");

            AssetLibrary.Init();
            DefLibrary.Init();

            DefLibrary.LoadDefs("Defs");

            Console.WriteLine("FlareCS: Initialized");
        }

        static void Shutdown()
        {
            DefLibrary.Clear();
            AssetLibrary.ClearAssets();

            Console.WriteLine("FlareCS: Shutdown");
        }

        static void Update(double a_delta, double a_time)
        {
            
        }
    }
}
