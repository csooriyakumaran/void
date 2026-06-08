#include "base.h"

#include <string.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static u64 align_forward_u64(u64 value, u64 align)
{
    ASSERT(align != 0);
    ASSERT((align & (align - 1)) == 0);

    u64 mask = align - 1;
    return (value + mask) & ~mask;
}

static u64 win32_get_pagesize(void)
{
    SYSTEM_INFO sysinfo = {0};
    GetSystemInfo(&sysinfo);

    return (u64)sysinfo.dwPageSize;
}

static void* win32_mem_reserve(u64 size)
{
    return VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_READWRITE);
}

static b32 win32_mem_commit(void* ptr, u64 size)
{
    void* ret = VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
    return ret != NULL;
}

static b32 win32_mem_decommit(void* ptr, u64 size)
{
    return VirtualFree(ptr, size, MEM_DECOMMIT);
}

static b32 win32_mem_release(void* ptr)
{
    return VirtualFree(ptr, 0, MEM_RELEASE);
}


Arena arena_alloc(u64 reserve_size, u64 commit_size)
{
    Arena arena = {0};

    u64 pagesize = win32_get_pagesize();

    reserve_size = align_forward_u64(reserve_size, pagesize);
    commit_size  = align_forward_u64(commit_size, pagesize);

    arena.base   = (u8*)win32_mem_reserve(reserve_size);

    ASSERT(arena.base);

    b32 committed = win32_mem_commit(arena.base, commit_size);

    ASSERT(committed);

    arena.reserved_size = reserve_size;
    arena.commit_size   = commit_size;
    arena.pos           = 0;

    return arena;
}

void arena_release(Arena* arena)
{
    if (arena->base)
        win32_mem_release(arena->base);

    arena->base          = 0;
    arena->reserved_size = 0;
    arena->commit_size   = 0;
    arena->pos           = 0;
}

void* arena_push(Arena* arena, u64 size, u64 align, b32 zero)
{
    u64 aligned_pos = align_forward_u64(arena->pos, align);
    u64 new_pos     = aligned_pos + size;

    ASSERT(new_pos <= arena->reserved_size);

    if (new_pos > arena->commit_size)
    {
        u64 pagesize = win32_get_pagesize();
        u64 new_commit_size = align_forward_u64(new_pos, pagesize);
        b32 committed = win32_mem_commit(arena->base + arena->commit_size, new_commit_size - arena->commit_size);
        ASSERT(committed);
        arena->commit_size = new_commit_size;
    }

    arena->pos = new_pos;
    void* result = arena->base + aligned_pos;

    if (zero)
        memset(result, 0, size);

    return result;

}

void arena_clear(Arena* arena)
{
    arena->pos = 0;
}


void arena_pop_to(Arena* arena, u64 pos)
{
    if (pos > arena->pos)
        pos = arena->pos;

    arena->pos = pos;
}

void arena_pop(Arena* arena, u64 amt)
{
    if(amt > arena->pos)
        amt = arena->pos;

    arena->pos -= amt;
}
