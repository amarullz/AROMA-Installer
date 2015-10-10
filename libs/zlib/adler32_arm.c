/*
 * adler32.c -- compute the Adler-32 checksum of a data stream
 *   arm implementation
 * Copyright (C) 1995-2007 Mark Adler
 * Copyright (C) 2009-2011 Jan Seiffert
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#if defined(__ARM_NEON__) && defined(__ARMEL__)
/*
 * Big endian NEON qwords are kind of broken.
 * They are big endian within the dwords, but WRONG
 * (really??) way round between lo and hi.
 * Creating some kind of PDP11 middle endian.
 *
 * This is madness and unsupportable. For this reason
 * GCC wants to disable qword endian specific patterns.
 */
#  include <arm_neon.h>

#  define SOVUCQ sizeof(uint8x16_t)
#  define SOVUC sizeof(uint8x8_t)
/* since we do not have the 64bit psadbw sum, we could still go a little higher (we are at 0xc) */
#  define VNMAX (8*NMAX)
#  define HAVE_ADLER32_VEC
#  define MIN_WORK 32

/* ========================================================================= */
local inline uint8x16_t neon_simple_alignq(uint8x16_t a, uint8x16_t b, unsigned amount)
{
    switch(amount % SOVUCQ)
    {
    case  0: return a;
    case  1: return vextq_u8(a, b,  1);
    case  2: return vextq_u8(a, b,  2);
    case  3: return vextq_u8(a, b,  3);
    case  4: return vextq_u8(a, b,  4);
    case  5: return vextq_u8(a, b,  5);
    case  6: return vextq_u8(a, b,  6);
    case  7: return vextq_u8(a, b,  7);
    case  8: return vextq_u8(a, b,  8);
    case  9: return vextq_u8(a, b,  9);
    case 10: return vextq_u8(a, b, 10);
    case 11: return vextq_u8(a, b, 11);
    case 12: return vextq_u8(a, b, 12);
    case 13: return vextq_u8(a, b, 13);
    case 14: return vextq_u8(a, b, 14);
    case 15: return vextq_u8(a, b, 15);
    }
    return b;
}

/* ========================================================================= */
local inline uint32x4_t vector_chop(uint32x4_t x)
{
    uint32x4_t y;

    y = vshlq_n_u32(x, 16);
    x = vshrq_n_u32(x, 16);
    y = vshrq_n_u32(y, 16);
    y = vsubq_u32(y, x);
    x = vaddq_u32(y, vshlq_n_u32(x, 4));
    return x;
}

/* ========================================================================= */
local noinline uLong adler32_vec(adler, buf, len)
    uLong adler;
    const Bytef *buf;
    uInt len;
{
    uint32x4_t v0_32 = (uint32x4_t){0,0,0,0};
    uint8x16_t    v0 = (uint8x16_t)v0_32;
    uint8x16_t vord, vord_a;
    uint32x4_t vs1, vs2;
    uint32x2_t v_tsum;
    uint8x16_t in16;
    uint32_t s1, s2;
    unsigned k;

    s1 = adler & 0xffff;
    s2 = (adler >> 16) & 0xffff;

    vord = (uint8x16_t){16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1};

    if (likely(len >= 2*SOVUCQ)) {
        unsigned f, n;

        /*
         * Add stuff to achieve alignment
         */
        /* align hard down */
        f = (unsigned) ALIGN_DOWN_DIFF(buf, SOVUCQ);
        n = SOVUCQ - f;
        buf = (const unsigned char *)ALIGN_DOWN(buf, SOVUCQ);

        /* add n times s1 to s2 for start round */
        s2 += s1 * n;

        /* set sums 0 */
        vs1 = v0_32;
        vs2 = v0_32;
        /*
         * the accumulation of s1 for every round grows very fast
         * (quadratic?), even if we accumulate in 4 dwords, more
         * rounds means nonlinear growth.
         * We already split it out of s2, normaly it would be in
         * s2 times 16... and even grow faster.
         * Thanks to this split and vector reduction, we can stay
         * longer in the loops. But we have to prepare for the worst
         * (all 0xff), only do 6 times the work.
         * (we could prop. stay a little longer since we have 4 sums,
         * not 2 like on x86).
         */
        k = len < VNMAX ? (unsigned)len : VNMAX;
        len -= k;
        /* insert scalar start somewhere */
        vs1 = vsetq_lane_u32(s1, vs1, 0);
        vs2 = vsetq_lane_u32(s2, vs2, 0);

        /* get input data */
        in16 = *(const uint8x16_t *)buf;
        /* mask out excess data */
        in16 = neon_simple_alignq(in16, v0, f);
        vord_a = neon_simple_alignq(vord, v0, f);
        /* pairwise add bytes and long, pairwise add word long acc */
        vs1 = vpadalq_u16(vs1, vpaddlq_u8(in16));
        /* apply order, add words, pairwise add word long acc */
        vs2 = vpadalq_u16(vs2,
                vmlal_u8(
                    vmull_u8(vget_low_u8(in16), vget_low_u8(vord_a)),
                    vget_high_u8(in16), vget_high_u8(vord_a)
                    )
                );

        buf += SOVUCQ;
        k -= n;

        if (likely(k >= SOVUCQ)) do {
            uint32x4_t vs1_r = v0_32;
            do {
                uint16x8_t vs2_lo = (uint16x8_t)v0_32, vs2_hi = (uint16x8_t)v0_32;
                unsigned j;

                j  = (k/16) > 16 ? 16 : k/16;
                k -= j * 16;
                do {
                    /* GCC does not create the most pretty inner loop,
                     * with extra moves and stupid scheduling, but
                     * i am not in the mood for inline ASM, keep it
                     * compatible.
                     */
                    /* get input data */
                    in16 = *(const uint8x16_t *)buf;
                    buf += SOVUCQ;

                    /* add vs1 for this round */
                    vs1_r = vaddq_u32(vs1_r, vs1);

                    /* pairwise add bytes and long, pairwise add word long acc */
                    vs1 = vpadalq_u16(vs1, vpaddlq_u8(in16));
                    /* apply order, word long and acc */
                    vs2_lo = vmlal_u8(vs2_lo, vget_low_u8(in16), vget_low_u8(vord));
                    vs2_hi = vmlal_u8(vs2_hi, vget_high_u8(in16), vget_high_u8(vord));
                } while(--j);
                /* pair wise add long and acc */
                vs2 = vpadalq_u16(vs2, vs2_lo);
                vs2 = vpadalq_u16(vs2, vs2_hi);
            } while (k >= SOVUCQ);
            /* chop vs1 round sum before multiplying by 16 */
            vs1_r = vector_chop(vs1_r);
            /* add vs1 for this round (16 times) */
            /* they have shift right and accummulate, where is shift left and acc?? */
            vs2 = vaddq_u32(vs2, vshlq_n_u32(vs1_r, 4));
            /* chop both vectors to something within 16 bit */
            vs2 = vector_chop(vs2);
            vs1 = vector_chop(vs1);
            len += k;
            k = len < VNMAX ? (unsigned) len : VNMAX;
            len -= k;
        } while (likely(k >= SOVUCQ));

        if (likely(k)) {
            /*
             * handle trailer
             */
            f = SOVUCQ - k;
            /* add k times vs1 for this trailer */
            vs2 = vmlaq_u32(vs2, vs1, vdupq_n_u32(k));

            /* get input data */
            in16 = *(const uint8x16_t *)buf;
            /* masks out bad data */
            in16 = neon_simple_alignq(v0, in16, k);

            /* pairwise add bytes and long, pairwise add word long acc */
            vs1 = vpadalq_u16(vs1, vpaddlq_u8(in16));
            /* apply order, add words, pairwise add word long acc */
            vs2 = vpadalq_u16(vs2,
                    vmlal_u8(
                        vmull_u8(vget_low_u8(in16), vget_low_u8(vord)),
                        vget_high_u8(in16), vget_high_u8(vord)
                        )
                    );

            buf += k;
            k -= k;
        }

        /* add horizontal */
        v_tsum = vpadd_u32(vget_high_u32(vs1), vget_low_u32(vs1));
        v_tsum = vpadd_u32(v_tsum, v_tsum);
        s1 = vget_lane_u32(v_tsum, 0);
        v_tsum = vpadd_u32(vget_high_u32(vs2), vget_low_u32(vs2));
        v_tsum = vpadd_u32(v_tsum, v_tsum);
        s2 = vget_lane_u32(v_tsum, 0);
    }

    if (unlikely(len)) do {
        s1 += *buf++;
        s2 += s1;
    } while (--len);
    MOD28(s1);
    MOD28(s2);

    return (s2 << 16) | s1;
}

#elif defined(__IWMMXT__)
#  ifndef __GNUC__
/* GCC doesn't take it's own intrinsic header and ICEs if forced to */
#    include <mmintrin.h>
#  else
typedef unsigned long long __m64;

// TODO: older gcc may need U constrain instead of y?
static inline __m64 _mm_setzero_si64(void)
{
    __m64 r;
#  if 0
    asm ("wzero %0" : "=y" (r));
#  else
    r = 0;
#  endif
    return r;
}
/* there is slli/srli and we want to use it, but it's iWMMXt-2 */
static inline __m64 _mm_sll_pi32(__m64 a, __m64 c)
{
    asm ("wsllw %0, %1, %2" : "=y" (a) : "y" (a), "y" (c));
    return a;
}
static inline __m64 _mm_srl_pi32(__m64 a, __m64 c)
{
    asm ("wsrlw %0, %1, %2" : "=y" (a) : "y" (a), "y" (c));
    return a;
}
static inline __m64 _mm_sub_pi32(__m64 a, __m64 b)
{
    asm ("wsubw %0, %1, %2" : "=y" (a) : "y" (a), "y" (b));
    return a;
}
static inline __m64 _mm_add_pi16(__m64 a, __m64 b)
{
    asm ("waddh %0, %1, %2" : "=y" (a) : "y" (a), "y" (b));
    return a;
}
static inline __m64 _mm_add_pi32(__m64 a, __m64 b)
{
    asm ("waddw %0, %1, %2" : "=y" (a) : "y" (a), "y" (b));
    return a;
}
static inline __m64 _mm_sada_pu8(__m64 acc, __m64 a, __m64 b)
{
    asm ("wsadb %0, %1, %2" : "=y" (acc) : "y" (a), "y" (b), "0" (acc));
    return acc;
}
static inline __m64 _mm_madd_pu16(__m64 a, __m64 b)
{
    asm ("wmaddu %0, %1, %2" : "=y" (a) : "y" (a), "y" (b));
    return a;
}
static inline __m64 _mm_mac_pu16(__m64 acc, __m64 a, __m64 b)
{
    asm ("wmacu %0, %1, %2" : "=y" (acc) : "y" (a), "y" (b), "0" (acc));
    return acc;
}
static inline __m64 _mm_unpackel_pu8(__m64 a)
{
    asm ("wunpckelub %0, %1" : "=y" (a) : "y" (a));
    return a;
}
static inline __m64 _mm_unpackeh_pu8(__m64 a)
{
    asm ("wunpckehub %0, %1" : "=y" (a) : "y" (a));
    return a;
}
static inline __m64 _mm_shuffle_pi16(__m64 a, const int m)
{
    asm ("wshufh %0, %1, %2" : "=y" (a) : "y" (a), "i" (m));
    return a;
}
static inline unsigned int _mm_extract_pu32(__m64 a, const int m)
{
    unsigned int r;
    asm ("textrmuw %0, %1, %2" : "=r" (r) : "y" (a), "i" (m));
    return r;
}
static inline __m64 _mm_insert_pi32(__m64 a, unsigned int b, const int m)
{
    asm ("tinsrw %0, %1, %2" : "=y" (a) : "r" (b), "i" (m), "0" (a));
    return a;
}
static inline __m64 _mm_align_si64(__m64 a, __m64 b, int c)
{
    asm ("walignr%U3 %0, %1, %2" : "=y" (a) : "y" (a), "y" (b), "z" (c));
    return a;
}
static inline __m64 _mm_set_pi16(short a, short b, short c, short d)
{
    __m64 r = (unsigned long long)d;
    r |= ((unsigned long long)c) << 16;
    r |= ((unsigned long long)b) << 32;
    r |= ((unsigned long long)a) << 48;
    return r;
}
#  endif

// TODO: we could go over NMAX, since we have split the vs2 sum
/* but we shuffle vs1_r only every 2056 byte, so we can not go full */
#  define VNMAX (3*NMAX)
#  define HAVE_ADLER32_VEC
#  define MIN_WORK 32
#  define SOV8 (sizeof(__m64))

/* ========================================================================= */
local inline __m64 vector_chop(__m64 x)
{
    static const __m64 four = 4;
    static const __m64 sixten = 16;
    __m64 y = _mm_sll_pi32(x, sixten);
    x = _mm_srl_pi32(x, sixten);
    y = _mm_srl_pi32(y, sixten);
    y = _mm_sub_pi32(y, x);
    x = _mm_add_pi32(y, _mm_sll_pi32(x, four));
    return x;
}

/* ========================================================================= */
local noinline uLong adler32_vec(adler, buf, len)
    uLong adler;
    const Bytef *buf;
    uInt len;
{
    unsigned int s1, s2;
    unsigned int k;

    s1 = adler & 0xffff;
    s2 = (adler >> 16) & 0xffff;

    if (likely(len >= 4 * SOV8)) {
        static const __m64 three = 3;
        __m64 vs1, vs2;
        __m64 vzero;
        __m64 vorder_l, vorder_h;
        unsigned int f, n;

        vzero = _mm_setzero_si64();

        /* align hard down */
        f = (unsigned int) ALIGN_DOWN_DIFF(buf, SOV8);
        buf = (const Bytef *)ALIGN_DOWN(buf, SOV8);
        n = SOV8 - f;

        /* add n times s1 to s2 for start round */
        s2 += s1 * n;

        k = len < VNMAX ? len : VNMAX;
        len -= k;

        /* insert scalar start */
        vs1 = _mm_insert_pi32(vzero, s1, 0);
        vs2 = _mm_insert_pi32(vzero, s2, 0);

// TODO: byte order?
        if (host_is_bigendian()) {
            vorder_l = _mm_set_pi16(4, 3, 2, 1);
            vorder_h = _mm_set_pi16(8, 7, 6, 5);
        } else {
            vorder_l = _mm_set_pi16(5, 6, 7, 8);
            vorder_h = _mm_set_pi16(1, 2, 3, 4);
        }

        {
            __m64 in = *(const __m64 *)buf;

            /* mask excess info out */
            if (host_is_bigendian()) {
                in = _mm_align_si64(vzero, in, n);
                in = _mm_align_si64(in, vzero, f);
            } else {
                in = _mm_align_si64(in, vzero, f);
                in = _mm_align_si64(vzero, in, n);
            }

            /* add horizontal and acc */
            vs1 = _mm_sada_pu8(vs1, in, vzero);

            /* widen bytes to words, apply order and acc */
            vs2 = _mm_mac_pu16(vs2, _mm_unpackel_pu8(in), vorder_l);
            vs2 = _mm_mac_pu16(vs2, _mm_unpackeh_pu8(in), vorder_h);
        }

        buf += SOV8;
        k -= n;

        do {
            __m64 vs1_r = vzero;

            do {
                __m64 vs2_l = vzero, vs2_h = vzero;
                unsigned int j;

                j  = k >= (257 * SOV8) ? 257 * SOV8 : k;
                j /= SOV8;
                k -= j * SOV8;
                do {
                    /* get input data */
                    __m64 in = *(const __m64 *)buf;
                    buf += SOV8;

                    /* add vs1 for this round */
                    vs1_r = _mm_add_pi32(vs1_r, vs1);

                    /* add horizontal and acc */
// TODO: how does wsad really work?
                    /*
                     * the Intel iwmmxt 1 & 2 manual says the wsad instruction
                     * always zeros the upper word (32 in the arm context),
                     * and then adds all sad into the lower word (again 32
                     * bit). If the z version is choosen, the lower word is
                     * also zeroed before, otherwise we get an acc.
                     *
                     * Visual studio only knows the sada intrinsic to reflect
                     * that, but no description, no prototype.
                     *
                     * But there is no sada intrinsic in the Intel manual.
                     * The Intel iwmmxt-1 manual only knows sad & sadz, two
                     * operands, instead the acc is done with the lvalue
                     * (which only really works with spec. compiler builtins).
                     * GCC follows the intel manual (but does gcc manages to
                     * use the lvalue?).
                     * To make matters worse the description for the _mm_sad_pu8
                     * intrinsic says it clears the upper _3_ fields, and only
                     * acc in the lowest, so only working in 16 Bit.
                     * So who is wrong?
                     *
                     * If this is different between 1 & 2 we are screwed, esp.
                     * since i can not find a preprocessor define if 1 or 2.
                     */
                    vs1 = _mm_sada_pu8(vs1, in, vzero);

                    /* widen bytes to words and acc */
                    vs2_l = _mm_add_pi16(vs2_l, _mm_unpackel_pu8(in));
                    vs2_h = _mm_add_pi16(vs2_h, _mm_unpackeh_pu8(in));
                } while (--j);
                /* shake and roll vs1_r, so both 32 bit sums get some input */
                vs1_r = _mm_shuffle_pi16(vs1_r, 0x4e);
                /* apply order and add to 32 bit */
                vs2_l = _mm_madd_pu16(vs2_l, vorder_l);
                vs2_h = _mm_madd_pu16(vs2_h, vorder_h);
                /* acc */
                vs2 = _mm_add_pi32(vs2, vs2_l);
                vs2 = _mm_add_pi32(vs2, vs2_h);
            } while (k >= SOV8);
            /* chop vs1 round sum before multiplying by 8 */
            vs1_r = vector_chop(vs1_r);
            /* add vs1 for this round (8 times) */
            vs2 = _mm_add_pi32(vs2, _mm_sll_pi32(vs1_r, three));
            /* chop both sums to something within 16 bit */
            vs2 = vector_chop(vs2);
            vs1 = vector_chop(vs1);
            len += k;
            k = len < VNMAX ? len : VNMAX;
            len -= k;
        } while (likely(k >= SOV8));
        len += k;
        vs1 = _mm_add_pi32(vs1, _mm_shuffle_pi16(vs1, 0x4e));
        vs2 = _mm_add_pi32(vs2, _mm_shuffle_pi16(vs2, 0x4e));
        s1 = _mm_extract_pu32(vs1, 0);
        s2 = _mm_extract_pu32(vs2, 0);
    }

    if (unlikely(len)) do {
        s1 += *buf++;
        s2 += s1;
    } while (--len);
    /* at this point we should have not so big s1 & s2 */
    MOD28(s1);
    MOD28(s2);

    return (s2 << 16) | s1;
}

/* inline asm, so only on GCC (or compatible) && ARM v6 or better */
#elif 0 && defined(__GNUC__) && ( \
        defined(__thumb2__)  && ( \
            !defined(__ARM_ARCH_7__) && !defined(__ARM_ARCH_7M__) \
        ) || ( \
        !defined(__thumb__) && ( \
            defined(__ARM_ARCH_6__)   || defined(__ARM_ARCH_6J__)  || \
            defined(__ARM_ARCH_6T2__) || defined(__ARM_ARCH_6ZK__) || \
            defined(__ARM_ARCH_7A__)  || defined(__ARM_ARCH_7R__) \
        )) \
    )
/* This code is disabled, since it is not faster, only for reference.
 * We are at speedup: 0.952830
 * Again counting instructions is futile, 5 instructions per 4 bytes
 * against at least 3 per byte (loop overhead excluded) is no win.
 * And split sums also does not save us.
 */
#  define SOU32 (sizeof(unsigned int))
#  define HAVE_ADLER32_VEC
#  define MIN_WORK 16
// TODO: maybe 2*NMAX is possible, but that's very thin
/* this way we are at 0xda */
#  define VNMAX (NMAX+((NMAX*9)/10))

/* ========================================================================= */
local noinline uLong adler32_vec(adler, buf, len)
    uLong adler;
    const Bytef *buf;
    uInt len;
{
    unsigned int s1, s2;
    unsigned int k;

    s1 = adler & 0xffff;
    s2 = (adler >> 16) & 0xffff;

    k    = ALIGN_DIFF(buf, SOU32);
    len -= k;
    if (k) do {
        s1 += *buf++;
        s2 += s1;
    } while (--k);

    if (likely(len >= 4 * SOU32)) {
        unsigned int vs1 = s1, vs2 = s2;
        unsigned int order_lo, order_hi;

        if (host_is_bigendian()) {
            order_lo = 0x00030001;
            order_hi = 0x00040002;
        } else {
            order_lo = 0x00020004;
            order_hi = 0x00010003;
        }
        k = len < VNMAX ? len : VNMAX;
        len -= k;

        do {
            unsigned int vs1_r = 0;
            do {
                unsigned int j;
                unsigned int vs2_lo = 0, vs2_hi = 0;

                j  = (k/4) >= 128 ? 128 : (k/4);
                k -= j * 4;
                do {
                    /* get input data */
                    unsigned int in = *(const unsigned int *)buf;
                    buf += SOU32;
                    /* add vs1 for this round */
                    vs1_r += vs1;
                    /* add horizontal and acc */
                    asm ("usada8 %0, %1, %2, %3" : "=r" (vs1) : "r" (in), "r" (0), "r" (vs1));
                    /* widen bytes to words and acc */
                    asm ("uxtab16 %0, %1, %2" : "=r" (vs2_lo) : "r" (vs2_lo), "r" (in));
                    asm ("uxtab16 %0, %1, %2, ror #8" : "=r" (vs2_hi) : "r" (vs2_hi), "r" (in));
                } while (--j);
                /* aply order and acc */
                asm ("smlad %0, %1, %2, %3" : "=r" (vs2) : "r" (vs2_lo) , "r" (order_lo), "r" (vs2));
                asm ("smlad %0, %1, %2, %3" : "=r" (vs2) : "r" (vs2_hi) , "r" (order_hi), "r" (vs2));
            } while (k >= SOU32);
            /* chop vs1 round sum before multiplying by 4 */
            CHOP(vs1_r);
            /* add vs1 for this round (4 times) */
            vs2 += vs1_r * 4;
            /* chop both sums */
            CHOP(vs2);
            CHOP(vs1);
            len += k;
            k = len < VNMAX ? len : VNMAX;
            len -= k;
        } while (likely(k >= SOU32));
        len += k;
        s1 = vs1;
        s2 = vs2;
    }

    if (unlikely(len)) do {
        s1 += *buf++;
        s2 += s1;
    } while (--len);
    /* at this point we should not have so big s1 & s2 */
    MOD28(s1);
    MOD28(s2);

    return (s2 << 16) | s1;
}
#endif
