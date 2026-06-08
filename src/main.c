#include <stdio.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <dwmapi.h>

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1
#define DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1 19
#endif // DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif //DWMWA_USE_IMMERSIVE_DARK_MODE

#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
#endif // DWMWA_SYSTEMBACKDROP_TYPE

#include "base.h"
#include "vector-math.h"
#include "renderer.h"

typedef struct AppMemory
{
    Arena permanent;
    Arena frame;
    Arena scratch;
} AppMemory;

typedef struct App
{
    HWND  window;    // native window handle
    HDC   dc;        // device-context for OpenGL
    HGLRC glrc;      // OpenGL rendering context

    Renderer renderer;
    RenderFrame frame;
    AppMemory memory;

    b8    running;

    b8     mouse_left_down;
    i32    mouse_x;
    i32    mouse_y;

    Mesh* test_mesh;

} App;

static App* g_app;

static void arena_test_commit_growth(Arena* arena)
{
    u64 old_commit = arena->commit_size;
   
    u64 size = MB(1);
    u8* block = arena_push_array(arena, u8, size);

    block[0] = 0xAA;
    block[size-1] = 0xBB;

    char buffer[256];
    snprintf(buffer, sizeof(buffer),
             "scratch allocation: pos = %llu | commit = %llu | reserve = %llu%s\n",
             arena->pos,
             arena->commit_size,
             arena->reserved_size,
             arena->commit_size != old_commit ? " | COMMIT GREW" : ""
             );

    OutputDebugStringA(buffer);
    
}

typedef BOOL (WINAPI *wgl_swap_interval_ext_fn)(int interval);

static void win32_set_vsync(b32 enabled)
{
    wgl_swap_interval_ext_fn wglSwapIntervalEXT = (wgl_swap_interval_ext_fn)wglGetProcAddress("wglSwapIntervalEXT");

    if (wglSwapIntervalEXT)
        wglSwapIntervalEXT(enabled ? 1 : 0);
}


// - return 1 on success
static HGLRC win32_create_opengl_context(HDC dc)
{

    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.nSize        = sizeof(pfd);
    pfd.nVersion     = 1;
    pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType   = PFD_TYPE_RGBA;
    pfd.cColorBits   = 32;
    pfd.cDepthBits   = 24;
    pfd.cStencilBits = 8;

    int pixel_format = ChoosePixelFormat(dc, &pfd);

    SetPixelFormat(dc, pixel_format, &pfd);

    HGLRC glrc = wglCreateContext(dc);
    wglMakeCurrent(dc, glrc);

    return glrc;
}

static void app_resize(App* app, int width, int height)
{
    render_resize(&app->renderer, width, height);
}


// Per-frame update hook.
static void app_update(App *app)
{
    arena_clear(&app->memory.frame);


    app->frame.line_cap   = 4096;
    app->frame.line_count = 0;
    app->frame.lines      = arena_push_array(
        &app->memory.frame,
        Line,
        app->frame.line_cap
    );

    app->frame.mesh_cap   = 128;
    app->frame.mesh_count = 0;
    app->frame.meshes     = arena_push_array(
        &app->memory.frame,
        MeshInstance,
        app->frame.mesh_cap
    );

    // temp draw grid
    int half_count = 10;
    f32 spacing    = 1.0f;
    f32 extent     = half_count * spacing;

    for (int i = -half_count; i <= half_count; ++i)
    {
        if (app->frame.line_count > app->frame.line_cap)
            break;

        f32 p = (f32)i * spacing;

        render_frame_push_lines(
            &app->frame,
            (v3){ .x=-extent, .y=0.0f,  .z=p },
            (v3){ .x=+extent, .y=0.0f,  .z=p },
            (v3){ .r=0.28f,   .g=0.28f, .b=0.28f }
        );
        render_frame_push_lines(
            &app->frame,
            (v3){ .x=p, .y=0.0f,  .z=-extent },
            (v3){ .x=p, .y=0.0f,  .z= extent },
            (v3){ .r=0.28f,   .g=0.28f, .b=0.28f }
        );
    }

    render_frame_push_mesh(
        &app->frame,
        app->test_mesh,
        m4_identity(),
        (v3){.r=1.0f, .g=1.0f, .b=1.0f},
        RenderMode_Wireframe
    );

    m4 tmp_transform = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        2.0f, 2.0f, 2.0f, 1.0f,
    };

    render_frame_push_mesh(
        &app->frame,
        app->test_mesh,
        tmp_transform,
        (v3){.r=1.0f, .g=0.0f, .b=1.0f},
        RenderMode_Wireframe
    );

}

// Per-frame render hook. 
static void app_render(App *app)
{
    render_begin_frame(&app->renderer);
    render_draw_frame(&app->renderer, &app->frame);

    render_end_frame(&app->renderer);

    SwapBuffers(app->dc);
}

static void app_draw_frame(App* app)
{
    app_update(app);
    app_render(app);
}

void win32_set_darkmode(HWND hwnd, b32 enabled)
{
    BOOL dark = enabled ? TRUE : FALSE;
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark));
}

void win32_set_backdrop(HWND hwnd, b32 enabled)
{
    if (!enabled) return;

    enum DWM_SYSTEMBACKDROP_TYPE backdrop = DWMSBT_TRANSIENTWINDOW;
    DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdrop, sizeof(backdrop));
}

// Basic window procedure: handle close/destroy and a simple 'q' to quit shortcut.
static LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    App* app = (App*)GetWindowLongPtrA(hwnd, GWLP_USERDATA);
    switch (msg)
    {
        case WM_CLOSE:
        {
            DestroyWindow(hwnd);
            return 0;
        }

        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }

        case WM_SIZE:
        {
            int width  = LOWORD(lparam);
            int height = HIWORD(lparam);
            if (app)
            {
                app_resize(app, width, height);
                app_draw_frame(app);
            }

            return 0;

        }

        case WM_SIZING:
        {
            if (app)
            {
                RECT rect;
                GetClientRect(hwnd, &rect);

                app_resize(app, rect.right - rect.left, rect.bottom - rect.top);
                app_draw_frame(app);

            }
            return TRUE;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            if (app)
            {
                RECT rect;
                GetClientRect(hwnd, &rect);

                app_resize(app, rect.right - rect.left, rect.bottom - rect.top);
                app_draw_frame(app);

            }
            EndPaint(hwnd, &ps);
            return 0;

        }

        case WM_ERASEBKGND:
        {
            return 1;
        }

        case WM_CHAR:
        {
            if (wparam == 'q' || wparam == 'Q')
            {
                DestroyWindow(hwnd);
            }
            if (wparam == 't' || wparam == 'T')
            {
                if (app) arena_test_commit_growth(&app->memory.scratch);
            }
            return 0;
        }

        case WM_LBUTTONDOWN:
        {
            if (app)
            {
                app->mouse_left_down = 1;
                app->mouse_x = GET_X_LPARAM(lparam);
                app->mouse_y = GET_Y_LPARAM(lparam);
                SetCapture(hwnd);
            }
            return 0;
        }

        case WM_LBUTTONUP:
        {
            if (app)
            {
                app->mouse_left_down = 0;
                ReleaseCapture();
            }
            return 0;
        }

        case WM_MOUSEMOVE:
        {
            if (app)
            {
                i32 x = GET_X_LPARAM(lparam);
                i32 y = GET_Y_LPARAM(lparam);

                i32 dx = x - app->mouse_x;
                i32 dy = y - app->mouse_y;

                app->mouse_x = x;
                app->mouse_y = y;

                if (app->mouse_left_down)
                {
                    render_camera_orbit(&app->renderer, (f32)dx, (f32)dy, 0.25f);
                    InvalidateRect(hwnd, 0, FALSE);
                }

            }
            return 0;
        }

        case WM_MOUSEWHEEL:
        {

            if (app)
            {
                i16 wheel_delta = GET_WHEEL_DELTA_WPARAM(wparam);
                render_camera_zoom(&app->renderer, (f32)wheel_delta /120.0f, 0.1f);
                InvalidateRect(hwnd, 0, FALSE);
            }
            return 0;
        }
    }

    return DefWindowProcA(hwnd, msg, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR command_line, int show_code)
{
    (void)prev_instance;
    (void)command_line;
    (void)show_code;

    WNDCLASSA wc      = {0};
    wc.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc    = window_proc;
    wc.hInstance      = instance;
    wc.lpszClassName  = "ViewerWindowClass";

    if (!RegisterClassA(&wc))
    {
        return 1;
    }

    HWND window = CreateWindowExA(
        0,
        wc.lpszClassName,
        "Void Viewer",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1280,
        720,
        0,
        0,
        instance,
        0);

    if (!window)
    {
        return 1;
    }

    win32_set_darkmode(window, 1);
    win32_set_backdrop(window, 1);

    App app;
    app.window           = window;
    app.dc               = GetDC(app.window);
    app.glrc             = win32_create_opengl_context(app.dc);
    app.running          = 1;
    app.mouse_left_down  = 0;
    app.memory.permanent = arena_alloc(GB(1), MB(16));
    app.memory.frame     = arena_alloc(MB(16), MB(1));
    app.memory.scratch   = arena_alloc(GB(1), MB(16));

    win32_set_vsync(1);


    Mesh* triangle = arena_push_t(&app.memory.permanent, Mesh);

    triangle->vertex_count = 3;
    triangle->index_count  = 1;
    triangle->vertices     = arena_push_array(&app.memory.permanent, MeshVertex, 3);
    triangle->indices      = arena_push_array(&app.memory.permanent, MeshIndex, 1);

    triangle->vertices[0].position = (v3){ .x=-0.5f, .y=-0.5f, .z=0.0f };
    triangle->vertices[1].position = (v3){ .x=+0.5f, .y=-0.5f, .z=0.0f };
    triangle->vertices[2].position = (v3){ .x= 0.0f, .y= 0.5f, .z=0.0f };

    for (u32 i = 0; i < 3; ++i)
        triangle->vertices[i].normal = (v3){ .x=0.0f, .y=0.0f, .z=1.0f };

    triangle->indices[0].i0 = 0;
    triangle->indices[0].i1 = 1;
    triangle->indices[0].i2 = 2;

    app.test_mesh = triangle;

    render_init(&app.renderer);

    RECT rect;
    GetClientRect(app.window, &rect);
    app_resize(&app, (i32)(rect.right - rect.left), (i32)(rect.bottom - rect.top));

    SetWindowLongPtrA(app.window, GWLP_USERDATA, (LONG_PTR)&app);

    g_app = &app;

    // Main loop: pump messages, then run update/render for each frame.
    while (app.running)
    {

        MSG msg;

        while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                app.running = 0;
                break;
            }

            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }

        if (!app.running)
        {
            break;
        }

        app_update(&app);
        app_render(&app);
    }

    if (app.glrc)
    {
        wglMakeCurrent(0, 0);
        wglDeleteContext(app.glrc);
    }

    if (app.dc)
    {
        ReleaseDC(app.window, app.dc);
    }

    RenderMode mode = RenderMode_None;

    arena_release(&app.memory.permanent);
    arena_release(&app.memory.frame);
    arena_release(&app.memory.scratch);

    return 0;
}

