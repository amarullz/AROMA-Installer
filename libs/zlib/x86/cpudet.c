/* cpudet.c -- runtime cpu detection, x86 part
 * Copyright (C) 2009-2011 Jan Seiffert
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#include "x86.h"

/* ========================================================================= */
/* Internal data types */
struct cpuid_regs
{
    unsigned long eax, ebx, ecx, edx;
};

local struct
{
    unsigned int max_basic;
    unsigned int features[FEATURE_WORDS];
    int init_done;
} our_cpu;

/* ========================================================================= */
local inline unsigned long read_flags(void)
{
    unsigned long f;
    __asm__ __volatile__ (
            "pushf\n\t"
            "pop %0\n\t"
        : "=r" (f)
    );
    return f;
}

/* ========================================================================= */
local inline void write_flags(unsigned long f)
{
    __asm__ __volatile__ (
            "push %0\n\t"
            "popf\n\t"
        : : "ri" (f) : "cc"
    );
}

/* ========================================================================= */
local inline void cpuid(struct cpuid_regs *regs, unsigned long func)
{
    /* save ebx around cpuid call, PIC code needs it */
    __asm__ __volatile__ (
            "xchg	%1, " PICREG "\n\t"
            "cpuid\n\t"
            "xchg	%1, " PICREG "\n"
        : /* %0 */ "=a" (regs->eax),
          /* %1 */ "=r" (regs->ebx),
          /* %2 */ "=c" (regs->ecx),
          /* %4 */ "=d" (regs->edx)
        : /* %5 */ "0" (func),
          /* %6 */ "2" (regs->ecx)
        : "cc"
    );
}

/* ========================================================================= */
local inline void cpuids(struct cpuid_regs *regs, unsigned long func)
{
    regs->ecx = 0;
    cpuid(regs, func);
}

/* ========================================================================= */
local inline int toggle_eflags_test(const unsigned long mask)
{
    unsigned long f;
    int result;

    f = read_flags();
    write_flags(f ^ mask);
    result = !!((f ^ read_flags()) & mask);
    /*
     * restore the old flags, the test for i486 tests the alignment
     * check bit, and left set will confuse the x86 software world.
     */
    write_flags(f);
    return result;
}

/* ========================================================================= */
local inline int is_486(void)
{
    return toggle_eflags_test(1 << 18);
}

/* ========================================================================= */
local inline int has_cpuid(void)
{
    return toggle_eflags_test(1 << 21);
}

/* ========================================================================= */
local void identify_cpu(void)
{
    struct cpuid_regs a;

    if (our_cpu.init_done)
        return;

    our_cpu.init_done = -1;
    /* force a write out to memory */
    __asm__ __volatile__ ("" : : "m" (our_cpu.init_done));

    if (!is_486())
        return;

    if (!has_cpuid())
        return;

    /* get the maximum basic leaf number */
    cpuids(&a, 0x00000000);
    our_cpu.max_basic = (unsigned int)a.eax;
    /* we could get the vendor string from ebx, edx, ecx */

    /* get the first basic leaf, if it is avail. */
    if (our_cpu.max_basic >= 0x00000001)
        cpuids(&a, 0x00000001);
    else
        a.eax = a.ebx = a.ecx = a.edx = 0;

    /* we could extract family, model, stepping from eax */

    /* there is the first set of features */
    our_cpu.features[0] = a.edx;
    our_cpu.features[1] = a.ecx;

    /* now we could test the extended features, but is not needed, for now */
}

/* ========================================================================= */
int ZLIB_INTERNAL _test_cpu_feature (t, l)
    const struct test_cpu_feature *t;
    unsigned int l;
{
    unsigned int i, j, f;
    identify_cpu();

    for (i = 0; i < l; i++) {
        if (t[i].flags & CFF_DEFAULT)
            return t[i].f_type;
        for (f = 0, j = 0; j < FEATURE_WORDS; j++)
            f |= (our_cpu.features[j] & t[i].features[j]) ^ t[i].features[j];
        if (f)
            continue;
        return t[i].f_type;
    }
    return 1; /* default */
}
