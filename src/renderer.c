#include "renderer.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>

void render_init(Renderer* r)
{

    r->width  = 1;
    r->height = 1;

    r->camera.yaw      = 35.0f;
    r->camera.pitch    = 25.0f;
    r->camera.distance = 12.0f;

    glEnable(GL_DEPTH_TEST);
}


void render_resize(Renderer* r, i32 width, i32 height)
{

    r->width  = width  > 0 ? width  : 1;
    r->height = height > 0 ? height : 1;

    glViewport(0, 0, r->width, r->height);

}


void render_begin_frame(Renderer* r)
{
    glViewport(0, 0, r->width, r->height);

    glClearColor(0.125f, 0.125f, 0.125f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    f32 aspect = (f32)r->width / (f32)r->height;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // crude perspective frustum
    glFrustum(-aspect, aspect, -1.0, 1.0, 1.0, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // crude camera
    glTranslatef(0.0f, 0.0f, -r->camera.distance);
    glRotatef(r->camera.pitch, 1.0f, 0.0f, 0.0f);
    glRotatef(r->camera.yaw,   0.0f, 1.0f, 0.0f);
}


void render_end_frame(Renderer* r)
{
    (void)r;

}

void render_draw_frame(Renderer* r, RenderFrame* f)
{
    (void)r;

    // Meshes
    for (u32 i = 0; i < f->mesh_count; ++i)
    {
        MeshInstance* inst = &f->meshes[i];
        Mesh* mesh = inst->mesh;

        glColor3f(inst->color.r, inst->color.g, inst->color.b);

        glPushMatrix();
        glMultMatrixf(inst->transform.data);

        if ((inst->mode & RenderMode_Shaded) != 0)
        {
            glBegin(GL_TRIANGLES);
            for (u32 t = 0; t < mesh->index_count; ++t)
            {
                MeshIndex idx = mesh->indices[t];

                MeshVertex* v0 = &mesh->vertices[idx.i0];
                MeshVertex* v1 = &mesh->vertices[idx.i1];
                MeshVertex* v2 = &mesh->vertices[idx.i2];

                glVertex3f(v0->position.x, v0->position.y, v0->position.z);
                glVertex3f(v1->position.x, v1->position.y, v1->position.z);
                glVertex3f(v2->position.x, v2->position.y, v2->position.z);
            }
            glEnd();
        }
        if ((inst->mode & RenderMode_Wireframe) != 0)
        {
            glBegin(GL_LINES);
            for (u32 t = 0; t < mesh->index_count; ++t)
            {
                MeshIndex idx = mesh->indices[t];

                MeshVertex* v0 = &mesh->vertices[idx.i0];
                MeshVertex* v1 = &mesh->vertices[idx.i1];
                MeshVertex* v2 = &mesh->vertices[idx.i2];

                glVertex3f(v0->position.x, v0->position.y, v0->position.z);
                glVertex3f(v1->position.x, v1->position.y, v1->position.z);
                glVertex3f(v1->position.x, v1->position.y, v1->position.z);
                glVertex3f(v2->position.x, v2->position.y, v2->position.z);
                glVertex3f(v2->position.x, v2->position.y, v2->position.z);
                glVertex3f(v0->position.x, v0->position.y, v0->position.z);
            }
            glEnd();
        }
        glPopMatrix();
    }

    // lines
    glBegin(GL_LINES);
    for (u32 i = 0; i < f->line_count; ++i)
    {
        Line* l = f->lines + i;

        glColor3f(l->color.r, l->color.g, l->color.b);
        glVertex3f(l->a.x, l->a.y, l->a.z);
        glVertex3f(l->b.x, l->b.y, l->b.z);
    }
    glEnd();
}

void render_camera_orbit(Renderer* r, f32 dx, f32 dy, f32 v)
{
    r->camera.yaw   += dx * v;
    r->camera.pitch += dy * v;

    if ( r->camera.pitch > +89.0f ) r->camera.pitch = +89.0f;
    if ( r->camera.pitch < -89.0f ) r->camera.pitch = -89.0f;
}


void render_camera_zoom(Renderer* r, f32 amount, f32 v)
{
    r->camera.distance *= 1.0f - amount * v;

    if ( r->camera.distance < 1.0f )   r->camera.distance = 1.0f;
    if ( r->camera.distance > 200.0f ) r->camera.distance = 200.0f;
}

void render_frame_push_lines(RenderFrame* f, v3 a, v3 b, v3 color)
{
    if (f->line_count < f->line_cap)
    {
        Line* l = f->lines + f->line_count++;
        l->a     = a;
        l->b     = b;
        l->color = color;
    }
}

void render_frame_push_mesh(RenderFrame* f, Mesh* mesh, m4 transform, v3 color, RenderMode mode)
{
    if (f->mesh_count < f->mesh_cap)
    {
        MeshInstance* inst = f->meshes + f->mesh_count++;
        inst->mesh = mesh;
        inst->transform = transform;
        inst->color = color;
        inst->mode = mode;
    }

}

m4 make_transform(v3 t, m3 r, v3 s)
{
    m4 M = {0};

    M4(M, 0, 0) = s.x * M3(r, 0, 0);
    M4(M, 0, 1) = s.y * M3(r, 0, 1);
    M4(M, 0, 2) = s.z * M3(r, 0, 2);
    M4(M, 0, 3) = t.x;

    M4(M, 1, 0) = s.x * M3(r, 1, 0);
    M4(M, 1, 1) = s.y * M3(r, 1, 1);
    M4(M, 1, 2) = s.z * M3(r, 1, 2);
    M4(M, 1, 3) = t.y;

    M4(M, 2, 0) = s.x * M3(r, 2, 0);
    M4(M, 2, 1) = s.y * M3(r, 2, 1);
    M4(M, 2, 2) = s.z * M3(r, 2, 2);
    M4(M, 2, 3) = t.z;

    M4(M, 3, 0) = 0.0f;
    M4(M, 3, 1) = 0.0f;
    M4(M, 3, 2) = 0.0f;
    M4(M, 3, 3) = 1.0f;

    return M;
}

