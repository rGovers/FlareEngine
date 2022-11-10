using FlareEngine.Definitions;
using FlareEngine.Maths;
using FlareEngine.Rendering;
using System;

namespace FlareEngine
{
    class Program
    {
        static Camera Cam;

        static Object Obj;

        static Model Model;

        static void Main(string[] a_args)
        {
            Logger.Message("FlareCS: Started");

            Time.Init();

            AssetLibrary.Init();
            DefLibrary.Init();

            DefLibrary.LoadDefs("Defs");
            DefLibrary.ResolveDefs();

            MaterialDef matDef = DefLibrary.GetDef<MaterialDef>("TestMat");
            ObjectDef objDef = DefLibrary.GetDef<ObjectDef>("TestObj");

            Model = PrimitiveGenerator.CreatePrimitive(PrimitiveType.Cube);

            Cam = Object.Instantiate<Camera>();
            Cam.Transform.Translation = new Vector3(1, -2, 10);
            Cam.Transform.Rotation = Quaternion.FromAxisAngle(Vector3.Right, (float)Math.PI * 0.1f);

            Obj = Object.FromDef(objDef);
            MeshRenderer meshRenderer = Obj.AddComponent<MeshRenderer>();
            meshRenderer.Material = AssetLibrary.GetMaterial(matDef);
            meshRenderer.Model = Model;

            Logger.Message("FlareCS: Initialized");
        }

        static void Shutdown()
        {
            Cam.Dispose();
            Obj.Dispose();

            Model.Dispose();

            DefLibrary.Clear();
            AssetLibrary.ClearAssets();

            Logger.Message("FlareCS: Shutdown");
        }

        static void Update(double a_delta, double a_time)
        {
            Time.DDeltaTime = a_delta;
            Time.DTimePassed = a_time;

            Object.UpdateObjects();
        }
    }
}
