using System;
using FlareEngine.Definitions;
using FlareEngine.Maths;
using FlareEngine.Mod;
using FlareEngine.Rendering;

namespace FlareEngine
{
    class Program
    {
        static Camera Cam;

        static Object GameObj;
        static Model Model;

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

            AssetLibrary.Init(workingDir);
            DefLibrary.Init();

            ModControl.Init(workingDir);

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
            ModControl.Close();

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
            
            Profiler.StartFrame("Testing Code");
            GameObj.Transform.Rotation = Quaternion.FromAxisAngle(Vector3.Up, Time.TimePassed);
            GameObj.Transform.Translation = new Vector3(0.0f, (float)Math.Sin(Time.DTimePassed * 0.5), 0.0f);
            Profiler.StopFrame();

            ModControl.Update();

            Object.UpdateObjects();
        }
    }
}
