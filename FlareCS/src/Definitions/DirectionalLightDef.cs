using FlareEngine.Rendering.Lighting;

namespace FlareEngine.Definitions
{
    public class DirectionalLightDef : LightDef
    {
        public DirectionalLightDef()
        {
            ComponentType = typeof(DirectionalLight);
        }

        public override void PostResolve()
        {
            base.PostResolve();

            if (ComponentType != typeof(DirectionalLight) && !ComponentType.IsSubclassOf(typeof(DirectionalLight)))
            {
                Logger.FlareError($"Directional Light Def Invalid ComponentType: {ComponentType}");
            }
        }
    }
}