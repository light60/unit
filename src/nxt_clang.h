
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) NGINX, Inc.
 */

#ifndef _NXT_CLANG_H_INCLUDED_
#define _NXT_CLANG_H_INCLUDED_


#define nxt_inline     static inline __attribute__((always_inline))
#define nxt_noinline   __attribute__((noinline))
#define nxt_cdecl


#if (NXT_CLANG)

/* Any __asm__ directive disables loop vectorization in GCC and Clang. */
#define                                                                       \
nxt_pragma_loop_disable_vectorization                                         \
    __asm__("")

#else

#define                                                                       \
nxt_pragma_loop_disable_vectorization

#endif


#if (NXT_HAVE_BUILTIN_EXPECT)

#define                                                                       \
nxt_fast_path(x)                                                              \
    __builtin_expect((long) (x), 1)

#define                                                                       \
nxt_slow_path(x)                                                              \
    __builtin_expect((long) (x), 0)


#else

#define                                                                       \
nxt_fast_path(x)                                                              \
    (x)

#define                                                                       \
nxt_slow_path(x)                                                              \
    (x)

#endif


#if (NXT_HAVE_BUILTIN_UNREACHABLE)

#define                                                                       \
nxt_unreachable()                                                             \
    __builtin_unreachable()

#else

#define                                                                       \
nxt_unreachable()

#endif


#if (NXT_HAVE_BUILTIN_PREFETCH)

#define                                                                       \
nxt_prefetch(a)                                                               \
    __builtin_prefetch(a)

#else

#define                                                                       \
nxt_prefetch(a)

#endif


#if (NXT_HAVE_GCC_ATTRIBUTE_VISIBILITY)

#define NXT_EXPORT         __attribute__((visibility("default")))

#else

#define NXT_EXPORT

#endif


#if (NXT_HAVE_GCC_ATTRIBUTE_MALLOC)

#define NXT_MALLOC_LIKE    __attribute__((__malloc__))

#else

#define NXT_MALLOC_LIKE

#endif


#if (NXT_HAVE_GCC_ATTRIBUTE_ALIGNED)

#define nxt_aligned(x)     __attribute__((aligned(x)))

#else

#define nxt_aligned(x)

#endif


#ifndef NXT_ALIGNMENT

#if (NXT_SOLARIS)
#define NXT_ALIGNMENT      _POINTER_ALIGNMENT     /* x86_64: 8,   i386: 4    */
                                                  /* sparcv9: 8,  sparcv8: 4 */
#elif (__i386__ || __i386)
#define NXT_ALIGNMENT      4

#elif (__arm__)
#define NXT_ALIGNMENT      8         /* 32-bit ARM may use 64-bit load/store */

#elif (__ia64__)
#define NXT_ALIGNMENT      8         /* long long */

#else
#define NXT_ALIGNMENT      NXT_PTR_SIZE
#endif

#endif


#ifndef NXT_MAX_ALIGNMENT

#if (NXT_SOLARIS)
#define NXT_MAX_ALIGNMENT  _MAX_ALIGNMENT        /* x86_64: 16,   i386: 4    */
                                                 /* sparcv9: 16,  sparcv8: 8 */
#elif (__i386__ || __i386)
#define NXT_MAX_ALIGNMENT  4

#elif (__arm__)
#define NXT_MAX_ALIGNMENT  16

#elif (__ia64__)
#define NXT_MAX_ALIGNMENT  16

#else
#define NXT_MAX_ALIGNMENT  16
#endif

#endif


#define                                                                       \
nxt_alloca(size)                                                              \
    alloca(size)


#define                                                                       \
nxt_container_of(p, type, field)                                              \
    (type *) ((u_char *) (p) - offsetof(type, field))


#define                                                                       \
nxt_nitems(x)                                                                 \
    (sizeof(x) / sizeof((x)[0]))


/* GCC and Clang use __builtin_abs() instead of libc abs(). */

#define                                                                       \
nxt_abs(val)                                                                  \
    abs(val)


#define                                                                       \
nxt_max(val1, val2)                                                           \
    ((val1 < val2) ? (val2) : (val1))


#define                                                                       \
nxt_min(val1, val2)                                                           \
    ((val1 > val2) ? (val2) : (val1))


#define                                                                       \
nxt_bswap32(val)                                                              \
    (   ((val)               >> 24)                                           \
     | (((val) & 0x00ff0000) >>  8)                                           \
     | (((val) & 0x0000ff00) <<  8)                                           \
     |  ((val)               << 24))



#define                                                                       \
nxt_align_size(d, a)                                                          \
    (((d) + ((size_t) (a) - 1)) & ~((size_t) (a) - 1))


#define                                                                       \
nxt_align_ptr(p, a)                                                           \
    (u_char *) (((uintptr_t) (p) + ((uintptr_t) (a) - 1))                     \
                  & ~((uintptr_t) (a) - 1))

#define                                                                       \
nxt_trunc_ptr(p, a)                                                           \
    (u_char *) ((uintptr_t) (p) & ~((uintptr_t) (a) - 1))


#endif /* _NXT_CLANG_H_INCLUDED_ */
