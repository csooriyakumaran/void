#ifndef VOID_MATH_H_
#define VOID_MATH_H_

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include "base.h"

typedef union v2
{
    struct { f32 x, y;};
    struct { f32 r, g;};
    f32 data[2];
} v2;

typedef union v3
{
    struct { f32 x, y, z;};
    struct { f32 r, g, b;};
    f32 data[3];
} v3;

typedef union v4
{
    struct { f32 x, y, z, w;};
    struct { f32 r, g, b, a;};
    f32 data[4];
} v4;

typedef struct m3
{
    f32 data[9];
} m3;

typedef struct m4
{
    f32 data[16];
} m4;

#define M3(m, r, c) ((m).data[(c)*3 + (r)])
#define M4(m, r, c) ((m).data[(c)*4 + (r)])

f32 deg_to_rad(f32 d);

v3  v3_add(v3 a, v3 b);
v3  v3_sub(v3 a, v3 b);
v3  v3_scale(v3 v, f32 s);
f32 v3_dot(v3 a, v3 b);
v3  v3_cross(v3 a, v3 b);
v3  v3_norm(v3 v);

m3  m3_identity(void);
m3  m3_mul(m3 a, m3 b);
m3  m3_rotation_x(f32 angle);
m3  m3_rotation_y(f32 angle);
m3  m3_rotation_z(f32 angle);

m3  m3_from_euler_xyz(f32 pitch, f32 yaw, f32 roll);

m4  m4_identity(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // VOID_MATH_H_
