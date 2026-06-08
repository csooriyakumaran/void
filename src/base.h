#ifndef VOID_BASE__H_
#define VOID_BASE__H_

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <stdint.h>
#include <stddef.h>

// --- [ B A S E - T Y P E S ] ---

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float    f32;
typedef double   f64;

typedef int8_t   b8;
typedef int32_t  b32;

typedef struct str8
{
    u8* data;
    u64 size;
} str8;

#define STR(s) ((str8){ (u8*)(s), sizeof(s) - 1 })

#define ARRAY_COUNT(a) (sizeof(a) / sizeof((a)[0]))

#define ASSERT(x) do { if (!(x)) __debugbreak(); } while (0)

#define BIT(x)  (1ull << (x))

// --- [ U N I T S ] ---

#define KB(n) (((u64)(n)) << 10)
#define MB(n) (((u64)(n)) << 20)
#define GB(n) (((u64)(n)) << 30)
#define TB(n) (((u64)(n)) << 40)

// --- [ A R E N A S ] ---
#define ARENA_ALIGN sizeof(void*)

typedef struct Arena
{
    u8* base;
    u64 reserved_size;
    u64 commit_size;
    u64 pos;
} Arena;

Arena arena_alloc(u64 reserve_size, u64 commit_size);
void  arena_release(Arena* arena);

void* arena_push(Arena* arena, u64 size, u64 align, b32 zero);
void  arena_pop(Arena* arena, u64 amt);
void  arena_pop_to(Arena* arena, u64 pos);

void  arena_clear(Arena* arena);

#define arena_push_t(arena, T) (T*)arena_push((arena), sizeof(T), ARENA_ALIGN, 1)
#define arena_push_t_nozero(arena, T) (T*)arena_push((arena), sizeof(T), ARENA_ALIGN, 0)

#define arena_push_array(arena, T, n) (T*)arena_push((arena), sizeof(T) * (n), ARENA_ALIGN, 1)
#define arena_push_array_nozero(arena, T, n) (T*)arena_push((arena), sizeof(T) * (n), ARENA_ALIGN, 0)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // VOID_BASE__H_

