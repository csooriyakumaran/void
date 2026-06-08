#ifndef VOID_RENDERER_H_
#define VOID_RENDERER_H_

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include "base.h"
#include "vector-math.h"

typedef u64 RenderMode;
enum RenderMode_
{
    RenderMode_None      = BIT(0),
    RenderMode_Wireframe = BIT(1),
    RenderMode_Shaded    = BIT(2)
};

typedef struct Camera
{
    f32 yaw;
    f32 pitch;
    f32 distance;
} Camera;

typedef struct Renderer
{
    i32 width;
    i32 height;

    Camera camera;
    RenderMode mode;

} Renderer;

typedef struct Line
{
    v3 a;
    v3 b;
    v3 color;
} Line;

typedef struct MeshVertex
{
    v3 position;
    v3 normal;
} MeshVertex;

typedef struct MeshIndex
{
    u32 i0, i1, i2;
} MeshIndex;

typedef struct Mesh
{
    MeshVertex* vertices;
    u32         vertex_count;

    MeshIndex*  indices;
    u32         index_count;
} Mesh;

typedef struct MeshInstance
{
    Mesh*      mesh;
    m4         transform; // world transform
    v3         color;
    RenderMode mode;
} MeshInstance;

typedef struct RenderFrame
{
    Line* lines;
    u32   line_count;
    u64   line_cap;

    MeshInstance* meshes;
    u32           mesh_count;
    u64           mesh_cap;

    // RenderText* text;
    // u32         text_count;
} RenderFrame;

void render_init(Renderer* r);
void render_resize(Renderer* r, i32 w, i32 h);
void render_begin_frame(Renderer* r);
void render_end_frame(Renderer* r);

void render_camera_orbit(Renderer* r, f32 dx, f32 dy, f32 v);
void render_camera_zoom(Renderer* r, f32 amount, f32 v);

// - Render Commands
void render_frame_push_lines(RenderFrame* f, v3 a, v3 b, v3 color);
void render_frame_push_mesh(RenderFrame* f, Mesh* mesh, m4 transform, v3 color, RenderMode mode);

void render_draw_frame(Renderer* r, RenderFrame* f);

void render_draw_test_traingle(Renderer* r);

// - transforms
m4 make_transform(v3 t, m3 r, v3 s);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // VOID_RENDERER_H_

