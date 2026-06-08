#include <math.h>
#include "vector-math.h"

f32 deg_to_rad(f32 d) { return d * (3.1415926535f / 180.0f); }

v3  v3_add(v3 a, v3 b)
{
    return (v3) {
        .x = a.x + b.x,
        .y = a.y + b.y, 
        .z = a.z + b.z
    };
}

v3  v3_sub(v3 a, v3 b)
{
    return (v3) {
        .x = a.x - b.x,
        .y = a.y - b.y, 
        .z = a.z - b.z
    };
}

v3  v3_scale(v3 v, f32 s)
{
    return (v3) {
        .x = v.x * s,
        .y = v.y * s,
        .z = v.z * s
    };
}

f32 v3_dot(v3 a, v3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

v3  v3_cross(v3 a, v3 b)
{
    return (v3) {
        .x = a.y * b.z - a.z * b.y,
        .y = a.z * b.x - a.x * b.z,
        .z = a.x * b.y - a.y * b.x
    };
}

v3  v3_norm(v3 v)
{
    f32 mag = sqrt(v.x*v.x + v.y*v.y + v.z*v.z);

    if ( mag > 0.0f )
        return (v3) {
            .x = v.x / mag, 
            .y = v.y / mag,
            .z = v.z / mag
        };
    else
        return (v3){
            .x = 0.0f, 
            .y = 0.0f,
            .z = 0.0f
        };
}

m3 m3_identity(void)
{
    return (m3) {
        .data = {
            1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 1.0f
        }
    };
}

m3  m3_mul(m3 a, m3 b)
{
    m3 R;
    for (int r = 0; r < 3; ++r)
    {
        for (int c = 0; c < 3; ++c)
        {
            f32 sum = 0.0f;
            for (int k = 0; k < 3; ++k)
            {
                sum += M3(a, r, k) * M3(b, k, c);
            }
            M3(R, r, c) = sum;
        }
    }

    return R;
}

m3  m3_rotation_x(f32 a)
{
    m3  R = m3_identity();
    f32 c = cosf(a);
    f32 s = sinf(a);

    M3(R, 1, 1) =  c;
    M3(R, 1, 2) = -s;
    M3(R, 2, 1) =  s;
    M3(R, 2, 2) =  c;

    return R;
}

m3  m3_rotation_y(f32 a)
{
    m3  R = m3_identity();
    f32 c = cosf(a);
    f32 s = sinf(a);

    M3(R, 0, 0) =  c;
    M3(R, 0, 2) =  s;
    M3(R, 2, 0) = -s;
    M3(R, 2, 2) =  c;

    return R;
}
m3  m3_rotation_z(f32 a)
{
    m3  R = m3_identity();
    f32 c = cosf(a);
    f32 s = sinf(a);

    M3(R, 0, 0) =  c;
    M3(R, 0, 1) = -s;
    M3(R, 1, 0) =  s;
    M3(R, 1, 1) =  c;

    return R;
}

m3  m3_from_euler_xyz(f32 pitch, f32 yaw, f32 roll)
{
    m3 Rx = m3_rotation_x(pitch);
    m3 Ry = m3_rotation_y(yaw);
    m3 Rz = m3_rotation_z(roll);

    m3 R  = m3_mul(Rz, m3_mul(Rx, Ry));
    return R;
}

m4 m4_identity(void)
{
    return (m4) {
        .data = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        }
    };
}
