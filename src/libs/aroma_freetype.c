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
 * Freetype Font Handler
 *
 */

#include "../aroma.h"
#include FT_LCD_FILTER_H
#include FT_BITMAP_H
#include FT_OUTLINE_H


/*****************************[ GLOBAL VARIABLES ]*****************************/
static FT_Library             aft_lib;            // Freetype Library
static byte                   aft_initialized = 0; // Is Library Initialized
static byte                   aft_locked = 0;     // On Lock
static AFTFAMILY              aft_big;            // Big Font Family
static AFTFAMILY              aft_small;          // Small Font Family

/******************************[ LOCK FUNCTIONS ]******************************/
static pthread_mutex_t  _afont_mutex = PTHREAD_MUTEX_INITIALIZER;
void aft_waitlock() {
  pthread_mutex_lock(&_afont_mutex);
}
void aft_unlock() {
  pthread_mutex_unlock(&_afont_mutex);
}

/*******************************[ RTL FUNCTION ]*******************************/
//*
//* RTL CHECKER
//*
byte aft_isrtl(int c, byte checkleft) {
  if (
    (c == 0x5BE) || (c == 0x5C0) || (c == 0x5C3) || (c == 0x5C6) || ((c >= 0x5D0) && (c <= 0x5F4)) || (c == 0x608) || (c == 0x60B) || (c == 0x60D) ||
    ((c >= 0x61B) && (c <= 0x64A)) || ((c >= 0x66D) && (c <= 0x66F)) || ((c >= 0x671) && (c <= 0x6D5)) || ((c >= 0x6E5) && (c <= 0x6E6)) ||
    ((c >= 0x6EE) && (c <= 0x6EF)) || ((c >= 0x6FA) && (c <= 0x710)) || ((c >= 0x712) && (c <= 0x72F)) || ((c >= 0x74D) && (c <= 0x7A5)) ||
    ((c >= 0x7B1) && (c <= 0x7EA)) || ((c >= 0x7F4) && (c <= 0x7F5)) || ((c >= 0x7FA) && (c <= 0x815)) || (c == 0x81A) || (c == 0x824) || (c == 0x828) ||
    ((c >= 0x830) && (c <= 0x858)) || ((c >= 0x85E) && (c <= 0x8AC)) || (c == 0x200F) || (c == 0xFB1D) || ((c >= 0xFB1F) && (c <= 0xFB28)) ||
    ((c >= 0xFB2A) && (c <= 0xFD3D)) || ((c >= 0xFD50) && (c <= 0xFDFC)) || ((c >= 0xFE70) && (c <= 0xFEFC)) || ((c >= 0x10800) && (c <= 0x1091B)) ||
    ((c >= 0x10920) && (c <= 0x10A00)) || ((c >= 0x10A10) && (c <= 0x10A33)) || ((c >= 0x10A40) && (c <= 0x10B35)) || ((c >= 0x10B40) && (c <= 0x10C48)) ||
    ((c >= 0x1EE00) && (c <= 0x1EEBB))
  ) {
    return 1;
  }
  else if (checkleft) {
    if (
      ((c >= 0x41) && (c <= 0x5A)) || ((c >= 0x61) && (c <= 0x7A)) || (c == 0xAA) || (c == 0xB5) || (c == 0xBA) || ((c >= 0xC0) && (c <= 0xD6)) ||
      ((c >= 0xD8) && (c <= 0xF6)) || ((c >= 0xF8) && (c <= 0x2B8)) || ((c >= 0x2BB) && (c <= 0x2C1)) || ((c >= 0x2D0) && (c <= 0x2D1)) ||
      ((c >= 0x2E0) && (c <= 0x2E4)) || (c == 0x2EE) || ((c >= 0x370) && (c <= 0x373)) || ((c >= 0x376) && (c <= 0x37D)) || (c == 0x386) ||
      ((c >= 0x388) && (c <= 0x3F5)) || ((c >= 0x3F7) && (c <= 0x482)) || ((c >= 0x48A) && (c <= 0x589)) || ((c >= 0x903) && (c <= 0x939)) ||
      (c == 0x93B) || ((c >= 0x93D) && (c <= 0x940)) || ((c >= 0x949) && (c <= 0x94C)) || ((c >= 0x94E) && (c <= 0x950)) || ((c >= 0x958) && (c <= 0x961)) ||
      ((c >= 0x964) && (c <= 0x97F)) || ((c >= 0x982) && (c <= 0x9B9)) || ((c >= 0x9BD) && (c <= 0x9C0)) || ((c >= 0x9C7) && (c <= 0x9CC)) ||
      ((c >= 0x9CE) && (c <= 0x9E1)) || ((c >= 0x9E6) && (c <= 0x9F1)) || ((c >= 0x9F4) && (c <= 0x9FA)) || ((c >= 0xA03) && (c <= 0xA39)) ||
      ((c >= 0xA3E) && (c <= 0xA40)) || ((c >= 0xA59) && (c <= 0xA6F)) || ((c >= 0xA72) && (c <= 0xA74)) || ((c >= 0xA83) && (c <= 0xAB9)) ||
      ((c >= 0xABD) && (c <= 0xAC0)) || ((c >= 0xAC9) && (c <= 0xACC)) || ((c >= 0xAD0) && (c <= 0xAE1)) || ((c >= 0xAE6) && (c <= 0xAF0)) ||
      ((c >= 0xB02) && (c <= 0xB39)) || ((c >= 0xB3D) && (c <= 0xB3E)) || (c == 0xB40) || ((c >= 0xB47) && (c <= 0xB4C)) || ((c >= 0xB57) && (c <= 0xB61)) ||
      ((c >= 0xB66) && (c <= 0xB77)) || ((c >= 0xB83) && (c <= 0xBBF)) || ((c >= 0xBC1) && (c <= 0xBCC)) || ((c >= 0xBD0) && (c <= 0xBF2)) ||
      ((c >= 0xC01) && (c <= 0xC3D)) || ((c >= 0xC41) && (c <= 0xC44)) || ((c >= 0xC58) && (c <= 0xC61)) || ((c >= 0xC66) && (c <= 0xC6F)) ||
      ((c >= 0xC7F) && (c <= 0xCB9)) || ((c >= 0xCBD) && (c <= 0xCCB)) || ((c >= 0xCD5) && (c <= 0xCE1)) || ((c >= 0xCE6) && (c <= 0xD40)) ||
      ((c >= 0xD46) && (c <= 0xD4C)) || ((c >= 0xD4E) && (c <= 0xD61)) || ((c >= 0xD66) && (c <= 0xDC6)) || ((c >= 0xDCF) && (c <= 0xDD1)) ||
      ((c >= 0xDD8) && (c <= 0xE30)) || (c == 0xE32) || (c == 0xE40) || ((c >= 0xE4F) && (c <= 0xEB0)) || ((c >= 0xEB2) && (c <= 0xEB3)) ||
      ((c >= 0xEBD) && (c <= 0xEC6)) || ((c >= 0xED0) && (c <= 0xF17)) || ((c >= 0xF1A) && (c <= 0xF34)) || (c == 0xF36) || (c == 0xF38) ||
      ((c >= 0xF3E) && (c <= 0xF6C)) || (c == 0xF7F) || (c == 0xF85) || ((c >= 0xF88) && (c <= 0xF8C)) || ((c >= 0xFBE) && (c <= 0xFC5)) ||
      ((c >= 0xFC7) && (c <= 0x102C)) || (c == 0x1031) || (c == 0x1038) || ((c >= 0x103B) && (c <= 0x103C)) || ((c >= 0x103F) && (c <= 0x1057)) ||
      ((c >= 0x105A) && (c <= 0x105D)) || ((c >= 0x1061) && (c <= 0x1070)) || ((c >= 0x1075) && (c <= 0x1081)) || ((c >= 0x1083) && (c <= 0x1084)) ||
      ((c >= 0x1087) && (c <= 0x108C)) || ((c >= 0x108E) && (c <= 0x109C)) || ((c >= 0x109E) && (c <= 0x135A)) || ((c >= 0x1360) && (c <= 0x138F)) ||
      ((c >= 0x13A0) && (c <= 0x13F4)) || ((c >= 0x1401) && (c <= 0x167F)) || ((c >= 0x1681) && (c <= 0x169A)) || ((c >= 0x16A0) && (c <= 0x1711)) ||
      ((c >= 0x1720) && (c <= 0x1731)) || ((c >= 0x1735) && (c <= 0x1751)) || ((c >= 0x1760) && (c <= 0x1770)) || ((c >= 0x1780) && (c <= 0x17B3)) ||
      (c == 0x17B6) || ((c >= 0x17BE) && (c <= 0x17C5)) || ((c >= 0x17C7) && (c <= 0x17C8)) || ((c >= 0x17D4) && (c <= 0x17DA)) || (c == 0x17DC) ||
      ((c >= 0x17E0) && (c <= 0x17E9)) || ((c >= 0x1810) && (c <= 0x18A8)) || ((c >= 0x18AA) && (c <= 0x191C)) || ((c >= 0x1923) && (c <= 0x1926)) ||
      ((c >= 0x1929) && (c <= 0x1931)) || ((c >= 0x1933) && (c <= 0x1938)) || ((c >= 0x1946) && (c <= 0x19DA)) || ((c >= 0x1A00) && (c <= 0x1A16)) ||
      ((c >= 0x1A19) && (c <= 0x1A55)) || (c == 0x1A57) || (c == 0x1A61) || ((c >= 0x1A63) && (c <= 0x1A64)) || ((c >= 0x1A6D) && (c <= 0x1A72)) ||
      ((c >= 0x1A80) && (c <= 0x1AAD)) || ((c >= 0x1B04) && (c <= 0x1B33)) || (c == 0x1B35) || (c == 0x1B3B) || ((c >= 0x1B3D) && (c <= 0x1B41)) ||
      ((c >= 0x1B43) && (c <= 0x1B6A)) || ((c >= 0x1B74) && (c <= 0x1B7C)) || ((c >= 0x1B82) && (c <= 0x1BA1)) || ((c >= 0x1BA6) && (c <= 0x1BA7)) ||
      (c == 0x1BAA) || ((c >= 0x1BAC) && (c <= 0x1BE5)) || (c == 0x1BE7) || ((c >= 0x1BEA) && (c <= 0x1BEC)) || (c == 0x1BEE) || ((c >= 0x1BF2) && (c <= 0x1C2B)) ||
      ((c >= 0x1C34) && (c <= 0x1C35)) || ((c >= 0x1C3B) && (c <= 0x1CC7)) || (c == 0x1CD3) || (c == 0x1CE1) || ((c >= 0x1CE9) && (c <= 0x1CEC)) ||
      ((c >= 0x1CEE) && (c <= 0x1CF3)) || ((c >= 0x1CF5) && (c <= 0x1DBF)) || ((c >= 0x1E00) && (c <= 0x1FBC)) || (c == 0x1FBE) || ((c >= 0x1FC2) && (c <= 0x1FCC)) ||
      ((c >= 0x1FD0) && (c <= 0x1FDB)) || ((c >= 0x1FE0) && (c <= 0x1FEC)) || ((c >= 0x1FF2) && (c <= 0x1FFC)) || (c == 0x200E) || (c == 0x2071) || (c == 0x207F) ||
      ((c >= 0x2090) && (c <= 0x209C)) || (c == 0x2102) || (c == 0x2107) || ((c >= 0x210A) && (c <= 0x2113)) || (c == 0x2115) || ((c >= 0x2119) && (c <= 0x211D)) ||
      (c == 0x2124) || (c == 0x2126) || (c == 0x2128) || ((c >= 0x212A) && (c <= 0x212D)) || ((c >= 0x212F) && (c <= 0x2139)) || ((c >= 0x213C) && (c <= 0x213F)) ||
      ((c >= 0x2145) && (c <= 0x2149)) || ((c >= 0x214E) && (c <= 0x214F)) || ((c >= 0x2160) && (c <= 0x2188)) || ((c >= 0x2336) && (c <= 0x237A)) || (c == 0x2395) ||
      ((c >= 0x249C) && (c <= 0x24E9)) || (c == 0x26AC) || ((c >= 0x2800) && (c <= 0x28FF)) || ((c >= 0x2C00) && (c <= 0x2CE4)) || ((c >= 0x2CEB) && (c <= 0x2CEE)) ||
      ((c >= 0x2CF2) && (c <= 0x2CF3)) || ((c >= 0x2D00) && (c <= 0x2D70)) || ((c >= 0x2D80) && (c <= 0x2DDE)) || ((c >= 0x3005) && (c <= 0x3007)) ||
      ((c >= 0x3021) && (c <= 0x3029)) || ((c >= 0x302E) && (c <= 0x302F)) || ((c >= 0x3031) && (c <= 0x3035)) || ((c >= 0x3038) && (c <= 0x303C)) ||
      ((c >= 0x3041) && (c <= 0x3096)) || ((c >= 0x309D) && (c <= 0x309F)) || ((c >= 0x30A1) && (c <= 0x30FA)) || ((c >= 0x30FC) && (c <= 0x31BA)) ||
      ((c >= 0x31F0) && (c <= 0x321C)) || ((c >= 0x3220) && (c <= 0x324F)) || ((c >= 0x3260) && (c <= 0x327B)) || ((c >= 0x327F) && (c <= 0x32B0)) ||
      ((c >= 0x32C0) && (c <= 0x32CB)) || ((c >= 0x32D0) && (c <= 0x3376)) || ((c >= 0x337B) && (c <= 0x33DD)) || ((c >= 0x33E0) && (c <= 0x33FE)) ||
      ((c >= 0x3400) && (c <= 0x4DB5)) || ((c >= 0x4E00) && (c <= 0xA48C)) || ((c >= 0xA4D0) && (c <= 0xA60C)) || ((c >= 0xA610) && (c <= 0xA66E)) ||
      ((c >= 0xA680) && (c <= 0xA697)) || ((c >= 0xA6A0) && (c <= 0xA6EF)) || ((c >= 0xA6F2) && (c <= 0xA6F7)) || ((c >= 0xA722) && (c <= 0xA787)) ||
      ((c >= 0xA789) && (c <= 0xA801)) || ((c >= 0xA803) && (c <= 0xA805)) || ((c >= 0xA807) && (c <= 0xA80A)) || ((c >= 0xA80C) && (c <= 0xA824)) || (c == 0xA827) ||
      ((c >= 0xA830) && (c <= 0xA837)) || ((c >= 0xA840) && (c <= 0xA873)) || ((c >= 0xA880) && (c <= 0xA8C3)) || ((c >= 0xA8CE) && (c <= 0xA8D9)) ||
      ((c >= 0xA8F2) && (c <= 0xA925)) || ((c >= 0xA92E) && (c <= 0xA946)) || ((c >= 0xA952) && (c <= 0xA97C)) || ((c >= 0xA983) && (c <= 0xA9B2)) ||
      ((c >= 0xA9B4) && (c <= 0xA9B5)) || ((c >= 0xA9BA) && (c <= 0xA9BB)) || ((c >= 0xA9BD) && (c <= 0xAA28)) || ((c >= 0xAA2F) && (c <= 0xAA30)) ||
      ((c >= 0xAA33) && (c <= 0xAA34)) || ((c >= 0xAA40) && (c <= 0xAA42)) || ((c >= 0xAA44) && (c <= 0xAA4B)) || ((c >= 0xAA4D) && (c <= 0xAAAF)) ||
      (c == 0xAAB1) || ((c >= 0xAAB5) && (c <= 0xAAB6)) || ((c >= 0xAAB9) && (c <= 0xAABD)) || (c == 0xAAC0) || ((c >= 0xAAC2) && (c <= 0xAAEB)) ||
      ((c >= 0xAAEE) && (c <= 0xAAF5)) || ((c >= 0xAB01) && (c <= 0xABE4)) || ((c >= 0xABE6) && (c <= 0xABE7)) || ((c >= 0xABE9) && (c <= 0xABEC)) ||
      ((c >= 0xABF0) && (c <= 0xFB17)) || ((c >= 0xFF21) && (c <= 0xFF3A)) || ((c >= 0xFF41) && (c <= 0xFF5A)) || ((c >= 0xFF66) && (c <= 0xFFDC)) ||
      ((c >= 0x10000) && (c <= 0x10100)) || ((c >= 0x10102) && (c <= 0x1013F)) || ((c >= 0x101D0) && (c <= 0x101FC)) || ((c >= 0x10280) && (c <= 0x104A9)) ||
      (c == 0x11000) || ((c >= 0x11002) && (c <= 0x11037)) || ((c >= 0x11047) && (c <= 0x1104D)) || ((c >= 0x11066) && (c <= 0x1106F)) || ((c >= 0x11082) && (c <= 0x110B2)) ||
      ((c >= 0x110B7) && (c <= 0x110B8)) || ((c >= 0x110BB) && (c <= 0x110F9)) || ((c >= 0x11103) && (c <= 0x11126)) || (c == 0x1112C) || ((c >= 0x11136) && (c <= 0x11143)) ||
      ((c >= 0x11182) && (c <= 0x111B5)) || ((c >= 0x111BF) && (c <= 0x116AA)) || (c == 0x116AC) || ((c >= 0x116AE) && (c <= 0x116AF)) || (c == 0x116B6) ||
      ((c >= 0x116C0) && (c <= 0x16F7E)) || ((c >= 0x16F93) && (c <= 0x1D166)) || ((c >= 0x1D16A) && (c <= 0x1D172)) || ((c >= 0x1D183) && (c <= 0x1D184)) ||
      ((c >= 0x1D18C) && (c <= 0x1D1A9)) || ((c >= 0x1D1AE) && (c <= 0x1D1DD)) || ((c >= 0x1D360) && (c <= 0x1D6DA)) || ((c >= 0x1D6DC) && (c <= 0x1D714)) ||
      ((c >= 0x1D716) && (c <= 0x1D74E)) || ((c >= 0x1D750) && (c <= 0x1D788)) || ((c >= 0x1D78A) && (c <= 0x1D7C2)) || ((c >= 0x1D7C4) && (c <= 0x1D7CB)) ||
      ((c >= 0x1F110) && (c <= 0x1F169)) || ((c >= 0x1F170) && (c <= 0x1F251)) || ((c >= 0x20000) && (c <= 0x2FA1D))
    ) {
      return 0;
    }
  }
  
  return (checkleft ? 2 : 0);
}

/**************************[ GLYPH CACHE MANAGEMENT ]***************************/
//*
//* Create Glyph Cache for given face
//*
byte aft_createglyph(AFTFACEP f) {
  if (!aft_initialized) {
    return 0;
  }
  
  if (f == NULL) {
    return 0;
  }
  
  f->cache_n  = f->face->num_glyphs;
  int sz      = f->cache_n * sizeof(AFTGLYPH);
  f->cache    = (AFTGLYPHP) malloc(sz);
  memset(f->cache, 0, sz);
  return 1;
}

//*
//* Close Glyph Cache for given face
//*
byte aft_closeglyph(AFTFACEP f) {
  if (!aft_initialized) {
    return 0;
  }
  
  if (f == NULL) {
    return 0;
  }
  
  if (f->cache != NULL) {
    long i = 0;
    
    for (i = 0; i < f->cache_n; i++) {
      if (f->cache[i].init) {
        FT_Done_Glyph(f->cache[i].g);
        f->cache[i].init = 0;
      }
    }
    
    free(f->cache);
    f->cache = NULL;
    f->cache_n = 0;
  }
  
  return 1;
}

//*
//* Cache Readed Glyph
//*
byte aft_cacheglyph(AFTFACEP f, long id) {
  if (!aft_initialized) {
    return 0;
  }
  
  if (f == NULL) {
    return 0;
  }
  
  if (f->cache_n < id) {
    return 0;
  }
  
  if (!f->cache[id].init) {
    FT_Get_Glyph(f->face->glyph, &f->cache[id].g);
    f->cache[id].w    = f->face->glyph->advance.x >> 6;
    f->cache[id].init = 1;
  }
  
  return 1;
}

/**************************[ FONT FAMILY MANAGEMENT ]***************************/
//*
//* Get glyph index & face for given character
//*
long aft_id(AFTFACEP * f, int c, byte isbig) {
  if (!aft_initialized) {
    return 0;
  }
  
  if (c == 0xfeff) {
    return 0;
  }
  
  AFTFAMILYP m = (isbig != 0) ? &aft_big : &aft_small;
  
  if (!m->init) {
    return 0;
  }
  
  if (m->facen > 0) {
    aft_waitlock();
    long id = 0;
    int  i  = 0;
    
    for (i = 0; i < m->facen; i++) {
      id = FT_Get_Char_Index(m->faces[i].face, c);
      
      if (id != 0) {
        *f = &(m->faces[i]);
        aft_unlock();
        return id;
      }
    }
    
    *f = &(m->faces[0]);
    aft_unlock();
    return 0;
  }
  
  return 0;
}

//*
//* Get horizontal kerning size for given chars
//*
int aft_kern(int c, int p, byte isbig) {
  if (!aft_initialized) {
    return 0;
  }
  
  if ((c == 0xfeff) || (p == 0xfeff)) {
    return 0;
  }
  
  AFTFAMILYP m = (isbig != 0) ? &aft_big : &aft_small;
  
  if (!m->init) {
    return 0;
  }
  
  AFTFACEP cf = NULL;
  AFTFACEP pf = NULL;
  long  up = aft_id(&pf, p, isbig);
  long  uc = aft_id(&cf, c, isbig);
  
  if (up && uc && cf && pf) {
    if (cf == pf) {
      if (cf->kern == 1) {
        aft_waitlock();
        FT_Vector delta;
        FT_Get_Kerning(cf->face, up, uc, FT_KERNING_DEFAULT, &delta );
        aft_unlock();
        return (delta.x >> 6);
      }
    }
  }
  
  return 0;
}

//*
//* Free Font Family
//*
byte aft_free(AFTFAMILYP m) {
  if (!aft_initialized) {
    return 0;
  }
  
  if (m == NULL) {
    return 0;
  }
  
  if (!m->init) {
    return 0;
  }
  
  int fn = m->facen;
  m->facen = 0;
  m->init = 0;
  
  if (fn > 0) {
    int i;
    
    for (i = 0; i < fn; i++) {
      aft_closeglyph(&(m->faces[i]));
      FT_Done_Face(m->faces[i].face);
      free(m->faces[i].mem);
    }
    
    free(m->faces);
  }
  
  return 1;
}

//*
//* Load Font Family
//*
byte aft_load(const char * source_name, int size, byte isbig, char * relativeto) {
  if (!aft_initialized) {
    return 0;
  }
  
  const char * zip_paths = source_name;
  char  vc = 0;
  char  zpaths[10][256];
  int   count   = 0;
  int   zpath_n = 0;
  
  while ((vc = *zip_paths++)) {
    if ((zpath_n >= 255) || (count >= 10)) {
      break;
    }
    
    if (zpath_n == 0) {
      count++;
    }
    
    if (vc == ';') {
      zpaths[count - 1][zpath_n]  = 0;
      zpath_n = 0;
    }
    else {
      zpaths[count - 1][zpath_n++] = vc;
      zpaths[count - 1][zpath_n]  = 0;
    }
  }
  
  //-- Calculating Size
  if (!size) {
    size  = 12;  //-- Default Font Size
  }
  
  if (count > 10) {
    count = 10;  //-- Maximum Font per Family
  }
  
  byte m_s = size;
  byte m_p = ceil((agdp() * m_s) / 2);
  byte m_h = ceil(m_p * 1.1);
  byte m_y = (m_h - m_p) * 2;
  //-- Load Faces
  int i = 0;
  int c = 0;
  FT_Face ftfaces[10];
  char  * ftmem[10];
  
  for (i = 0; i < count; i++) {
    if (strlen(zpaths[i]) > 0) {
      char zpath[256];
      snprintf(zpath, 256, "%s%s", relativeto, zpaths[i]);
      AZMEM mem;
      
      if (az_readmem(&mem, zpath, 1)) {
        if (FT_New_Memory_Face(aft_lib, mem.data, mem.sz, 0, &ftfaces[c]) == 0) {
          if (FT_Set_Pixel_Sizes(ftfaces[c], 0, m_p) == 0) {
            ftmem[c] = mem.data;
            c++;
          }
          else {
            FT_Done_Face(ftfaces[c]);
            free(mem.data);
          }
        }
        else {
          free(mem.data);
        }
      }
    }
  }
  
  if (c > 0) {
    aft_waitlock();
    AFTFAMILYP m = (isbig != 0) ? &aft_big : &aft_small;
    //-- Cleanup Font
    aft_free(m);
    m->s = m_s;
    m->p = m_p;
    m->h = m_h;
    m->y = m_y;
    m->faces = malloc(sizeof(AFTFACE) * c);
    memset(m->faces, 0, sizeof(AFTFACE) * c);
    
    for (i = 0; i < c; i++) {
      m->faces[i].face = ftfaces[i];
      m->faces[i].mem = ftmem[i];
      m->faces[i].kern = FT_HAS_KERNING(m->faces[i].face) ? 1 : 0;
      aft_createglyph(&(m->faces[i]));
    }
    
    m->facen = c;
    m->init  = 1;
    LOGS("(%i) Freetype fonts loaded as Font Family\n", c);
    aft_unlock();
    return 1;
  }
  
  LOGS("No Freetype fonts loaded. Using png font.\n");
  return 0;
}

//*
//* Open Freetype Library
//*
byte aft_open() {
  if (aft_initialized) {
    return 0;
  }
  
  aft_big.init = 0;
  aft_small.init = 0;
  
  if (FT_Init_FreeType( &aft_lib ) == 0) {
    FT_Library_SetLcdFilter(aft_lib, FT_LCD_FILTER_DEFAULT);
    aft_initialized = 1;
    return 1;
  }
  
  return 0;
}

//*
//* Is Font Ready?
//*
byte aft_fontready(byte isbig) {
  if (!aft_initialized) {
    return 0;
  }
  
  AFTFAMILYP m = (isbig) ? &aft_big : &aft_small;
  
  if (!m->init) {
    return 0;
  }
  
  return 1;
}

//*
//* Close Freetype Library
//*
byte aft_close() {
  if (!aft_initialized) {
    return 0;
  }
  
  //-- Release All Font Family
  aft_free(&aft_big);
  aft_free(&aft_small);
  
  if (FT_Done_FreeType( aft_lib ) == 0) {
    aft_initialized = 0;
    return 1;
  }
  
  return 0;
}

//*
//* Font Width - No Auto Unlock
//*
int aft_fontwidth_lock(int c, byte isbig, AFTGLYPHP * ch, byte * onlock) {
  if (!aft_initialized) {
    return 0;
  }
  
  if (c == 0xfeff) {
    return 0;
  }
  
  AFTFACEP   f = NULL;
  long uc      = aft_id(&f, c, isbig);
  
  if (f == NULL) {
    return 0;
  }
  
  if (f->cache == NULL) {
    return 0;
  }
  
  if (uc > f->cache_n) {
    return 0;
  }
  
  aft_waitlock();
  *onlock = 1;
  
  if (f->cache[uc].init) {
    if (ch != NULL) {
      *ch = &f->cache[uc];
    }
    
    return f->cache[uc].w;
  }
  
  if (FT_Load_Glyph(f->face, uc, FT_LOAD_DEFAULT) == 0) {
    if (aft_cacheglyph(f, uc)) {
      if (ch != NULL) {
        *ch = &f->cache[uc];
      }
      
      return f->cache[uc].w;
    }
    
    return 0;
  }
  
  return 0;
}

//*
//* Font Width - Auto Unlock
//*
int aft_fontwidth(int c, byte isbig) {
  if (!aft_initialized) {
    return 0;
  }
  
  byte onlock = 0;
  int w = aft_fontwidth_lock(c, isbig, NULL, &onlock);
  
  if (onlock) {
    aft_unlock();
  }
  
  return w;
}

//*
//* Space Width
//*
int aft_spacewidth(byte isbig) {
  if (!aft_initialized) {
    return 0;
  }
  
  return aft_fontwidth(' ', isbig);
}

//*
//* Font Height
//*
byte aft_fontheight(byte isbig) {
  if (!aft_initialized) {
    return 0;
  }
  
  AFTFAMILYP m      = (isbig) ? &aft_big : &aft_small;
  
  if (!m->init) {
    return 0;
  }
  
  return m->h;
}
/* Multisampling Alpha */
color aAlphaMulti(color dcl, color scl, byte lr, byte lg, byte lb) {
  if (scl == dcl) {
    return scl;
  }
  else if (lr + lg + lb == 0) {
    return dcl;
  }
  else if (lr + lg + lb == 765) {
    return scl;
  }
  
  byte  rr = 255 - lr;
  byte  rg = 255 - lg;
  byte  rb = 255 - lb;
  byte r = (byte) (((((int) ag_r(dcl)) * rr) + (((int) ag_r(scl)) * lr)) >> 8);
  byte g = (byte) (((((int) ag_g(dcl)) * rg) + (((int) ag_g(scl)) * lg)) >> 8);
  byte b = (byte) (((((int) ag_b(dcl)) * rb) + (((int) ag_b(scl)) * lb)) >> 8);
  return ag_rgb(r, g, b);
}
//*
//* Draw Font
//*
byte aft_drawfont(CANVAS * _b, byte isbig, int fpos, int xpos, int ypos, color cl, byte underline, byte bold, byte italic, byte lcd) {
  if (!aft_initialized) {
    return 0;
  }
  
  //-- Is Default Canvas?
  if (_b == NULL) {
    _b = agc();
  }
  
  //-- Get Font Glyph
  AFTFAMILYP m      = (isbig) ? &aft_big : &aft_small;
  
  if (!m->init) {
    return 0;
  }
  
  AFTGLYPHP ch      = NULL;
  byte      onlock  = 0;
  int       fw      = aft_fontwidth_lock(fpos, isbig, &ch, &onlock);
  int       fh      = aft_fontheight(isbig);
  
  //-- Check Validity
  if ((fw == 0) || (ch == NULL)) {
    if (onlock) {
      aft_unlock();
    }
    
    return 0;
  }
  
  if (!ch->init) {
    if (onlock) {
      aft_unlock();
    }
    
    return 0;
  }
  
  //-- Copy & Render
  FT_Glyph glyph;
  FT_Glyph_Copy(ch->g, &glyph);
  /* Outline Embolden - BOLD */
  byte embolded = 0;
  
  if (bold) {
    if (glyph->format == FT_GLYPH_FORMAT_OUTLINE) {
      FT_OutlineGlyph foglyph = (FT_OutlineGlyph) glyph;
      FT_Outline_Embolden(&foglyph->outline, 80);
      embolded = 1;
    }
  }
  
  /* Transform Italic */
  if (italic) {
    FT_Matrix matrix;
    matrix.xx = 0x10000L;
    matrix.xy = 0x5000L;
    matrix.yx = 0;
    matrix.yy = 0x10000L;
    FT_Glyph_Transform(glyph, &matrix, NULL);
  }
  
  if (lcd) {
    FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_LCD, 0, 1);
  }
  else {
    FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
  }
  
  //-- Prepare Raster Glyph
  FT_BitmapGlyph  bit = (FT_BitmapGlyph) glyph;
  
  /* Bitmap Embolden  - BOLD */
  if ((bold) && (!embolded)) {
    FT_Bitmap_Embolden(bit->root.library, &bit->bitmap, 80, 80);
  }
  
  //-- Draw
  if (lcd) {
    int xx, yy;
    int fhalf = ceil(((float) fh) / 2);
    int bmp_w = bit->bitmap.width / 3;
    
    for (yy = 0; yy < bit->bitmap.rows; yy++) {
      for (xx = 0; xx < bmp_w; xx++) {
        byte ar = bit->bitmap.buffer[ (yy * bit->bitmap.pitch) + xx * 3];
        byte ag = bit->bitmap.buffer[ (yy * bit->bitmap.pitch) + xx * 3 + 1];
        byte ab = bit->bitmap.buffer[ (yy * bit->bitmap.pitch) + xx * 3 + 2];
        
        if (ar + ag + ab > 0) {
          int bx = xpos + bit->left + xx;
          int by = (ypos + yy + fh - m->y) - bit->top;
          color * dst = agxy(_b, bx, by);
          
          if (dst) {
            *dst = aAlphaMulti(*dst, cl, ar, ag, ab);
          }
        }
      }
    }
  }
  else {
    int xx, yy;
    int fhalf = ceil(((float) fh) / 2);
    int bmp_w = bit->bitmap.width;
    
    for (yy = 0; yy < bit->bitmap.rows; yy++) {
      for (xx = 0; xx < bmp_w; xx++) {
        byte a = bit->bitmap.buffer[ (yy * bit->bitmap.pitch) + xx];
        
        if (a > 0) {
          int bx = xpos + bit->left + xx;
          int by = (ypos + yy + fh - m->y) - bit->top;
          ag_subpixel(_b, bx, by, cl, a);
        }
      }
    }
  }
  
  //-- Release Glyph
  FT_Done_Glyph(glyph);
  
  //-- Draw Underline
  if (underline) {
    int usz = ceil(((float) m->p) / 12);
    int ux, uy;
    
    for (uy = m->p - usz; uy < m->p; uy++) {
      for (ux = 0; ux < fw; ux++) {
        ag_setpixel(_b, xpos + ux, ypos + uy, cl);
      }
    }
  }
  
  //-- Unlock
  if (onlock) {
    aft_unlock();
  }
  
  return 1;
}
