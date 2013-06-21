#include <arm_neon.h>

/* NEON SIMD 16bit to 32bit BLT */
void aBlt32_neon(int n, dword * dst, const word * src, byte clset) {
  int i;
  
  /* use non simd */
  if (n < 8) {
    if (clset) {
      for (i = 0; i < n; i++) {
        dst[i] = (ag_r(src[i]) << colorspace_positions[0]) |
                 (ag_g(src[i]) << colorspace_positions[1]) |
                 (ag_b(src[i]) << colorspace_positions[2]) ;
      }
    }
    else {
      dst[i] = (ag_r(src[i]) << 16) |
               (ag_g(src[i]) << 8) |
               ag_b(src[i]);
    }
    
    return;
  }
  
  int rps, gps, bps;
  
  if (clset) {
    rps = colorspace_positions[0] >> 3;
    gps = colorspace_positions[1] >> 3;
    bps = colorspace_positions[2] >> 3;
  }
  else {
    rps = 2;
    gps = 1;
    bps = 0;
  }
  
  uint8x8_t mask5, mask6, alp;
  mask5 = vmov_n_u8(0xf8); /* 5 mask - red */
  mask6 = vmov_n_u8(0xfc); /* 6 mask - green */
  alp   = vmov_n_u8(0xff); /* Alpha constant */
  /* Change Types */
  uint16_t * p565 = (uint16_t *) src;
  uint8_t * p888  = (uint8_t *) dst;
  int nn = n / 8, left = n % 8;
  
  for (i = 0; i < nn; i++) { /* Loop per 8 pixels */
    uint8x8_t red, grn, blu;
    uint16x8_t pix;
    uint8x8x4_t rgb;
    pix = vld1q_u16(p565 + 8 * i); /* load 8 pixel */
    /* right shift */
    red = vshrn_n_u16(pix, 8);
    grn = vshrn_n_u16(pix, 3);
    blu = vmovn_u16(pix);
    /* and mask */
    red = vand_u8(red, mask5);
    grn = vand_u8(grn, mask6);
    blu = vshl_n_u8(blu, 3);
    /* dump */
    rgb.val[rps]  = red;
    rgb.val[gps]  = grn;
    rgb.val[bps]  = blu;
    vst4_u8(p888 + 32 * i, rgb);
    
    /* leftover */
    if ((i + 1 == nn) && (left > 0)) {
      p565  = ((uint16_t *) src) - (8 - left);
      p888  = ((uint8_t *)  dst) - ((8 - left) * 4);
      nn++;
      left = 0;
    }
  }
}


/* Set Color Buffer */
void aMemcpyColorPos_neon(dword * dst, dword * src, int n, byte pos_src) {
  int i;
  
  /* use non simd */
  if (n < 8) {
    for (i = 0; i < n; i++) {
      dword cl = src[i];
      
      if (pos_src) {
        dst[i] = ag_rgb32(
                   (byte) ((cl >> colorspace_positions[0]) & 0xff),
                   (byte) ((cl >> colorspace_positions[1]) & 0xff),
                   (byte) ((cl >> colorspace_positions[2]) & 0xff)
                 );
      }
      else {
        dst[i] = (
                   ((ag_r32(cl) & 0xff) << colorspace_positions[0]) |
                   ((ag_g32(cl) & 0xff) << colorspace_positions[1]) |
                   ((ag_b32(cl) & 0xff) << colorspace_positions[2])
                 );
      }
    }
    
    return;
  }
  
  int rps = colorspace_positions[0] >> 3;
  int gps = colorspace_positions[1] >> 3;
  int bps = colorspace_positions[2] >> 3;
  /* Change Types */
  uint8_t * u_dst   = (uint8_t *) dst;
  uint8_t * u_src   = (uint8_t *) src;
  /* Loop Variables */
  int nn = n / 8, left = n % 8;
  
  for (i = 0; i < nn; i++) {
    /* Move Layers Data */
    uint8x8x4_t n_src = vld4_u8(u_src + (32 * i));
    /* Dump result into output buffer */
    uint8x8x4_t n_dst;
    
    if (pos_src) {
      n_dst.val[2] = n_src.val[rps];
      n_dst.val[1] = n_src.val[gps];
      n_dst.val[0] = n_src.val[bps];
    }
    else {
      n_dst.val[rps] = n_src.val[2];
      n_dst.val[gps] = n_src.val[1];
      n_dst.val[bps] = n_src.val[0];
    }
    
    vst4_u8(u_dst + 32 * i, n_dst);
    
    /* leftover */
    if ((i + 1 == nn) && (left > 0)) {
      for (i = n - left; i < n; i++) {
        dword cl = src[i];
        
        if (pos_src) {
          dst[i] = ag_rgb32(
                     (byte) ((cl >> colorspace_positions[0]) & 0xff),
                     (byte) ((cl >> colorspace_positions[1]) & 0xff),
                     (byte) ((cl >> colorspace_positions[2]) & 0xff)
                   );
        }
        else {
          dst[i] = (
                     ((ag_r32(cl) & 0xff) << colorspace_positions[0]) |
                     ((ag_g32(cl) & 0xff) << colorspace_positions[1]) |
                     ((ag_b32(cl) & 0xff) << colorspace_positions[2])
                   );
        }
      }
      
      return;
    }
  }
}