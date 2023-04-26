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
    }
}