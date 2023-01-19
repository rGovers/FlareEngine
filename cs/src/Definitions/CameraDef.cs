using FlareEngine.Maths;
using FlareEngine.Rendering;
using System;

namespace FlareEngine.Definitions
{
    public class CameraDef : GameObjectDef
    {
        [EditorTooltip("Viewport to determine the portion of the screen rendered to.")]
        public Viewport Viewport = new Viewport()
        {
            Position = Vector2.Zero,
            Size = Vector2.One,
            MinDepth = 0.0f,
            MaxDepth = 1.0f
        };
        [EditorTooltip("Field of View for the camera.")]
        public float FOV = (float)(Math.PI * 0.45f);
        [EditorTooltip("Near clipping plane for the camera.")]
        public float Near = 0.1f;
        [EditorTooltip("Far clipping plane for the camera.")]
        public float Far = 100.0f;
        [EditorTooltip("Renders objects of the same render layer. Binary bit based.")]
        public uint RenderLayer = 0b1;

        public CameraDef()
        {
            ObjectType = typeof(Camera);
        }
    }
}