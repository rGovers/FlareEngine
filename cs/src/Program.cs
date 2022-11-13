using FlareEngine.Definitions;
using FlareEngine.Mod;

namespace FlareEngine
{
    class Program
    {
        static void Main(string[] a_args)
        {
            Logger.Message("FlareCS: Started");

            Time.Init();

            AssetLibrary.Init();
            DefLibrary.Init();

            // ModControl.Init();

            Logger.Message("FlareCS: Initialized");
        }

        static void Shutdown()
        {
            // ModControl.Close();

            // DefLibrary.Clear();
            // AssetLibrary.ClearAssets();

            Logger.Message("FlareCS: Shutdown");
        }

        static void Update(double a_delta, double a_time)
        {
            Time.DDeltaTime = a_delta;
            Time.DTimePassed = a_time;

            // ModControl.Update();

            Object.UpdateObjects();
        }
    }
}
