using FlareEngine.Rendering.Lighting;

namespace FlareEngine.Definitions
{
    public class SpotLightDef : LightDef
    {
        public float InnerCutoffAngle = 1.0f;
        public float OuterCutoffAngle = 1.5f;
        public float Radius = 10.0f;

        public SpotLightDef()
        {
            ComponentType = typeof(SpotLight);
        }
    }
}