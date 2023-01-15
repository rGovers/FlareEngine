using FlareEngine.Maths;
using FlareEngine.Rendering;
using System;

namespace FlareEngine.Definitions
{
    public class CameraDef : GameObjectDef
    {
        public Viewport Viewport = new Viewport()
        {
            Position = Vector2.Zero,
            Size = Vector2.One,
            MinDepth = 0.0f,
            MaxDepth = 1.0f
        };
        public float FOV = (float)(Math.PI * 0.45f);
        public float Near = 0.1f;
        public float Far = 100.0f;
        public uint RenderLayer = 0b1;

        public CameraDef()
        {
            ObjectType = typeof(Camera);
        }
    }
}