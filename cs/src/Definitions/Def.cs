namespace FlareEngine.Definitions
{
    public class Def
    {
        public string DefName = string.Empty;
        public string DefPath = string.Empty;

        public virtual void PostResolve() { }
    };
}