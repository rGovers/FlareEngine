using FlareEngine.Rendering.Lighting;

namespace FlareEngine.Definitions
{
    public class DirectionalLightDef : LightDef
    {
        public DirectionalLightDef()
        {
            ComponentType = typeof(DirectionalLight);
        }
    }
}