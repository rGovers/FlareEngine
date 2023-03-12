using System;
using System.Runtime.InteropServices;

namespace FlareEngine.Maths
{
    [StructLayout(LayoutKind.Explicit, Pack = 0)]
    public struct Vector2
    {
        [FieldOffset(0)]
        public float X;
        [FieldOffset(4)]
        public float Y;

#region CONSTANTS
        public static readonly Vector2 Zero = new Vector2(0.0f);
        public static readonly Vector2 One = new Vector2(1.0f);

        public static readonly Vector2 Infinity = new Vector2(float.PositiveInfinity);

        public static readonly Vector2 Right = new Vector2(1.0f, 0.0f);
        public static readonly Vector2 Left = new Vector2(-1.0f, 0.0f);
        public static readonly Vector2 Up = new Vector2(0.0f, 1.0f);
        public static readonly Vector2 Down = new Vector2(0.0f, -1.0f);
#endregion

        public Vector2(float a_val)
        {
            X = a_val;
            Y = a_val;   
        }
        public Vector2(float a_x, float a_y)
        {
            X = a_x;
            Y = a_y;
        }
        public Vector2(Vector2 a_other)
        {
            X = a_other.X;
            Y = a_other.Y;
        }

        public static Vector2 operator +(Vector2 a_lhs, Vector2 a_rhs)
        {
            return new Vector2(a_lhs.X + a_rhs.X, a_lhs.Y + a_rhs.Y);
        }
        public static Vector2 operator -(Vector2 a_lhs, Vector2 a_rhs)
        {
            return new Vector2(a_lhs.X - a_rhs.X, a_lhs.Y - a_rhs.Y);
        }
        public static Vector2 operator *(Vector2 a_lhs, float a_rhs)
        {
            return new Vector2(a_lhs.X * a_rhs, a_lhs.Y * a_rhs);
        }
        public static Vector2 operator /(Vector2 a_lhs, float a_rhs)
        {
            return new Vector2(a_lhs.X / a_rhs, a_lhs.Y / a_rhs);
        }
        public static Vector2 operator *(Vector2 a_lhs, Vector2 a_rhs)
        {
            return new Vector2(a_lhs.X * a_rhs.X, a_lhs.Y * a_rhs.Y);
        }
        public static Vector2 operator /(Vector2 a_lhs, Vector2 a_rhs)
        {
            return new Vector2(a_lhs.X / a_lhs.X, a_lhs.Y / a_rhs.Y);
        }

        public static bool operator ==(Vector2 a_lhs, Vector2 a_rhs)
        {
            return a_lhs.X == a_rhs.X && a_lhs.Y == a_rhs.Y;
        }
        public static bool operator !=(Vector2 a_lhs, Vector2 a_rhs)
        {
            return a_lhs.X != a_rhs.X || a_lhs.Y != a_rhs.Y;
        }

        public override bool Equals(object a_obj)
        {
            if (a_obj == null || !this.GetType().Equals(a_obj.GetType()))
            {
                return false;
            }
            
            Vector2 vec = (Vector2)a_obj;

            return X == vec.X && Y == vec.Y;
        }

        public override int GetHashCode()
        {
            unchecked
            {
                // Lazy so just prime hash till it bites me
                int hash = 73;
                hash = hash * 79 + X.GetHashCode();
                hash = hash * 79 + Y.GetHashCode();
                return hash;
            }
        }
        public override string ToString()
        {
            return $"({X}, {Y})";
        }

        public float MagnitudeSqr
        {
            get
            {
                return X * X + Y * Y;
            }
        }
        public float Magnitude
        {
            get
            {
                return (float)Math.Sqrt(X * X + Y * Y);
            }
        }

        public void Normalize()
        {
            float mag = Magnitude;

            X /= mag;
            Y /= mag;
        }

        public static Vector2 Normalized(Vector2 a_vector)
        {
            float mag = a_vector.Magnitude;

            return new Vector2(a_vector.X / mag, a_vector.Y / mag);
        }

        public static float Dot(Vector2 a_lhs, Vector2 a_rhs)
        {
            return a_lhs.X * a_rhs.X + a_lhs.Y * a_rhs.Y;
        }
    }}