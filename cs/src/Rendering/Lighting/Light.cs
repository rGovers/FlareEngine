using FlareEngine.Definitions;

namespace FlareEngine.Rendering.Lighting
{
    public enum LightType : ushort
    {
        Directional = 0,
        Point,
        Spot
    }

    public abstract class Light : Component
    {
        public abstract LightType LightType
        {
            get;
        }

        public LightDef LightDef
        {
            get
            {
                return Def as LightDef;
            }
        }
    }
}