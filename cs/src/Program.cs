using FlareEngine.Definitions;
using FlareEngine.Mod;
using FlareEngine.Rendering;

namespace FlareEngine
{
    class Program
    {
        static Camera Cam;

        static Object GameObj;
        static Model Model;

        static void Main(string[] a_args)
        {
            Logger.Message("FlareCS: Started");

            Time.Init();

            AssetLibrary.Init();
            DefLibrary.Init();

            // ModControl.Init();

            DefLibrary.LoadDefs("Defs");

            DefLibrary.ResolveDefs();

            MaterialDef def = DefLibrary.GetDef<MaterialDef>("TestMat");

            Model = PrimitiveGenerator.CreateCube();
            
            Cam = Object.Instantiate<Camera>();
            Cam.Transform.Translation = new Maths.Vector3(0.0f, -1.0f, 10.0f);

            GameObj = Object.Instantiate<Object>();
            MeshRenderer renderer = GameObj.AddComponent<MeshRenderer>();
            renderer.Material = AssetLibrary.GetMaterial(def);
            renderer.Model = Model;

            Logger.Message("FlareCS: Initialized");
        }

        static void Shutdown()
        {
            // ModControl.Close();

            Cam.Dispose();
            GameObj.Dispose();
            Model.Dispose();

            DefLibrary.Clear();
            AssetLibrary.ClearAssets();

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
