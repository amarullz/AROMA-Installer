/* slhash.c -- slide the hash table during fill_window()
 * Copyright (C) 1995-2010 Jean-loup Gailly and Mark Adler
 * Copyright (C) 2011 Jan Seiffert
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#include "x86.h"

/* inline asm, so only on GCC (or compatible) */
#if defined(__GNUC__) && !defined(VEC_NO_GO)
#  define HAVE_SLHASH_VEC
#  define HAVE_SLHASH_COMPLETE

#define IGNORE_MMX

local noinline void update_hoffset_x86(Posf *p, uInt wsize, unsigned n);
local noinline void slhash_x86(Posf *p, Posf *q, uInt wsize, unsigned n);
local noinline void slhash_SSE2(Posf *p, Posf *q, uInt wsize, unsigned n);

/* NOTE:
 * We do not precheck the length or wsize for small values because
 * we assume a minimum len of 256 (for MEM_LEVEL 1) and a minimum wsize
 * of 256 for windowBits 8
 */

/* ========================================================================= */
/* This is totally bogus, because the Pos type is only 16 bit, and as soon as
 * wsize > 65534, we can not hold the distances in a Pos. All this is a
 * kind of complicated memset 0.
 */
local void update_hoffset_SSE4_1(Posf *p, uInt wsize, unsigned n)
{
    register unsigned m;
    unsigned int i, j;

    i  = ALIGN_DIFF(p, 8)/sizeof(Pos);
    n -= i;
    if (unlikely(i)) do {
        m = *p;
        *p++ = (Pos)(m >= wsize ? m-wsize : NIL);
    } while (--i);
    i = n / 4;
    n %= 4;
    asm (
        "pxor %%xmm6, %%xmm6\n\t"
        "movd %k3, %%xmm7\n\t"
        "pshufd $0, %%xmm7, %%xmm7\n\t"
        "test $8, %0\n\t"
        "jz 2f\n\t"
        "movq (%0), %%xmm0\n\t"
        "add $8, %0\n\t"
        "dec %1\n\t"
        "punpcklwd %%xmm6, %%xmm0\n\t"
        "psubd %%xmm7, %%xmm0\n\t"
        "packusdw %%xmm6, %%xmm0\n\t"
        "movq %%xmm0, -8(%0)\n"
        "2:\n\t"
        "mov %1, %2\n\t"
        "shr $1, %1\n\t"
        "and $1, %2\n\t"
        ".p2align 3\n"
        "1:\n\t"
        "movdqa (%0), %%xmm0\n\t"
        "add $16, %0\n\t"
        "movdqa %%xmm0, %%xmm1\n\t"
        "punpcklwd %%xmm6, %%xmm0\n\t"
        "punpckhwd %%xmm6, %%xmm1\n\t"
        "psubd %%xmm7, %%xmm0\n\t"
        "psubd %%xmm7, %%xmm1\n\t"
        "packusdw %%xmm1, %%xmm0\n\t"
        "movdqa %%xmm0, -16(%0)\n\t"
        "dec %1\n\t"
        "jnz 1b\n\t"
        "test %2, %2\n\t"
        "jz 3f\n\t"
        "movq (%0), %%xmm0\n\t"
        "add $8, %0\n\t"
        "punpcklwd %%xmm6, %%xmm0\n\t"
        "psubd %%xmm7, %%xmm0\n\t"
        "packusdw %%xmm6, %%xmm0\n\t"
        "movq %%xmm0, -8(%0)\n"
        "3:"
        : /* %0 */ "=r" (p),
          /* %1 */ "=r" (i),
          /* %2 */ "=r" (j)
        : /* %3 */ "r" (wsize),
          /*  */ "0" (p),
          /*  */ "1" (i)
#  ifdef __SSE2__
        : "xmm0", "xmm7"
#  endif
    );
    if (unlikely(n))
        update_hoffset_x86(p, wsize, n);
}

/* ========================================================================= */
local void slhash_SSE4_1(Posf *p, Posf *q, uInt wsize, unsigned n)
{
    if (likely(wsize <= (1<<16)-1)) {
        slhash_SSE2(p, q, wsize, n);
        return;
    }

    update_hoffset_SSE4_1(p, wsize, n);
#  ifndef FASTEST
    /* If n is not on any hash chain, prev[n] is garbage but
     * its value will never be used.
     */
    update_hoffset_SSE4_1(q, wsize, wsize);
#  endif
}

/* ========================================================================= */
local void update_hoffset_SSE2(Posf *p, uInt wsize, unsigned n)
{
    register unsigned m;
    unsigned int i, j;

    i  = ALIGN_DIFF(p, 8)/sizeof(Pos);
    n -= i;
    if (unlikely(i)) do {
        m = *p;
        *p++ = (Pos)(m >= wsize ? m-wsize : NIL);
    } while (--i);
    i = n / 4;
    n %= 4;
    asm (
        "movd %k3, %%xmm7\n\t"
        "pshuflw $0, %%xmm7, %%xmm7\n\t"
        "pshufd $0, %%xmm7, %%xmm7\n\t"
        "test $8, %0\n\t"
        "jz 2f\n\t"
        "movq (%0), %%xmm0\n\t"
        "add $8, %0\n\t"
        "dec %1\n\t"
        "psubusw %%xmm7, %%xmm0\n\t"
        "movq %%xmm0, -8(%0)\n\t"
        "2:\n\t"
        "mov %1, %2\n\t"
        "shr $1, %1\n\t"
        "and $1, %2\n\t"
        ".p2align 3\n"
        "1:\n\t"
        "movdqa (%0), %%xmm0\n\t"
        "add $16, %0\n\t"
        "psubusw %%xmm7, %%xmm0\n\t"
        "movdqa %%xmm0, -16(%0)\n\t"
        "dec %1\n\t"
        "jnz 1b\n\t"
        "test %2, %2\n\t"
        "jz 3f\n\t"
        "movq (%0), %%xmm0\n\t"
        "add $8, %0\n\t"
        "psubusw %%xmm7, %%xmm0\n\t"
        "movq %%xmm0, -8(%0)\n\t"
        "3:"
        : /* %0 */ "=r" (p),
          /* %1 */ "=r" (i),
          /* %2 */ "=r" (j)
        : /* %3 */ "r" (wsize),
          /*  */ "0" (p),
          /*  */ "1" (i)
#  ifdef __SSE2__
        : "xmm0", "xmm7"
#  endif
    );
    if (unlikely(n))
        update_hoffset_x86(p, wsize, n);
}

/* ========================================================================= */
local noinline void slhash_SSE2(Posf *p, Posf *q, uInt wsize, unsigned n)
{
    if (unlikely(wsize > (1 << 16)-1)) {
        slhash_x86(p, q, wsize, n);
        return;
    }

    update_hoffset_SSE2(p, wsize, n);
#  ifndef FASTEST
    /* If n is not on any hash chain, prev[n] is garbage but
     * its value will never be used.
     */
    update_hoffset_SSE2(q, wsize, wsize);
#  endif
}

#  ifndef __x86_64__
#  ifndef IGNORE_MMX
/* ========================================================================= */
local void update_hoffset_MMX(Posf *p, uInt wsize, unsigned n)
{
    register unsigned m;
    unsigned int i;

    i  = ALIGN_DIFF(p, 8)/sizeof(Pos);
    n -= i;
    if (unlikely(i)) do {
        m = *p;
        *p++ = (Pos)(m >= wsize ? m-wsize : NIL);
    } while (--i);
    i = n / 4;
    n %= 4;
    asm (
        "movd %k2, %%mm7\n\t"
        "pshufw $0, %%mm7, %%mm7\n\t"
        ".p2align 2\n"
        "1:\n\t"
        "movq (%0), %%mm0\n\t"
        "add $8, %0\n\t"
        "psubusw %%mm7, %%mm0\n\t"
        "movq %%xmm0, -8(%0)\n\t"
        "dec %1\n\t"
        "jnz 1b"
        : /* %0 */ "=r" (p),
          /* %1 */ "=r" (i)
        : /* %2 */ "r" (wsize),
          /*  */ "0" (p),
          /*  */ "1" (i)
#    ifdef __MMX__
        : "mm0", "mm7"
#    endif
    );
    if (unlikely(n))
        update_hoffset_x86(p, wsize, n);
}

/* ========================================================================= */
local noinline void slhash_MMX(Posf *p, Posf *q, uInt wsize, unsigned n)
{
    if (unlikely(wsize > (1 << 16)-1)) {
        slhash_x86(p, q, wsize, n);
        return;
    }

    update_hoffset_MMX(p, wsize, n);
#    ifndef FASTEST
    /* If n is not on any hash chain, prev[n] is garbage but
     * its value will never be used.
     */
    update_hoffset_MMX(q, wsize, wsize);
#    endif
    asm volatile ("emms");
}
#  endif
#  endif
/* ========================================================================= */
local noinline void update_hoffset_x86(Posf *p, uInt wsize, unsigned n)
{
    /*
     * This code is cheaper then a cmov, measuring whole loops with
     * rdtsc:
     * This code:  593216
     * compiler:  1019864
     * (and 1000 runs show the same trend)
     * Old CPUs without cmov will also love it, better then jumps.
     *
     * GCC does not manage to create it, x86 is a cc_mode target,
     * and prop. will stay forever.
     */
    do {
        register unsigned m = *p;
        unsigned t;
        asm (
            "sub	%2, %0\n\t"
            "sbb	$0, %1\n\t"
            : "=r" (m),
              "=r" (t)
            : "r" (wsize),
              "0" (m),
              "1" (0)
        );
        *p++ = (Pos)(m & ~t);
    } while (--n);
}

/* ========================================================================= */
local noinline void slhash_x86(Posf *p, Posf *q, uInt wsize, unsigned n)
{
    update_hoffset_x86(p, wsize, n);
#  ifndef FASTEST
    /* If n is not on any hash chain, prev[n] is garbage but
     * its value will never be used.
     */
    update_hoffset_x86(q, wsize, wsize);
#  endif
}

/*
 * Knot it all together with a runtime switch
 */
/* ========================================================================= */
/* function enum */
enum slhash_types
{
    T_SLHASH_RTSWITCH = 0,
    T_SLHASH_X86,
#  ifndef __x86_64__
#  ifndef IGNORE_MMX
    T_SLHASH_MMX,
#  endif
#  endif
    T_SLHASH_SSE2,
    T_SLHASH_SSE4_1,
    T_SLHASH_MAX
};

/* ========================================================================= */
/* Decision table */
local const struct test_cpu_feature tfeat_slhash_vec[] =
{
    /* func               flags  features       */
    {T_SLHASH_SSE4_1,        0, {0,                  CFB(CFEATURE_SSE4_1)}},
    {T_SLHASH_SSE2,          0, {CFB(CFEATURE_SSE2),                    0}},
#  ifndef __x86_64__
#  ifndef IGNORE_MMX
    {T_SLHASH_MMX,           0, {CFB(CFEATURE_MMX),                     0}},
#  endif
#  endif
    {T_SLHASH_X86, CFF_DEFAULT, { 0, 0}},
};

/* ========================================================================= */
/* Prototypes */
local void slhash_vec_runtimesw(Posf *p, Posf *q, uInt wsize, unsigned n);

/* ========================================================================= */
/* Function pointer table */
local void (*const slhash_ptr_tab[])(Posf *p, Posf *q, uInt wsize, unsigned n) =
{
    slhash_vec_runtimesw,
    slhash_x86,
#  ifndef __x86_64__
#  ifndef IGNORE_MMX
    slhash_MMX,
#  endif
#  endif
    slhash_SSE2,
    slhash_SSE4_1,
};

/* ========================================================================= */
#  if _FORTIFY_SOURCE-0 > 0
/* Runtime decide var */
local enum slhash_types slhash_f_type = T_SLHASH_RTSWITCH;
#  else
/* Runtime Function pointer */
local void (*slhash_vec_ptr)(Posf *p, Posf *q, uInt wsize, unsigned n) = slhash_vec_runtimesw;
#  endif

/* ========================================================================= */
/* Constructor to init the decide var early */
local GCC_ATTR_CONSTRUCTOR void slhash_vec_select(void)
{
    enum slhash_types lf_type =
        _test_cpu_feature(tfeat_slhash_vec, sizeof (tfeat_slhash_vec)/sizeof (tfeat_slhash_vec[0]));
#  if _FORTIFY_SOURCE-0 > 0
    slhash_f_type = lf_type;
#  else
    slhash_vec_ptr = slhash_ptr_tab[lf_type];
#  endif
}

/* ========================================================================= */
/* Jump function */
void ZLIB_INTERNAL _sh_slide (p, q, wsize, n)
    Posf *p;
    Posf *q;
    uInt wsize;
    unsigned n;
{
    /*
     * Protect us from memory corruption. As long as the function pointer table
     * resides in rodata, with a little bounding we can prevent arb. code
     * execution (overwriten vtable pointer). We still may crash if the corruption
     * is within bounds (or the cpudata gets corrupted too) and we jump into an
     * function with unsupported instr., but this should mitigate the worst case
     * scenario.
     * But it's more expensive than a simple function pointer, so only when more
     * security is wanted.
     */
#  if _FORTIFY_SOURCE-0 > 0
    enum slhash_types lf_type = slhash_f_type;
    /*
     * If the compiler is smart he creates a cmp + sbb + and, cmov have a high
     * latency and are not always avail.
     * Otherwise compiler logic is advanced enough to see what's happening here,
     * so there maybe is a reason why he changes this to a cmov...
     * (or he simply does not see he can create a conditional -1/0 the cheap way)
     *
     * Maybe change it to an unlikely() cbranch? Which still leaves the question
     * what's the mispredition propability, esp. with lots of different x86
     * microarchs and not always perfect CFLAGS (-march/-mtune) to arrange the
     * code to the processors liking.
     */
    lf_type &= likely((unsigned)lf_type < (unsigned)T_SLHASH_MAX) ? -1 : 0;
    return slhash_ptr_tab[lf_type](p, q, wsize, n);
#  else
    return slhash_vec_ptr(p, q, wsize, n);
#  endif
}

/* ========================================================================= */
/*
 * the runtime switcher is a little racy, but this is OK,
 * it should normaly not run if the constructor works, and
 * we are on x86, which isn't that picky about ordering
 */
local void slhash_vec_runtimesw(Posf *p, Posf *q, uInt wsize, unsigned n)
{
    slhash_vec_select();
    return _sh_slide(p, q, wsize, n);
}
#endif
