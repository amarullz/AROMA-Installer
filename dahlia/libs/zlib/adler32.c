/* adler32.c -- compute the Adler-32 checksum of a data stream
 * Copyright (C) 1995-2011 Mark Adler
 * Copyright (C) 2010-2011 Jan Seiffert
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#include "zutil.h"

#define local static

local uLong adler32_combine_ OF((uLong adler1, uLong adler2, z_off64_t len2));

#define BASE 65521      /* largest prime smaller than 65536 */
#define NMAX 5552
/* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */

#define DO1(buf,i)  {adler += (buf)[i]; sum2 += adler;}
#define DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#define DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#define DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#define DO16(buf)   DO8(buf,0); DO8(buf,8);

#if defined(__alpha__)
/* even if GCC can generate a mul by inverse, the code is really
 * ugly (find global const pool pointer, load constant, a mul, lots
 * of shifts/add/sub), up to 14 instructions. The replacement code
 * only needs >= 5 instructions
 */
#  define NO_DIVIDE
#elif defined(__mips__)
// TODO: i hate synthetized processors
/*
 * If we have a full "high-speed" Multiply/Divide Unit,
 * the old multiply-by-reciproc should be the best way
 * (since then we should get a 32x32 mul in 2 cycles?),
 * but wait, we need 4 muls == 8 + 2 shift + 2 sub + 2 load
 * imidiate + other.
 * If we do not have the "full" MDU, a mul takes 32 cycles
 * and a div 25 (?!?).
 * GCC generates a classic div, prop. needs the right -mtune
 * for a mul.
 * Use our hand rolled reduce, 17 simple instructions for both
 * operands.
 */
#  define NO_DIVIDE
#endif

/* use NO_DIVIDE if your processor does not do division in hardware --
   try it both ways to see which is faster */
#ifdef NO_DIVIDE
/* note that this assumes BASE is 65521, where 65536 % 65521 == 15
   (thank you to John Reiser for pointing this out) */
#  define CHOP(a) \
    do { \
        unsigned long tmp = a >> 16; \
        a &= 0xffffUL; \
        a += (tmp << 4) - tmp; \
    } while (0)
#  define MOD28(a) \
    do { \
        CHOP(a); \
        if (a >= BASE) a -= BASE; \
    } while (0)
#  define MOD(a) \
    do { \
        CHOP(a); \
        MOD28(a); \
    } while (0)
#  define MOD63(a) \
    do { /* this assumes a is not negative */ \
        z_off64_t tmp = a >> 32; \
        a &= 0xffffffffL; \
        a += (tmp << 8) - (tmp << 5) + tmp; \
        tmp = a >> 16; \
        a &= 0xffffL; \
        a += (tmp << 4) - tmp; \
        tmp = a >> 16; \
        a &= 0xffffL; \
        a += (tmp << 4) - tmp; \
        if (a >= BASE) a -= BASE; \
    } while (0)
#else
#  define CHOP(a) a %= BASE
#  define MOD(a) a %= BASE
#  define MOD28(a) a %= BASE
#  define MOD63(a) a %= BASE
#endif

local int host_is_bigendian()
{
    local const union {
        uInt d;
        unsigned char endian[sizeof(uInt)];
    } x = {1};
    return x.endian[0] == 0;
}

#ifndef NO_ADLER32_VEC
#  if defined(__arm__)
#    include "adler32_arm.c"
#  elif defined(__alpha__)
#    include "alpha/adler32.c"
#  elif defined(__bfin__)
#    include "bfin/adler32.c"
#  elif defined(__ia64__)
#    include "ia64/adler32.c"
#  elif defined(__mips__)
#    include "mips/adler32.c"
#  elif defined(__powerpc__) || defined(__powerpc64__)
#    include "ppc/adler32.c"
#  elif defined(__sparc) || defined(__sparc__)
#    include "sparc/adler32.c"
#  elif defined(__i386__) || defined(__x86_64__)
#    include "x86/adler32.c"
#  endif
#endif

#ifndef MIN_WORK
#  define MIN_WORK 16
#endif

/* ========================================================================= */
local noinline uLong adler32_1(adler, buf, len)
    uLong adler;
    const Bytef *buf;
    uInt len GCC_ATTR_UNUSED_PARAM;
{
    unsigned long sum2;

    /* split Adler-32 into component sums */
    sum2 = (adler >> 16) & 0xffff;
    adler &= 0xffff;

    adler += buf[0];
    if (adler >= BASE)
        adler -= BASE;
    sum2 += adler;
    if (sum2 >= BASE)
        sum2 -= BASE;
    return adler | (sum2 << 16);
}

/* ========================================================================= */
local noinline uLong adler32_common(adler, buf, len)
    uLong adler;
    const Bytef *buf;
    uInt len;
{
    unsigned long sum2;

    /* split Adler-32 into component sums */
    sum2 = (adler >> 16) & 0xffff;
    adler &= 0xffff;

    while (len--) {
        adler += *buf++;
        sum2 += adler;
    }
    if (adler >= BASE)
        adler -= BASE;
    MOD28(sum2);             /* only added so many BASE's */
    return adler | (sum2 << 16);
}

#ifndef HAVE_ADLER32_VEC
#  if (defined(__LP64__) || ((SIZE_MAX-0) >> 31) >= 2) && !defined(NO_ADLER32_VEC)

/* On 64 Bit archs, we can do pseudo SIMD with a nice win.
 * This is esp. important for old Alphas, they do not have byte
 * access.
 * This needs some register but x86_64 is fine (>= 9 for the mainloop
 * req.). If your 64 Bit arch is more limited, throw it away...
 */
#    undef VNMAX
#    define VNMAX (2*NMAX+((9*NMAX)/10))

/* ========================================================================= */
local noinline uLong adler32_vec(adler, buf, len)
    uLong adler;
    const Bytef *buf;
    uInt len;
{
    unsigned int s1, s2;
    unsigned int k;

    /* split Adler-32 into component sums */
    s1 = adler & 0xffff;
    s2 = (adler >> 16) & 0xffff;

    /* align input data */
    k    = ALIGN_DIFF(buf, sizeof(size_t));
    len -= k;
    if (k) do {
        s1 += *buf++;
        s2 += s1;
    } while(--k);

    k = len > VNMAX ? VNMAX : len;
    len -= k;
    if (likely(k >= 2 * sizeof(size_t))) do
    {
        unsigned int vs1, vs2;
        unsigned int vs1s;

        /* add s1 to s2 for rounds to come */
        s2 += s1 * ROUND_TO(k, sizeof(size_t));
        vs1s = vs1 = vs2 = 0;
        do {
            size_t vs1l = 0, vs1h = 0, vs1l_s = 0, vs1h_s = 0;
            unsigned int a, b, c, d, e, f, g, h;
            unsigned int j;

            j = k > 23 * sizeof(size_t) ? 23 : k/sizeof(size_t);
            k -= j * sizeof(size_t);
            /* add s1 to s1 round sum for rounds to come */
            vs1s += j * vs1;
            do {
                size_t in8 = *(const size_t *)buf;
                buf += sizeof(size_t);
                /* add this s1 to s1 round sum */
                vs1l_s += vs1l;
                vs1h_s += vs1h;
                /* add up input data to s1 */
                vs1l +=  in8 & UINT64_C(0x00ff00ff00ff00ff);
                vs1h += (in8 & UINT64_C(0xff00ff00ff00ff00)) >> 8;
            } while(--j);

            /* split s1 */
            if(host_is_bigendian()) {
                a = (vs1h >> 48) & 0x0000ffff;
                b = (vs1l >> 48) & 0x0000ffff;
                c = (vs1h >> 32) & 0x0000ffff;
                d = (vs1l >> 32) & 0x0000ffff;
                e = (vs1h >> 16) & 0x0000ffff;
                f = (vs1l >> 16) & 0x0000ffff;
                g = (vs1h      ) & 0x0000ffff;
                h = (vs1l      ) & 0x0000ffff;
            } else {
                a = (vs1l      ) & 0x0000ffff;
                b = (vs1h      ) & 0x0000ffff;
                c = (vs1l >> 16) & 0x0000ffff;
                d = (vs1h >> 16) & 0x0000ffff;
                e = (vs1l >> 32) & 0x0000ffff;
                f = (vs1h >> 32) & 0x0000ffff;
                g = (vs1l >> 48) & 0x0000ffff;
                h = (vs1h >> 48) & 0x0000ffff;
            }

            /* add s1 & s2 horiz. */
            vs2 += 8*a + 7*b + 6*c + 5*d + 4*e + 3*f + 2*g + 1*h;
            vs1 += a + b + c + d + e + f + g + h;

            /* split and add up s1 round sum */
            vs1l_s = ((vs1l_s      ) & UINT64_C(0x0000ffff0000ffff)) +
                     ((vs1l_s >> 16) & UINT64_C(0x0000ffff0000ffff));
            vs1h_s = ((vs1h_s      ) & UINT64_C(0x0000ffff0000ffff)) +
                     ((vs1h_s >> 16) & UINT64_C(0x0000ffff0000ffff));
            vs1l_s += vs1h_s;
            vs1s += ((vs1l_s      ) & UINT64_C(0x00000000ffffffff)) +
                    ((vs1l_s >> 32) & UINT64_C(0x00000000ffffffff));
        } while (k >= sizeof(size_t));
        CHOP(vs1s);
        s2 += vs1s * 8 + vs2;
        CHOP(s2);
        s1 += vs1;
        CHOP(s1);
        len += k;
        k = len > VNMAX ? VNMAX : len;
        len -= k;
    } while (k >= sizeof(size_t));

    /* handle trailer */
    if (k) do {
        s1 += *buf++;
        s2 += s1;
    } while (--k);
    MOD28(s1);
    MOD28(s2);

    /* return recombined sums */
    return (s2 << 16) | s1;
}

#  else

/* ========================================================================= */
local noinline uLong adler32_vec(adler, buf, len)
    uLong adler;
    const Bytef *buf;
    uInt len;
{
    unsigned long sum2;
    unsigned n;

    /* split Adler-32 into component sums */
    sum2 = (adler >> 16) & 0xffff;
    adler &= 0xffff;

    /* do length NMAX blocks -- requires just one modulo operation */
    while (len >= NMAX) {
        len -= NMAX;
        n = NMAX / 16;          /* NMAX is divisible by 16 */
        do {
            DO16(buf);          /* 16 sums unrolled */
            buf += 16;
        } while (--n);
        MOD(adler);
        MOD(sum2);
    }

    /* do remaining bytes (less than NMAX, still just one modulo) */
    if (len) {                  /* avoid modulos if none remaining */
        while (len >= 16) {
            len -= 16;
            DO16(buf);
            buf += 16;
        }
        while (len--) {
            adler += *buf++;
            sum2 += adler;
        }
        MOD(adler);
        MOD(sum2);
    }

    /* return recombined sums */
    return adler | (sum2 << 16);
}
#  endif
#endif

/* ========================================================================= */
#if MIN_WORK - 16 > 0
#  ifndef NO_ADLER32_GE16
local noinline uLong adler32_ge16(adler, buf, len)
    uLong adler;
    const Bytef *buf;
    uInt len;
{
    unsigned long sum2;
    unsigned n;

    /* split Adler-32 into component sums */
    sum2 = (adler >> 16) & 0xffff;
    adler &= 0xffff;
    n = len / 16;
    len %= 16;

    do {
        DO16(buf); /* 16 sums unrolled */
        buf += 16;
    } while (--n);

    /* handle trailer */
    while (len--) {
        adler += *buf++;
        sum2 += adler;
    }

    MOD28(adler);
    MOD28(sum2);

    /* return recombined sums */
    return adler | (sum2 << 16);
}
#  endif
#  define COMMON_WORK 16
#else
#  define COMMON_WORK MIN_WORK
#endif

/* ========================================================================= */
uLong ZEXPORT adler32(adler, buf, len)
    uLong adler;
    const Bytef *buf;
    uInt len;
{
    /* in case user likes doing a byte at a time, keep it fast */
    if (len == 1)
        return adler32_1(adler, buf, len); /* should create a fast tailcall */

    /* initial Adler-32 value (deferred check for len == 1 speed) */
    if (buf == Z_NULL)
        return 1L;

    /* in case short lengths are provided, keep it somewhat fast */
    if (len < COMMON_WORK)
        return adler32_common(adler, buf, len);
#if MIN_WORK - 16 > 0
    if (len < MIN_WORK)
        return adler32_ge16(adler, buf, len);
#endif

    return adler32_vec(adler, buf, len);
}

/* ========================================================================= */
local uLong adler32_combine_(adler1, adler2, len2)
    uLong adler1;
    uLong adler2;
    z_off64_t len2;
{
    unsigned long sum1;
    unsigned long sum2;
    unsigned rem;

    /* for negative len, return invalid adler32 as a clue for debugging */
    if (len2 < 0)
        return 0xffffffffUL;

    /* the derivation of this formula is left as an exercise for the reader */
    MOD63(len2);                /* assumes len2 >= 0 */
    rem = (unsigned)len2;
    sum1 = adler1 & 0xffff;
    sum2 = rem * sum1;
    MOD(sum2);
    sum1 += (adler2 & 0xffff) + BASE - 1;
    sum2 += ((adler1 >> 16) & 0xffff) + ((adler2 >> 16) & 0xffff) + BASE - rem;
    if (sum1 >= BASE) sum1 -= BASE;
    if (sum1 >= BASE) sum1 -= BASE;
    if (sum2 >= (BASE << 1)) sum2 -= (BASE << 1);
    if (sum2 >= BASE) sum2 -= BASE;
    return sum1 | (sum2 << 16);
}

/* ========================================================================= */
uLong ZEXPORT adler32_combine(adler1, adler2, len2)
    uLong adler1;
    uLong adler2;
    z_off_t len2;
{
    return adler32_combine_(adler1, adler2, len2);
}

uLong ZEXPORT adler32_combine64(adler1, adler2, len2)
    uLong adler1;
    uLong adler2;
    z_off64_t len2;
{
    return adler32_combine_(adler1, adler2, len2);
}
