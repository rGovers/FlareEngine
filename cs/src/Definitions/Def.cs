namespace FlareEngine.Definitions
{
    public class Def
    {
        [HideInEditor]
        public string DefName = string.Empty;
        [HideInEditor]
        public string DefPath = string.Empty;

        public virtual void PostResolve() { }
    };
}