/*
 * Copyright (C) 2011 Ahmad Amarullah ( http://amarullz.com/ )
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Descriptions:
 * -------------
 * AROMA FILEMANAGER PNG & PNG Font Handler
 *
 */
#include <png.h>
#include "../aroma.h"

/*********************************[ STRUCTRES ]********************************/
//-- READER STRUCTURE
typedef struct  {
  byte * data;
  int pos;
  int len;
} APNG_DATA;

/*********************************[ FUNCTIONS ]********************************/
byte apng_stretch_(
  CANVAS * _b,
  PNGCANVAS * p,
  int dx,
  int dy,
  int dw,
  int dh,

  int sx,
  int sy,
  int sw,
  int sh
);

//-- READER FUNCTION
void apng_readfn(png_structp pngPtr, png_bytep data, png_size_t length) {
  png_voidp a       = png_get_io_ptr(pngPtr);
  APNG_DATA * cpng  = (APNG_DATA *) a;
  
  if (cpng->pos + length >= cpng->len) {
    length = cpng->len - cpng->pos;
  }
  
  if (length) {
    memcpy(data, cpng->data + cpng->pos, length);
    cpng->pos += length;
  }
}

//-- CLOSE
void apng_close(PNGCANVAS * pngcanvas) {
  if (pngcanvas->r != NULL) {
    free(pngcanvas->r);
  }
  
  if (pngcanvas->g != NULL) {
    free(pngcanvas->g);
  }
  
  if (pngcanvas->b != NULL) {
    free(pngcanvas->b);
  }
  
  if (pngcanvas->a != NULL) {
    free(pngcanvas->a);
  }
  
  pngcanvas->r = NULL;
  pngcanvas->g = NULL;
  pngcanvas->b = NULL;
  pngcanvas->a = NULL;
}

//-- LOAD PNG FROM ZIP
byte apng_load(PNGCANVAS * pngcanvas, char * imgname) {
  char zpath[256];
  
  if (imgname[0] == '@') {
    char * icotheme_name = imgname;
    icotheme_name++;
    
    if (strcmp(acfg()->themename, "") == 0) {
      snprintf(zpath, 255, "%s/icons/%s.png", AROMA_DIR, icotheme_name);
    }
    else {
      snprintf(zpath, 255, "themes/%s/icon.%s", acfg()->themename, icotheme_name);
      
      if (apng_load(pngcanvas, zpath)) {
        return 1;
      }
      
      snprintf(zpath, 255, "%s/icons/%s.png", AROMA_DIR, icotheme_name);
    }
  }
  else {
    snprintf(zpath, 255, "%s/%s.png", AROMA_DIR, imgname);
  }
  
  memset(pngcanvas, 0, sizeof(PNGCANVAS));
  png_structp png_ptr   = NULL;
  png_infop info_ptr    = NULL;
  byte result           = 0;
  byte header[8];
  //-- LOAD DATA FROM ZIP
  AZMEM data_png;
  
  if (!az_readmem(&data_png, zpath, 1)) {
    return 0;
  }
  
  //-- CREATE PNG ARGUMENT
  APNG_DATA          apng_data;
  apng_data.data   = data_png.data;
  apng_data.pos    = 0;
  apng_data.len    = data_png.sz;
  //-- HEADER
  memcpy(header, apng_data.data, sizeof(header));
  apng_data.pos += sizeof(header);
  
  //-- COMPARE
  if (png_sig_cmp(header, 0, sizeof(header))) {
    goto exit;
  }
  
  //-- CREATE READ STRUCTURE
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  
  if (!png_ptr) {
    goto exit;
  }
  
  //-- CREATE INFO STRUCTURE
  info_ptr = png_create_info_struct(png_ptr);
  
  if (!info_ptr) {
    goto exit;
  }
  
  //-- JMP
  if (setjmp(png_jmpbuf(png_ptr))) {
    goto exit;
  }
  
  //-- SET FUNCTION
  png_set_read_fn(png_ptr, &apng_data, apng_readfn);
  png_set_sig_bytes(png_ptr, sizeof(header));
  png_read_info(png_ptr, info_ptr);
  
  //-- Check Color Mode
  if (!(
        (info_ptr->bit_depth == 8 &&
         (
           (info_ptr->channels == 3 && info_ptr->color_type == PNG_COLOR_TYPE_RGB) ||
           (info_ptr->channels == 4 && info_ptr->color_type == PNG_COLOR_TYPE_RGBA)
         )
        ) || (info_ptr->channels == 1 && info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
      )) {
    LOGE("PNG(%s): Not Supported. Only 8 Bit Depth with 3/4 Channel or Pallete.\n", zpath);
    goto exit;
  }
  
  LOGI("PNG(%s): %ix%ix%i\n", zpath, info_ptr->width, info_ptr->height, info_ptr->channels);
  
  if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE) {
    png_set_palette_to_rgb(png_ptr);
    png_read_update_info(png_ptr, info_ptr);
  }
  
  //-- Initializing Canvas
  pngcanvas->w    = info_ptr->width;
  pngcanvas->h    = info_ptr->height;
  pngcanvas->c    = info_ptr->channels;
  pngcanvas->s    = pngcanvas->w * pngcanvas->h;
  pngcanvas->r    = malloc(pngcanvas->s);
  pngcanvas->g    = malloc(pngcanvas->s);
  pngcanvas->b    = malloc(pngcanvas->s);
  
  if (pngcanvas->c == 4) {
    pngcanvas->a = malloc(pngcanvas->s);
  }
  else {
    pngcanvas->a = NULL;
  }
  
  //-- READ ROWS
  int row_sz          = (int) png_get_rowbytes(png_ptr, info_ptr);
  png_bytep row_data  = (png_bytep) malloc(row_sz);
  int y;
  
  for (y = 0; y < pngcanvas->h; ++y) {
    int x;
    png_read_row(png_ptr, row_data, NULL);
    
    for (x = pngcanvas->w - 1; x >= 0; x--) {
      //-- Get Row Data
      int sx = x * pngcanvas->c;
      int dx = y * pngcanvas->w + x;
      //-- SAVE RGB CHANNELS
      pngcanvas->r[dx] = row_data[sx];
      pngcanvas->g[dx] = row_data[sx + 1];
      pngcanvas->b[dx] = row_data[sx + 2];
      
      //-- SAVE ALPHA CHANNEL
      if (pngcanvas->c == 4) {
        pngcanvas->a[dx] = row_data[sx + 3];
      }
    }
  }
  
  free(row_data);
  result = 1;
exit:
  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
  free(data_png.data);
  return result;
}

//-- DRAW PNG INTO CANVAS
byte apng_draw(CANVAS * _b, PNGCANVAS * p, int xpos, int ypos) {
  if (p == NULL) {
    return 0;
  }
  
  return apng_draw_ex(_b, p, xpos, ypos, 0, 0, p->w, p->h);
}
byte apng_draw_ex(CANVAS * _b, PNGCANVAS * p, int xpos, int ypos, int sxpos, int sypos, int sw, int sh) {
  if (_b == NULL) {
    _b = agc();
  }
  
  if (p == NULL) {
    return 0;
  }
  
  if (p->s == 0) {
    return 0;
  }
  
  //-- Quantizer Error Dithering Data Termporary
  int    qz  = sw * 6; // p->s * 3;
  byte * qe  = malloc(qz);
  memset(qe, 0, qz);
  //-- Drawing
  int x, y;
  
  for (y = sypos; (y < sypos + sh) && (y < p->h) && ((y - sypos) + ypos < _b->h); y++) {
    int qy = (y - sypos) % 2;
    int qp = qy ? 0 : 1;
    memset(qe + (qp * sw * 3), 0, sw * 3);
    
    for (x = sxpos; (x < sxpos + sw) && (x < p->w) && ((x - sxpos) + xpos < _b->w); x++) {
      int sx = y * p->w + x;
      int qx = ((qy * sw) + (x - sxpos)) * 3;
      byte dr, dg, db;
      
      //-- Get Destination Color
      if (p->c == 3) {
        //-- NO ALPHA CHANNEL
        dr = p->r[sx];
        dg = p->g[sx];
        db = p->b[sx];
      }
      else {
        //-- Leave The Transparent
        if (p->a[sx] == 0) {
          continue;
        }
        
        if (p->a[sx] == 255) {
          dr = p->r[sx];
          dg = p->g[sx];
          db = p->b[sx];
        }
        else {
          //-- WITH ALPHA CHANNEL
          color * dstp = agxy(_b, (x - sxpos) + xpos, (y - sypos) + ypos);
          
          if (dstp == NULL) {
            continue;
          }
          
          color dcolor = dstp[0]; //-- Destination Color
          byte  ralpha = 255 - p->a[sx];
          dr = (byte) (((((int) ag_r(dcolor)) * ralpha) + (((int) p->r[sx]) * p->a[sx])) >> 8);
          dg = (byte) (((((int) ag_g(dcolor)) * ralpha) + (((int) p->g[sx]) * p->a[sx])) >> 8);
          db = (byte) (((((int) ag_b(dcolor)) * ralpha) + (((int) p->b[sx]) * p->a[sx])) >> 8);
        }
      }
      
      //-- Dithering
      /*
      byte old_r = (byte) min(((int) dr) + ((int) qe[qx]),  255);
      byte old_g = (byte) min(((int) dg) + ((int) qe[qx+1]),255);
      byte old_b = (byte) min(((int) db) + ((int) qe[qx+2]),255);
      byte new_r = ag_close_r(old_r);
      byte new_g = ag_close_g(old_g);
      byte new_b = ag_close_b(old_b);
      byte err_r = old_r - new_r;
      byte err_g = old_g - new_g;
      byte err_b = old_b - new_b;
      ag_dither(qe,qp,qx,x-sxpos,y-sypos,sw,sh,err_r,err_g,err_b);
      */
      /*byte dither_xy= (dither_y * 8) + (xx % 8);
      byte new_r = ag_close_r(min(ag_r32(curpix) + dither_tresshold_r[dither_xy], 255));
      byte new_g = ag_close_g(min(ag_g32(curpix) + dither_tresshold_g[dither_xy], 255));
      byte new_b = ag_close_b(min(ag_b32(curpix) + dither_tresshold_b[dither_xy], 255));
      
       ag_setpixel(_b,(x-sxpos)+xpos,(y-sypos)+ypos, ag_rgb(new_r,new_g,new_b));
       */
      ag_setpixel(_b, (x - sxpos) + xpos, (y - sypos) + ypos, ag_dodither_rgb(x, y, dr, dg, db));
    }
  }
  
  //printf("PNGDRAW: %ix%i on %ix%i\n",p->w,p->h,xpos,ypos);
  //LOGI("PNGDRAW: %ix%i on %ix%i\n",p->w,p->h,xpos,ypos);
  free(qe);
  return 1;
}

//-- LOAD PNG FONTS FROM ZIP
byte apng_loadfont(PNGFONTS * pngfont, const char * imgname) {
  png_structp png_ptr   = NULL;
  png_infop info_ptr    = NULL;
  byte result           = 0;
  byte header[8];
  //-- LOAD DATA FROM ZIP
  char zpath[256];
  snprintf(zpath, sizeof(zpath) - 1, "%s/%s.png", AROMA_DIR, imgname);
  AZMEM data_png;
  printf("Loading PNG : %s\n", zpath);
  
  if (!az_readmem(&data_png, zpath, 1)) {
    return 0;
  }
  
  printf("Loading PNG : %s OK\n", zpath);
  //-- CREATE PNG ARGUMENT
  APNG_DATA          apng_data;
  apng_data.data   = data_png.data;
  apng_data.pos    = 0;
  apng_data.len    = data_png.sz;
  //-- HEADER
  memcpy(header, apng_data.data, sizeof(header));
  apng_data.pos += sizeof(header);
  
  //-- COMPARE
  if (png_sig_cmp(header, 0, sizeof(header))) {
    goto exit;
  }
  
  //-- CREATE READ STRUCTURE
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  
  if (!png_ptr) {
    goto exit;
  }
  
  //-- CREATE INFO STRUCTURE
  info_ptr = png_create_info_struct(png_ptr);
  
  if (!info_ptr) {
    goto exit;
  }
  
  //-- JMP
  if (setjmp(png_jmpbuf(png_ptr))) {
    goto exit;
  }
  
  //-- SET FUNCTION
  png_set_read_fn(png_ptr, &apng_data, apng_readfn);
  png_set_sig_bytes(png_ptr, sizeof(header));
  png_read_info(png_ptr, info_ptr);
  
  //-- Check Color Mode
  if (!(info_ptr->bit_depth == 8 && (info_ptr->channels == 4 && info_ptr->color_type == PNG_COLOR_TYPE_RGBA))) {
    LOGE("PNG FONT(%s): Not Supported. Only 8 Bit Depth with 4 Channel.\n", zpath);
    goto exit;
  }
  
  LOGI("PNG(%s): %ix%ix%i\n", zpath, info_ptr->width, info_ptr->height, info_ptr->channels);
  //-- Initializing Canvas
  pngfont->w    = info_ptr->width;
  pngfont->h    = info_ptr->height;
  pngfont->c    = info_ptr->channels;
  pngfont->fh   = pngfont->h - 1;
  pngfont->s    = pngfont->w * pngfont->fh;
  pngfont->d    = malloc(pngfont->s);
  //-- READ ROWS
  int row_sz          = (int) png_get_rowbytes(png_ptr, info_ptr);
  png_bytep row_data  = (png_bytep) malloc(row_sz);
  int y;
  int f_x   = 0;
  int f_w   = 0;
  int f_p   = 0;
  
  for (y = 0; y < pngfont->h; ++y) {
    int x;
    png_read_row(png_ptr, row_data, NULL);
    
    for (x = 0; x < pngfont->w; x++) {
      //-- Get Row Data
      int sx        = x * pngfont->c;
      byte alphaval = row_data[sx + 3];
      
      if (y == 0) {
        if (alphaval == 255) {
          if (f_p < 96) {
            pngfont->fx[f_p] = x;
            pngfont->fw[f_p] = min(pngfont->w - x, 255);
            
            if (f_p > 0) {
              pngfont->fw[f_p - 1] = min(x - pngfont->fx[f_p - 1], 255);
              //LOGI("Font(%i): x = %i - w = %i\n",f_p-1,pngfont->fx[f_p-1],pngfont->fw[f_p-1]);
            }
            
            f_p++;
          }
        }
      }
      else {
        int dx        = (y - 1) * pngfont->w + x;
        pngfont->d[dx] = alphaval;
      }
    }
  }
  
  free(row_data);
  result = 1;
  pngfont->loaded = 1;
exit:
  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
  free(data_png.data);
  return result;
}
//-- CLOSE
void apng_closefont(PNGFONTS * p) {
  if (p->d != NULL) {
    free(p->d);
  }
  
  p->d = NULL;
  p->loaded = 0;
}

//-- DRAW PNG FONT INTO CANVAS
byte apng_drawfont(CANVAS * _b, PNGFONTS * p, byte fpos, int xpos, int ypos, color cl, byte underline, byte bold) {
  if (_b == NULL) {
    _b = agc();
  }
  
  if (p == NULL) {
    return 0;
  }
  
  if (p->loaded == 0) {
    return 0;
  }
  
  if (fpos > 95) {
    return 0;
  }
  
  //-- Quantizer Error Dithering Data Termporary
  byte   fw  = p->fw[fpos];
  int    fx  = p->fx[fpos];
  int    fh  = p->fh;
  int    qz  = (fh * fw) * 3;
  byte * qe  = malloc(qz);
  memset(qe, 0, qz);
  //-- Drawing
  int x, y;
  
  for (y = 0; (y < p->fh) && (y + ypos < _b->h); y++) {
    for (x = 0; (x < fw) && (x + xpos < _b->w); x++) {
      int sx = y * p->w + x + fx;
      int qx = (y * fw + x) * 3;
      byte a = p->d[sx];
      //-- Save Colors
      byte dr = ag_r(cl);
      byte dg = ag_g(cl);
      byte db = ag_b(cl);
      
      //-- Get Font Alpha
      if (a == 0) {
        //continue;
        color * dstp = agxy(_b, x + xpos, y + ypos);
        
        if (dstp == NULL) {
          continue;
        }
        
        color dcolor = dstp[0]; //-- Destination Color
        dr = ag_r(dcolor);
        dg = ag_g(dcolor);
        db = ag_b(dcolor);
      }
      else if (a != 255) {
        //-- WITH ALPHA CHANNEL
        color * dstp = agxy(_b, x + xpos, y + ypos);
        
        if (dstp == NULL) {
          continue;
        }
        
        color dcolor = dstp[0]; //-- Destination Color
        byte  ralpha = 255 - a;
        dr = (byte) (((((int) ag_r(dcolor)) * ralpha) + (((int) dr) * a)) >> 8);
        dg = (byte) (((((int) ag_g(dcolor)) * ralpha) + (((int) dg) * a)) >> 8);
        db = (byte) (((((int) ag_b(dcolor)) * ralpha) + (((int) db) * a)) >> 8);
      }
      
      //-- Dithering
      /*
      byte old_r = (byte) min(((int) dr) + ((int) qe[qx]),  255);
      byte old_g = (byte) min(((int) dg) + ((int) qe[qx+1]),255);
      byte old_b = (byte) min(((int) db) + ((int) qe[qx+2]),255);
      byte new_r = ag_close_r(old_r);
      byte new_g = ag_close_g(old_g);
      byte new_b = ag_close_b(old_b);
      byte err_r = old_r - new_r;
      byte err_g = old_g - new_g;
      byte err_b = old_b - new_b;
      
      ag_dither(qe,y+1,qx,x,y,fw,fh,err_r,err_g,err_b);
      ag_setpixel(_b,x+xpos,y+ypos,ag_rgb(new_r,new_g,new_b));
      */
      ag_setpixel(_b, x + xpos, y + ypos, ag_dodither_rgb(x, y, dr, dg, db));
      
      if (bold) {
        int bx    = x + xpos;
        int by    = y + ypos;
        ag_subpixel(_b, bx - 1, by - 1, cl, a / 4);
        ag_subpixel(_b, bx,  by - 1, cl, a / 2);
        ag_subpixel(_b, bx + 1, by - 1, cl, a / 4);
        ag_subpixel(_b, bx - 1, by, cl, a / 2);
        ag_subpixel(_b, bx, by, cl, a);
      }
      
      if (underline) {
        if (y == (p->fh - 1)) {
          ag_setpixel(_b, x + xpos, y + ypos, cl);
        }
      }
    }
  }
  
  free(qe);
  return 1;
}

//-- .9.png calculating
byte apng9_calc(PNGCANVAS * p, APNG9P v, byte with_pad) {
  if (p == NULL) {
    return 0;
  }
  
  if (p->s == 0) {
    return 0;
  }
  
  int ts = p->w;  //-- Top Start
  int te = 0;     //-- Top End
  int ls = p->h;  //-- Left Start
  int le = 0;     //-- Left End
  int bs = p->w;  //-- Bottom Start
  int be = 0;     //-- Bottom End
  int rs = p->h;  //-- Right Start
  int re = 0;     //-- Right End
  int x, y;
  int bottompos = (p->h - 1) * p->w;
  
  //-- Get Horizontal Strecth
  for (x = 1; x < p->w; x++) {
    if (p->a[x] == 255) {
      if (x < ts) {
        ts = x;
      }
      
      if (x > te) {
        te = x;
      }
    }
    
    if (with_pad) {
      if (p->a[x + bottompos] == 255) {
        if (x < bs) {
          bs = x;
        }
        
        if (x > be) {
          be = x;
        }
      }
    }
  }
  
  for (y = 1; y < p->h; y++) {
    int ypos = (y * p->w);
    
    if (p->a[ypos] == 255) {
      if (y < ls) {
        ls = y;
      }
      
      if (y > le) {
        le = y;
      }
    }
    
    if (with_pad) {
      if (p->a[ypos + (p->w - 1)] == 255) {
        if (y < rs) {
          rs = y;
        }
        
        if (y > re) {
          re = y;
        }
      }
    }
  }
  
  le -= ls - 1;
  te -= ts - 1;
  
  if (with_pad) {
    re -= rs - 1;
    be -= bs - 1;
  }
  
  v->x = ts;
  v->y = ls;
  v->w = te;
  v->h = le;
  
  if (with_pad) {
    v->t = rs;
    v->b = (p->h - 1) - ((rs + re));
    v->l = bs;
    v->r = (p->w - 1) - ((bs + be));
  }
  else {
    v->t = v->b = v->l = v->r = 0;
  }
  
  return 1;
}

byte apng9_draw(
  CANVAS * _b,
  PNGCANVAS * p,
  int dx,
  int dy,
  int dw,
  int dh,
  APNG9P v,
  byte with_pad
) {
  if (_b == NULL) {
    _b = agc();
  }
  
  if (p == NULL) {
    return 0;
  }
  
  if (p->s == 0) {
    return 0;
  }
  
  if ((dh < 3) || (dw < 3)) {
    return 1;
  }
  
  APNG9 tmpv;
  
  if (v == NULL) {
    v = &tmpv;
  }
  
  apng9_calc(p, v, with_pad);
  int minW  = floor((dw - 2) / 2);
  int minH  = floor((dh - 2) / 2);
  int rx = v->x + v->w;
  int ry = v->y + v->h;
  int lw = v->x - 1;
  int lh = v->y - 1;
  int rw = (p->w - (with_pad ? 1 : 0)) - rx;
  int rh = (p->h - (with_pad ? 1 : 0)) - ry;
  int dlw = min(lw, minW);
  int dlh = min(lh, minH);
  int drw = min(rw, minW);
  int drh = min(rh, minH);
  //-- Top Left
  apng_stretch(
    _b, p, dx, dy, dlw, dlh, 1, 1, lw, lh
  );
  //-- Top Right
  apng_stretch(
    _b, p, (dx + dw) - drw, dy, drw, dlh, rx, 1, rw, lh
  );
  //-- Bottom Left
  apng_stretch(
    _b, p, dx, (dy + dh) - drh, dlw, drh, 1, ry, lw, rh
  );
  //-- Bottom Right
  apng_stretch(
    _b, p, (dx + dw) - drw, (dy + dh) - drh, drw, drh, rx, ry, rw, rh
  );
  //-- Top
  apng_stretch(_b, p,
               dx + dlw,        dy,
               dw - (dlw + drw),   dlh,
               v->x,         1,
               v->w,         lh
              );
  //-- left
  apng_stretch(_b, p,
               dx,           dy + dlh,
               dlw,           dh - (dlh + drh),
               1,            v->y,
               lw,           v->h
              );
  //-- Bottom
  apng_stretch(_b, p,
               dx + dlw,        (dy + dh) - drh,
               dw - (dlw + drw),   drh,
               v->x,         v->y + v->h,
               v->w,         rh
              );
  //-- Right
  apng_stretch(_b, p,
               (dx + dw) - drw,   dy + dlh,
               drw,           dh - (dlh + drh),
               v->x + v->w,    v->y,
               rw,           v->h
              );
  //-- Center
  apng_stretch(_b, p,
               dx + dlw,        dy + dlh,
               dw - (dlw + drw),   dh - (dlh + drh),
               v->x,         v->y,
               v->w,         v->h
              );
  return 1;
}

byte apng_stretch(CANVAS * _b, PNGCANVAS * p,
                  int dx,
                  int dy,
                  int wDst,
                  int hDst,

                  int sx,
                  int sy,
                  int wSrc,
                  int hSrc) {
  if (_b == NULL) {
    _b = agc();
  }
  
  if (p == NULL) {
    return 0;
  }
  
  if (p->s == 0) {
    return 0;
  }
  
  if ((hDst < 1) || (wDst < 1) || (hSrc < 1) || (wSrc < 1)) {
    return 0;
  }
  
  if ((hDst < 2) || (wDst < 2) || (hSrc < 2) || (wSrc < 2)) {
    return apng_stretch_(_b, p, dx, dy, wDst, hDst, sx, sy, wSrc, hSrc);
  }
  
  unsigned int wStepFixed16b, hStepFixed16b, wCoef, hCoef, x, y;
  unsigned int hc1, hc2, wc1, wc2, offsetX, offsetY;
  int id1, id2, id3, id4, line1, line2;
  byte dr, dg, db, da;
  wStepFixed16b = ((wSrc - 1) << 16) / (wDst - 1);
  hStepFixed16b = ((hSrc - 1) << 16) / (hDst - 1);
  hCoef = 0;
  
  for (y = 0 ; y < hDst ; y++) {
    offsetY = (hCoef >> 16);
    hc2 = (hCoef >> 9) & 127;
    hc1 = 128 - hc2;
    wCoef = 0;
    line1 = (offsetY + sy) * p->w;
    line2 = (offsetY + sy + 1) * p->w;
    
    for (x = 0 ; x < wDst ; x++) {
      color * dstp = agxy(_b, dx + x, dy + y);
      
      if (dstp != NULL) {
        offsetX = (wCoef >> 16);
        wc2 = (wCoef >> 9) & 127;
        wc1 = 128 - wc2;
        id1 = line1 + offsetX + sx;
        id2 = line2 + offsetX + sx;
        id3 = line1 + offsetX + sx + 1;
        id4 = line2 + offsetX + sx + 1;
        
        if (id2 < p->s) {
          dr = ((p->r[id1] * hc1 + p->r[id2] * hc2) * wc1 +
                (p->r[id3] * hc1 + p->r[id4] * hc2) * wc2) >> 14;
          dg = ((p->g[id1] * hc1 + p->g[id2] * hc2) * wc1 +
                (p->g[id3] * hc1 + p->g[id4] * hc2) * wc2) >> 14;
          db = ((p->b[id1] * hc1 + p->b[id2] * hc2) * wc1 +
                (p->b[id3] * hc1 + p->b[id4] * hc2) * wc2) >> 14;
                
          if (p->c == 4) {
            da = ((p->a[id1] * hc1 + p->a[id2] * hc2) * wc1 +
                  (p->a[id3] * hc1 + p->a[id4] * hc2) * wc2) >> 14;
            color dcolor = dstp[0];
            byte  falpha = da;
            byte  ralpha = 255 - falpha;
            dr = (byte) (((((int) ag_r(dcolor)) * ralpha) + (((int) dr) * falpha)) >> 8);
            dg = (byte) (((((int) ag_g(dcolor)) * ralpha) + (((int) dg) * falpha)) >> 8);
            db = (byte) (((((int) ag_b(dcolor)) * ralpha) + (((int) db) * falpha)) >> 8);
          }
          
          ag_setpixel(_b, dx + x, dy + y,
                      ag_dodither_rgb(x, y, dr, dg, db)
                     );
        }
      }
      
      wCoef += wStepFixed16b;
    }
    
    hCoef += hStepFixed16b;
  }
  
  return 1;
}


//-- STRETCH
byte apng_stretch_(
  CANVAS * _b,
  PNGCANVAS * p,
  int dx,
  int dy,
  int dw,
  int dh,

  int sx,
  int sy,
  int sw,
  int sh
) {
  if (_b == NULL) {
    _b = agc();
  }
  
  if (p == NULL) {
    return 0;
  }
  
  if (p->s == 0) {
    return 0;
  }
  
  if ((dh < 1) || (dw < 1) || (sh < 1) || (sw < 1)) {
    return 0;
  }
  
  //-- Quantizer Error Dithering Data Termporary
  int    ds  = dw * dh;
  int    qz  = dw * 6;
  byte * qe  = malloc(qz);
  memset(qe, 0, qz);
  //-- Different Scale
  float xscale = ((float) sw) / ((float) dw);
  float yscale = ((float) sh) / ((float) dh);
  int x, y;
  
  for (y = 0; y < dh; y++) {
    int qp = (y % 2);
    int qn = qp ? 0 : 1;
    memset(qe + (qn * dw * 3), 0, dw * 3);
    
    for (x = 0; x < dw; x++) {
      int xpos = round(x * xscale);
      int ypos = round(y * yscale);
      
      if ((xpos + sx < p->w) && (ypos + sy < p->h) && (x + dx < _b->w) && (y + dy < _b->h)) {
        byte  dr, dg, db;
        int   spos = ((ypos + sy) * p->w) + (xpos + sx);
        int   dpx  = x + dx;
        int   dpy  = y + dy;
        //int   sx = y * p->w + x;
        // int   qx = ((y * dw) + x) * 3;
        int   qx = (qp * dw + x) * 3;
        
        //-- Get Destination Color
        if (p->c == 3) {
          //-- NO ALPHA CHANNEL
          dr = p->r[spos];
          dg = p->g[spos];
          db = p->b[spos];
        }
        else {
          //-- Leave The Transparent
          if (p->a[spos] == 0) {
            continue;
          }
          
          if (p->a[spos] == 255) {
            dr = p->r[spos];
            dg = p->g[spos];
            db = p->b[spos];
          }
          else {
            //-- WITH ALPHA CHANNEL
            color * dstp = agxy(_b, dpx, dpy);
            
            if (dstp == NULL) {
              continue;
            }
            
            //-- Destination Color
            color dcolor = dstp[0];
            byte  falpha = p->a[spos];
            byte  ralpha = 255 - falpha;
            dr = (byte) (((((int) ag_r(dcolor)) * ralpha) + (((int) p->r[spos]) * falpha)) >> 8);
            dg = (byte) (((((int) ag_g(dcolor)) * ralpha) + (((int) p->g[spos]) * falpha)) >> 8);
            db = (byte) (((((int) ag_b(dcolor)) * ralpha) + (((int) p->b[spos]) * falpha)) >> 8);
          }
        }
        
        //-- Dithering
        /*
        byte old_r = (byte) min(((int) dr) + ((int) qe[qx]),  255);
        byte old_g = (byte) min(((int) dg) + ((int) qe[qx+1]),255);
        byte old_b = (byte) min(((int) db) + ((int) qe[qx+2]),255);
        byte new_r = ag_close_r(old_r);
        byte new_g = ag_close_g(old_g);
        byte new_b = ag_close_b(old_b);
        byte err_r = old_r - new_r;
        byte err_g = old_g - new_g;
        byte err_b = old_b - new_b;
        
        //-- New Dithering
        ag_dither(qe,qn,qx,x,y,dw,dh,err_r,err_g,err_b);
        ag_setpixel(_b,dpx,dpy, ag_rgb(new_r,new_g,new_b));
        */
        ag_setpixel(_b, dpx, dpy, ag_dodither_rgb(x, y, dr, dg, db));
      }
    }
  }
  
  free(qe);
  return 1;
}