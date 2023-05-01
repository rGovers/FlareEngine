using FlareEngine.Rendering.Lighting;

namespace FlareEngine.Definitions
{
    public class PointLightDef : LightDef
    {
        public float Radius = 1.0f;

        public PointLightDef()
        {
            ComponentType = typeof(PointLight);
        }

        public override void PostResolve()
        {
            base.PostResolve();

            if (ComponentType != typeof(PointLight) && !ComponentType.IsSubclassOf(typeof(PointLight)))
            {
                Logger.FlareError($"Point Light Def Invalid ComponentType: {ComponentType}");

                return;
            }
        }
    }
}