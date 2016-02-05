/* x86.h -- x86 cpu magic
 * Copyright (C) 2009-2011 Jan Seiffert
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ifndef X86_H
#define X86_H

#if GCC_VERSION_GE(207)
#  define GCC_ATTR_CONSTRUCTOR __attribute__((__constructor__))
#else
#  define VEC_NO_GO
#endif

#ifdef __x86_64__
#  define PICREG "%%rbx"
#else
#  define PICREG "%%ebx"
#endif

/* Flags */
#define CFF_DEFAULT (1 << 0)
/* Processor features */
#define CFEATURE_CMOV   (15 +   0)
#define CFEATURE_MMX    (23 +   0)
#define CFEATURE_SSE    (25 +   0)
#define CFEATURE_SSE2   (26 +   0)
#define CFEATURE_SSSE3  ( 9 +  32)
#define CFEATURE_SSE4_1 (19 +  32)

#define CFB(x) (1 << ((x)%32))

#define FEATURE_WORDS 2

/* ========================================================================= */
/* data structure */
struct test_cpu_feature
{
    int f_type;
    int flags;
    unsigned int features[FEATURE_WORDS];
};

int ZLIB_INTERNAL _test_cpu_feature OF((const struct test_cpu_feature *t, unsigned int l));
#endif
