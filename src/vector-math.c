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

m3  m3_mul(m3 a, m3 b);
m3  m3_rotation_x(f32 angle);
m3  m3_rotation_y(f32 angle);
m3  m3_rotation_z(f32 angle);

m3  m3_from_euler_xyz(f32 pitch, f32 yaw, f32 roll);

m4  m4_identity(void);

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
