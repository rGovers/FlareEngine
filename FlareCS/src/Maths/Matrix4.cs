namespace FlareEngine.Maths
{
    public struct Matrix4
    {
        public static readonly Matrix4 Identity = new Matrix4(1.0f);

        float[] Data;

        public Vector4 this[int a_key]
        {
            get
            {
                int offset = a_key * 4;

                return new Vector4(Data[offset + 0], Data[offset + 1], Data[offset + 2], Data[offset + 3]);
            }
            set
            {
                int offset = a_key * 4;

                Data[offset + 0] = value.X;
                Data[offset + 1] = value.Y;
                Data[offset + 2] = value.Z;
                Data[offset + 3] = value.W;
            }
        }

        public Matrix4(float a_val) : this(a_val, 0.0f, 0.0f, 0.0f,
                                           0.0f, a_val, 0.0f, 0.0f,
                                           0.0f, 0.0f, a_val, 0.0f,
                                           0.0f, 0.0f, 0.0f, a_val)
        {
        }
        public Matrix4(Vector4 a_blk1, Vector4 a_blk2, Vector4 a_blk3, Vector4 a_blk4) : this(a_blk1.X, a_blk1.Y, a_blk1.Z, a_blk1.W,
                                                                                              a_blk2.X, a_blk2.Y, a_blk2.Z, a_blk2.W,
                                                                                              a_blk3.X, a_blk3.Y, a_blk3.Z, a_blk3.W,
                                                                                              a_blk4.X, a_blk4.Y, a_blk4.Z, a_blk4.W)
        {

        }
        public Matrix4(float a_0_0, float a_0_1, float a_0_2, float a_0_3,
                       float a_1_0, float a_1_1, float a_1_2, float a_1_3,
                       float a_2_0, float a_2_1, float a_2_2, float a_2_3,
                       float a_3_0, float a_3_1, float a_3_2, float a_3_3)
        {
            Data = new float[16];

            Data[0] = a_0_0;
            Data[1] = a_0_1;
            Data[2] = a_0_2;
            Data[3] = a_0_3;

            Data[4] = a_1_0;
            Data[5] = a_1_1;
            Data[6] = a_1_2;
            Data[7] = a_1_3;

            Data[8] = a_2_0;
            Data[9] = a_2_1;
            Data[10] = a_2_2;
            Data[11] = a_2_3;

            Data[12] = a_3_0;
            Data[13] = a_3_1;
            Data[14] = a_3_2;
            Data[15] = a_3_3;
        }
        public Matrix4(Matrix4 a_other)
        {
            Data = new float[16];

            for (int i = 0; i < 16; ++i)
            {
                Data[i] = a_other.Data[i];
            }
        }

        public static Matrix4 FromTransform(Vector3 a_translation, Quaternion a_rotation, Vector3 a_scale)
        {
            Matrix4 translation = new Matrix4
            (
                1.0f, 0.0f, 0.0f, a_translation.X, 
                0.0f, 1.0f, 0.0f, a_translation.Y,
                0.0f, 0.0f, 1.0f, a_translation.Z,
                0.0f, 0.0f, 0.0f, 1.0f
            );
            Matrix4 scale = new Matrix4
            (
                a_scale.X, 0.0f, 0.0f, 0.0f,
                0.0f, a_scale.Y, 0.0f, 0.0f,
                0.0f, 0.0f, a_scale.Z, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            );

            return translation * a_rotation.ToMatrix() * scale;
        }

        public static Matrix4 Transpose(Matrix4 a_matrix)
        {
            Matrix4 mat = new Matrix4(0.0f);

            for (int i = 0; i < 4; ++i)
            {
                for (int j = 0; j < 4; ++j)
                {
                    mat.Data[i * 4 + j] = a_matrix.Data[i + j * 4];
                }
            }

            return mat;
        }

        public static Matrix4 operator *(Matrix4 a_lhs, Matrix4 a_rhs)
        {
            // Not the fastest matrix multiplication but should work for now
            Matrix4 mat = new Matrix4(0.0f);

            for (int i = 0; i < 4; ++i)
            {
                for (int j = 0; j < 4; ++j)
                {
                    for (int k = 0; k < 4; ++k)
                    {
                        mat.Data[i * 4 + j] += a_lhs.Data[i * 4 + k] * a_rhs.Data[k * 4 + j]; 
                    }
                }
            }

            return mat;
        }
        public static Matrix4 operator +(Matrix4 a_lhs, Matrix4 a_rhs)
        {
            Matrix4 mat = new Matrix4(0.0f);

            for (int i = 0; i < 16; ++i)
            {
                mat.Data[i] = a_lhs.Data[i] + a_rhs.Data[i];
            }

            return mat;
        }
        public static Matrix4 operator -(Matrix4 a_lhs, Matrix4 a_rhs)
        {
            Matrix4 mat = new Matrix4(0.0f);

            for (int i = 0; i < 16; ++i)
            {
                mat.Data[i] = a_lhs.Data[i] + a_rhs.Data[i];
            }

            return mat;
        }
        public static Vector4 operator *(Matrix4 a_lhs, Vector4 a_rhs)
        {
            Vector4 vec = new Vector4(0.0f);

            for (int i = 0; i < 4; ++i)
            {
                for (int j = 0; j < 4; ++j)
                {
                    vec[i] += a_lhs.Data[i * 4 + j] * a_rhs[j];
                }
            }

            return vec;
        }
    }
}