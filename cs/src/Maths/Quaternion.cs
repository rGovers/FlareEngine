using System;
using System.Runtime.InteropServices;

namespace FlareEngine.Maths
{
    [StructLayout(LayoutKind.Explicit, Pack = 0)]
    public struct Quaternion
    {
        [FieldOffset(0)]
        public float X;
        [FieldOffset(4)]
        public float Y;
        [FieldOffset(8)]
        public float Z;
        [FieldOffset(12)]
        public float W;

        public static readonly Quaternion Identity = new Quaternion(0.0f, 0.0f, 0.0f, 1.0f); 

        public Quaternion(Quaternion a_other)
        {
            X = a_other.X;
            Y = a_other.Y;
            Z = a_other.Z;
            W = a_other.W;
        }
        public Quaternion(float a_x, float a_y, float a_z, float a_w)
        {
            X = a_x;
            Y = a_y;
            Z = a_z;
            W = a_w;
        }

        public static Quaternion FromAxisAngle(Vector3 a_axis, float a_angle)
        {
            float halfAngle = a_angle * 0.5f;

            float sin = (float)Math.Sin(halfAngle);

            return new Quaternion(a_axis.X * sin, a_axis.Y * sin, a_axis.Z * sin, (float)Math.Cos(halfAngle));
        }
        public static Quaternion FromEuler(Vector3 a_euler)
        {
            Vector3 halfVec = a_euler * 0.5f;
            double c1 = Math.Cos(halfVec.Y);
            double s1 = Math.Sin(halfVec.Y);
            double c2 = Math.Cos(halfVec.X);
            double s2 = Math.Sin(halfVec.X);
            double c3 = Math.Cos(halfVec.Z);
            double s3 = Math.Sin(halfVec.Z);

            double w = Math.Sqrt(1.0 + c1 * c2 + c1 * c3 - s1 * s2 * s3 + c2 * c3) * 0.5;
            double w4 = 4 * w;

            return new Quaternion
            (
                (float)((c2 * s3 + c1 * s3 + s1 * s2 * c3) / w4), 
                (float)((s1 * c2 + s1 * c3 + c1 * s2 * s3) / w4),
                (float)((-s1 * s3 + c1 * s2 * c3 + s2) / w4),
                (float)w
            );
        }

        public float MagnitudeSqr
        {
            get
            {
                return X * X + Y * Y + Z * Z + W * W;
            }
        }
        public float Magnitude
        {
            get
            {
                return (float)Math.Sqrt(MagnitudeSqr);
            }
        }

        public static Quaternion operator *(Quaternion a_lhs, Quaternion a_rhs)
        {
            return new Quaternion
            (
                a_lhs.W * a_rhs.X + a_lhs.X * a_rhs.W + a_lhs.Y * a_rhs.Z - a_lhs.Z * a_rhs.Y, 
                a_lhs.W * a_rhs.Y + a_lhs.Y * a_rhs.W + a_lhs.Z * a_rhs.X - a_lhs.X * a_rhs.Z, 
                a_lhs.W * a_rhs.Z + a_lhs.Z * a_rhs.W + a_lhs.X * a_rhs.Y - a_lhs.Y * a_rhs.X, 
                a_lhs.W * a_rhs.W - a_lhs.X * a_rhs.X - a_lhs.Y * a_rhs.Y - a_lhs.Z * a_rhs.Z
            );
        }
        public static Vector3 operator *(Quaternion a_lhs, Vector3 a_rhs)
        {
            Vector3 qVec = new Vector3(a_lhs.X, a_lhs.Y, a_lhs.Z);
            Vector3 c = Vector3.Cross(qVec, a_rhs);
            Vector3 cc = Vector3.Cross(qVec, c);

            return a_rhs + ((c * a_lhs.W) + cc) * 2.0f;
        }
        public static Vector4 operator *(Quaternion a_lhs, Vector4 a_rhs)
        {
            return new Vector4(a_lhs * a_rhs.XYZ, a_rhs.W);
        }

        public void Normalize()
        {
            float mag = Magnitude;

            X /= mag;
            Y /= mag;
            Z /= mag;
            W /= mag;
        }
        public static Quaternion Normalized(Quaternion a_quat)
        {
            float mag = a_quat.Magnitude;

            return new Quaternion(a_quat.X / mag, a_quat.Y / mag, a_quat.Z / mag, a_quat.W / mag);
        }
    }
}