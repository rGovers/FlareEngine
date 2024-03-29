using System;

namespace FlareEngine.Maths
{
    public static class Mathf
    {
        public const float PI = (float)Math.PI;
        public const float HalfPI = PI * 0.5f;
        public const float TwoPI = PI * 2.0f;

        public const float DegToRad = PI / 180.0f;
        public const float RadToDef = 180.0f / PI;
        
        public static float Asin(float a_v)
        {
            return (float)Math.Asin(a_v);
        }
        public static float Acos(float a_v)
        {
            return (float)Math.Acos(a_v);
        }
        public static float Atan2(float a_x, float a_y)
        {
            return (float)Math.Atan2(a_x, a_y);
        }
        public static float Sin(float a_a)
        {
            return (float)Math.Sin(a_a);
        }
        public static float Cos(float a_a)
        {
            return (float)Math.Cos(a_a);
        }
        public static float Tan(float a_a)
        {
            return (float)Math.Tan(a_a);
        }

        public static float Sqrt(float a_a)
        {
            return (float)Math.Sqrt(a_a);
        }
    }
}