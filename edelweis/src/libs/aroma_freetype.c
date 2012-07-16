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

/*****************************[ GLOBAL VARIABLES ]*****************************/
static FT_Library             aft_lib;            // Freetype Library
static byte                   aft_initialized=0;  // Is Library Initialized
static byte                   aft_locked=0;       // On Lock
static AFTFAMILY              aft_big;            // Big Font Family
static AFTFAMILY              aft_small;          // Small Font Family

/******************************[ LOCK FUNCTIONS ]******************************/
void aft_waitlock(){ while (aft_locked) usleep(50); aft_locked=1; }
void aft_unlock(){ aft_locked=0; }

/*******************************[ RTL FUNCTION ]*******************************/
//*
//* RTL CHECKER
//*
byte aft_isrtl(int c, byte checkleft){
  if (
    (c==0x5BE)||(c==0x5C0)||(c==0x5C3)||(c==0x5C6)||((c>=0x5D0)&&(c<=0x5F4))||(c==0x608)||(c==0x60B)||(c==0x60D)||
    ((c>=0x61B)&&(c<=0x64A))||((c>=0x66D)&&(c<=0x66F))||((c>=0x671)&&(c<=0x6D5))||((c>=0x6E5)&&(c<=0x6E6))||
    ((c>=0x6EE)&&(c<=0x6EF))||((c>=0x6FA)&&(c<=0x710))||((c>=0x712)&&(c<=0x72F))||((c>=0x74D)&&(c<=0x7A5))||
    ((c>=0x7B1)&&(c<=0x7EA))||((c>=0x7F4)&&(c<=0x7F5))||((c>=0x7FA)&&(c<=0x815))||(c==0x81A)||(c==0x824)||(c==0x828)||
    ((c>=0x830)&&(c<=0x858))||((c>=0x85E)&&(c<=0x8AC))||(c==0x200F)||(c==0xFB1D)||((c>=0xFB1F)&&(c<=0xFB28))||
    ((c>=0xFB2A)&&(c<=0xFD3D))||((c>=0xFD50)&&(c<=0xFDFC))||((c>=0xFE70)&&(c<=0xFEFC))||((c>=0x10800)&&(c<=0x1091B))||
    ((c>=0x10920)&&(c<=0x10A00))||((c>=0x10A10)&&(c<=0x10A33))||((c>=0x10A40)&&(c<=0x10B35))||((c>=0x10B40)&&(c<=0x10C48))||
    ((c>=0x1EE00)&&(c<=0x1EEBB))
  ) return 1;
  else if (checkleft){
    if (
      ((c>=0x41)&&(c<=0x5A))||((c>=0x61)&&(c<=0x7A))||(c==0xAA)||(c==0xB5)||(c==0xBA)||((c>=0xC0)&&(c<=0xD6))||
      ((c>=0xD8)&&(c<=0xF6))||((c>=0xF8)&&(c<=0x2B8))||((c>=0x2BB)&&(c<=0x2C1))||((c>=0x2D0)&&(c<=0x2D1))||
      ((c>=0x2E0)&&(c<=0x2E4))||(c==0x2EE)||((c>=0x370)&&(c<=0x373))||((c>=0x376)&&(c<=0x37D))||(c==0x386)||
      ((c>=0x388)&&(c<=0x3F5))||((c>=0x3F7)&&(c<=0x482))||((c>=0x48A)&&(c<=0x589))||((c>=0x903)&&(c<=0x939))||
      (c==0x93B)||((c>=0x93D)&&(c<=0x940))||((c>=0x949)&&(c<=0x94C))||((c>=0x94E)&&(c<=0x950))||((c>=0x958)&&(c<=0x961))||
      ((c>=0x964)&&(c<=0x97F))||((c>=0x982)&&(c<=0x9B9))||((c>=0x9BD)&&(c<=0x9C0))||((c>=0x9C7)&&(c<=0x9CC))||
      ((c>=0x9CE)&&(c<=0x9E1))||((c>=0x9E6)&&(c<=0x9F1))||((c>=0x9F4)&&(c<=0x9FA))||((c>=0xA03)&&(c<=0xA39))||
      ((c>=0xA3E)&&(c<=0xA40))||((c>=0xA59)&&(c<=0xA6F))||((c>=0xA72)&&(c<=0xA74))||((c>=0xA83)&&(c<=0xAB9))||
      ((c>=0xABD)&&(c<=0xAC0))||((c>=0xAC9)&&(c<=0xACC))||((c>=0xAD0)&&(c<=0xAE1))||((c>=0xAE6)&&(c<=0xAF0))||
      ((c>=0xB02)&&(c<=0xB39))||((c>=0xB3D)&&(c<=0xB3E))||(c==0xB40)||((c>=0xB47)&&(c<=0xB4C))||((c>=0xB57)&&(c<=0xB61))||
      ((c>=0xB66)&&(c<=0xB77))||((c>=0xB83)&&(c<=0xBBF))||((c>=0xBC1)&&(c<=0xBCC))||((c>=0xBD0)&&(c<=0xBF2))||
      ((c>=0xC01)&&(c<=0xC3D))||((c>=0xC41)&&(c<=0xC44))||((c>=0xC58)&&(c<=0xC61))||((c>=0xC66)&&(c<=0xC6F))||
      ((c>=0xC7F)&&(c<=0xCB9))||((c>=0xCBD)&&(c<=0xCCB))||((c>=0xCD5)&&(c<=0xCE1))||((c>=0xCE6)&&(c<=0xD40))||
      ((c>=0xD46)&&(c<=0xD4C))||((c>=0xD4E)&&(c<=0xD61))||((c>=0xD66)&&(c<=0xDC6))||((c>=0xDCF)&&(c<=0xDD1))||
      ((c>=0xDD8)&&(c<=0xE30))||(c==0xE32)||(c==0xE40)||((c>=0xE4F)&&(c<=0xEB0))||((c>=0xEB2)&&(c<=0xEB3))||
      ((c>=0xEBD)&&(c<=0xEC6))||((c>=0xED0)&&(c<=0xF17))||((c>=0xF1A)&&(c<=0xF34))||(c==0xF36)||(c==0xF38)||
      ((c>=0xF3E)&&(c<=0xF6C))||(c==0xF7F)||(c==0xF85)||((c>=0xF88)&&(c<=0xF8C))||((c>=0xFBE)&&(c<=0xFC5))||
      ((c>=0xFC7)&&(c<=0x102C))||(c==0x1031)||(c==0x1038)||((c>=0x103B)&&(c<=0x103C))||((c>=0x103F)&&(c<=0x1057))||
      ((c>=0x105A)&&(c<=0x105D))||((c>=0x1061)&&(c<=0x1070))||((c>=0x1075)&&(c<=0x1081))||((c>=0x1083)&&(c<=0x1084))||
      ((c>=0x1087)&&(c<=0x108C))||((c>=0x108E)&&(c<=0x109C))||((c>=0x109E)&&(c<=0x135A))||((c>=0x1360)&&(c<=0x138F))||
      ((c>=0x13A0)&&(c<=0x13F4))||((c>=0x1401)&&(c<=0x167F))||((c>=0x1681)&&(c<=0x169A))||((c>=0x16A0)&&(c<=0x1711))||
      ((c>=0x1720)&&(c<=0x1731))||((c>=0x1735)&&(c<=0x1751))||((c>=0x1760)&&(c<=0x1770))||((c>=0x1780)&&(c<=0x17B3))||
      (c==0x17B6)||((c>=0x17BE)&&(c<=0x17C5))||((c>=0x17C7)&&(c<=0x17C8))||((c>=0x17D4)&&(c<=0x17DA))||(c==0x17DC)||
      ((c>=0x17E0)&&(c<=0x17E9))||((c>=0x1810)&&(c<=0x18A8))||((c>=0x18AA)&&(c<=0x191C))||((c>=0x1923)&&(c<=0x1926))||
      ((c>=0x1929)&&(c<=0x1931))||((c>=0x1933)&&(c<=0x1938))||((c>=0x1946)&&(c<=0x19DA))||((c>=0x1A00)&&(c<=0x1A16))||
      ((c>=0x1A19)&&(c<=0x1A55))||(c==0x1A57)||(c==0x1A61)||((c>=0x1A63)&&(c<=0x1A64))||((c>=0x1A6D)&&(c<=0x1A72))||
      ((c>=0x1A80)&&(c<=0x1AAD))||((c>=0x1B04)&&(c<=0x1B33))||(c==0x1B35)||(c==0x1B3B)||((c>=0x1B3D)&&(c<=0x1B41))||
      ((c>=0x1B43)&&(c<=0x1B6A))||((c>=0x1B74)&&(c<=0x1B7C))||((c>=0x1B82)&&(c<=0x1BA1))||((c>=0x1BA6)&&(c<=0x1BA7))||
      (c==0x1BAA)||((c>=0x1BAC)&&(c<=0x1BE5))||(c==0x1BE7)||((c>=0x1BEA)&&(c<=0x1BEC))||(c==0x1BEE)||((c>=0x1BF2)&&(c<=0x1C2B))||
      ((c>=0x1C34)&&(c<=0x1C35))||((c>=0x1C3B)&&(c<=0x1CC7))||(c==0x1CD3)||(c==0x1CE1)||((c>=0x1CE9)&&(c<=0x1CEC))||
      ((c>=0x1CEE)&&(c<=0x1CF3))||((c>=0x1CF5)&&(c<=0x1DBF))||((c>=0x1E00)&&(c<=0x1FBC))||(c==0x1FBE)||((c>=0x1FC2)&&(c<=0x1FCC))||
      ((c>=0x1FD0)&&(c<=0x1FDB))||((c>=0x1FE0)&&(c<=0x1FEC))||((c>=0x1FF2)&&(c<=0x1FFC))||(c==0x200E)||(c==0x2071)||(c==0x207F)||
      ((c>=0x2090)&&(c<=0x209C))||(c==0x2102)||(c==0x2107)||((c>=0x210A)&&(c<=0x2113))||(c==0x2115)||((c>=0x2119)&&(c<=0x211D))||
      (c==0x2124)||(c==0x2126)||(c==0x2128)||((c>=0x212A)&&(c<=0x212D))||((c>=0x212F)&&(c<=0x2139))||((c>=0x213C)&&(c<=0x213F))||
      ((c>=0x2145)&&(c<=0x2149))||((c>=0x214E)&&(c<=0x214F))||((c>=0x2160)&&(c<=0x2188))||((c>=0x2336)&&(c<=0x237A))||(c==0x2395)||
      ((c>=0x249C)&&(c<=0x24E9))||(c==0x26AC)||((c>=0x2800)&&(c<=0x28FF))||((c>=0x2C00)&&(c<=0x2CE4))||((c>=0x2CEB)&&(c<=0x2CEE))||
      ((c>=0x2CF2)&&(c<=0x2CF3))||((c>=0x2D00)&&(c<=0x2D70))||((c>=0x2D80)&&(c<=0x2DDE))||((c>=0x3005)&&(c<=0x3007))||
      ((c>=0x3021)&&(c<=0x3029))||((c>=0x302E)&&(c<=0x302F))||((c>=0x3031)&&(c<=0x3035))||((c>=0x3038)&&(c<=0x303C))||
      ((c>=0x3041)&&(c<=0x3096))||((c>=0x309D)&&(c<=0x309F))||((c>=0x30A1)&&(c<=0x30FA))||((c>=0x30FC)&&(c<=0x31BA))||
      ((c>=0x31F0)&&(c<=0x321C))||((c>=0x3220)&&(c<=0x324F))||((c>=0x3260)&&(c<=0x327B))||((c>=0x327F)&&(c<=0x32B0))||
      ((c>=0x32C0)&&(c<=0x32CB))||((c>=0x32D0)&&(c<=0x3376))||((c>=0x337B)&&(c<=0x33DD))||((c>=0x33E0)&&(c<=0x33FE))||
      ((c>=0x3400)&&(c<=0x4DB5))||((c>=0x4E00)&&(c<=0xA48C))||((c>=0xA4D0)&&(c<=0xA60C))||((c>=0xA610)&&(c<=0xA66E))||
      ((c>=0xA680)&&(c<=0xA697))||((c>=0xA6A0)&&(c<=0xA6EF))||((c>=0xA6F2)&&(c<=0xA6F7))||((c>=0xA722)&&(c<=0xA787))||
      ((c>=0xA789)&&(c<=0xA801))||((c>=0xA803)&&(c<=0xA805))||((c>=0xA807)&&(c<=0xA80A))||((c>=0xA80C)&&(c<=0xA824))||(c==0xA827)||
      ((c>=0xA830)&&(c<=0xA837))||((c>=0xA840)&&(c<=0xA873))||((c>=0xA880)&&(c<=0xA8C3))||((c>=0xA8CE)&&(c<=0xA8D9))||
      ((c>=0xA8F2)&&(c<=0xA925))||((c>=0xA92E)&&(c<=0xA946))||((c>=0xA952)&&(c<=0xA97C))||((c>=0xA983)&&(c<=0xA9B2))||
      ((c>=0xA9B4)&&(c<=0xA9B5))||((c>=0xA9BA)&&(c<=0xA9BB))||((c>=0xA9BD)&&(c<=0xAA28))||((c>=0xAA2F)&&(c<=0xAA30))||
      ((c>=0xAA33)&&(c<=0xAA34))||((c>=0xAA40)&&(c<=0xAA42))||((c>=0xAA44)&&(c<=0xAA4B))||((c>=0xAA4D)&&(c<=0xAAAF))||
      (c==0xAAB1)||((c>=0xAAB5)&&(c<=0xAAB6))||((c>=0xAAB9)&&(c<=0xAABD))||(c==0xAAC0)||((c>=0xAAC2)&&(c<=0xAAEB))||
      ((c>=0xAAEE)&&(c<=0xAAF5))||((c>=0xAB01)&&(c<=0xABE4))||((c>=0xABE6)&&(c<=0xABE7))||((c>=0xABE9)&&(c<=0xABEC))||
      ((c>=0xABF0)&&(c<=0xFB17))||((c>=0xFF21)&&(c<=0xFF3A))||((c>=0xFF41)&&(c<=0xFF5A))||((c>=0xFF66)&&(c<=0xFFDC))||
      ((c>=0x10000)&&(c<=0x10100))||((c>=0x10102)&&(c<=0x1013F))||((c>=0x101D0)&&(c<=0x101FC))||((c>=0x10280)&&(c<=0x104A9))||
      (c==0x11000)||((c>=0x11002)&&(c<=0x11037))||((c>=0x11047)&&(c<=0x1104D))||((c>=0x11066)&&(c<=0x1106F))||((c>=0x11082)&&(c<=0x110B2))||
      ((c>=0x110B7)&&(c<=0x110B8))||((c>=0x110BB)&&(c<=0x110F9))||((c>=0x11103)&&(c<=0x11126))||(c==0x1112C)||((c>=0x11136)&&(c<=0x11143))||
      ((c>=0x11182)&&(c<=0x111B5))||((c>=0x111BF)&&(c<=0x116AA))||(c==0x116AC)||((c>=0x116AE)&&(c<=0x116AF))||(c==0x116B6)||
      ((c>=0x116C0)&&(c<=0x16F7E))||((c>=0x16F93)&&(c<=0x1D166))||((c>=0x1D16A)&&(c<=0x1D172))||((c>=0x1D183)&&(c<=0x1D184))||
      ((c>=0x1D18C)&&(c<=0x1D1A9))||((c>=0x1D1AE)&&(c<=0x1D1DD))||((c>=0x1D360)&&(c<=0x1D6DA))||((c>=0x1D6DC)&&(c<=0x1D714))||
      ((c>=0x1D716)&&(c<=0x1D74E))||((c>=0x1D750)&&(c<=0x1D788))||((c>=0x1D78A)&&(c<=0x1D7C2))||((c>=0x1D7C4)&&(c<=0x1D7CB))||
      ((c>=0x1F110)&&(c<=0x1F169))||((c>=0x1F170)&&(c<=0x1F251))||((c>=0x20000)&&(c<=0x2FA1D))
      ) return 0;
  }
  return (checkleft?2:0);
}

/*****************************[ ARABIC FUNCTIONS ]****************************/
//*
//* ARABIC MACROS
//*
#define AFT_ARABIC_PROP_ISOLATED        0
#define AFT_ARABIC_PROP_INITIAL         1
#define AFT_ARABIC_PROP_MEDIAL          2
#define AFT_ARABIC_PROP_FINAL           3
#define AFT_ARABIC_CLASS_NONE           0
#define AFT_ARABIC_CLASS_TRANSPARENT    1
#define AFT_ARABIC_CLASS_RIGHT          2
#define AFT_ARABIC_CLASS_DUAL           3
#define AFT_ARABIC_CLASS_CAUSING        4

//*
//* ARABIC CONSTANT
//*
static const byte AFT_ARABIC[] = {
  /* U+0620 */ 0, 0, 2, 2, 2, 2, 3, 2, 3, 2, 3, 3, 3, 3, 3, 2,
  /* U+0630 */ 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0,
  /* U+0640 */ 4, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 1, 1, 1, 1, 1,
  /* U+0650 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
  /* U+0660 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3,
  /* U+0670 */ 1, 2, 2, 2, 0, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
  /* U+0680 */ 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2,
  /* U+0690 */ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3,
  /* U+06A0 */ 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  /* U+06B0 */ 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  /* U+06C0 */ 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 3, 2,
  /* U+06D0 */ 3, 3, 2, 2, 0, 2, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  /* U+06E0 */ 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 2, 2,
  /* U+06F0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 0, 0, 3
};
static const byte AFT_ARABIC_SUP[] = {
  /* U+0750 */ 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 3, 3, 3, 3,
  /* U+0760 */ 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 3, 0, 0,
  /* U+0770 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
static const byte AFT_ARABIC_NKO[] =
{
  /* U+07C0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3, 3, 3,
  /* U+07D0 */ 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  /* U+07E0 */ 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1,
  /* U+07F0 */ 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0,
};
static const int AFT_ARABIC_PRES[]={
  0xFE81, 0xFE82, 0, 0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0xFE8D, 0xFE8E, 0, 0, 0xFE8F, 0xFE90, 0xFE92, 0xFE91,
  0xFE93, 0xFE94, 0, 0, 0xFE95, 0xFE96, 0xFE98, 0xFE97, 0xFE99, 0xFE9A, 0xFE9C, 0xFE9B, 0xFE9D, 0xFE9E, 0xFEA0, 0xFE9F,
  0xFEA1, 0xFEA2, 0xFEA4, 0xFEA3, 0xFEA5, 0xFEA6, 0xFEA8, 0xFEA7, 0xFEA9, 0xFEAA, 0, 0, 0xFEAB, 0xFEAC, 0, 0,
  0xFEAD, 0xFEAE, 0, 0, 0xFEAF, 0xFEB0, 0, 0, 0xFEB1, 0xFEB2, 0xFEB4, 0xFEB3, 0xFEB5, 0xFEB6, 0xFEB8, 0xFEB7,
  0xFEB9, 0xFEBA, 0xFEBC, 0xFEBB, 0xFEBD, 0xFEBE, 0xFEC0, 0xFEBF, 0xFEC1, 0xFEC2, 0xFEC4, 0xFEC3, 0xFEC5, 0xFEC6, 0xFEC8, 0xFEC7,
  0xFEC9, 0xFECA, 0xFECC, 0xFECB, 0xFECD, 0xFECE, 0xFED0, 0xFECF, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
  0,0,0,0, 0xFED1, 0xFED2, 0xFED4, 0xFED3, 0xFED5, 0xFED6, 0xFED8, 0xFED7, 0xFED9, 0xFEDA, 0xFEDC, 0xFEDB,
  0xFEDD, 0xFEDE, 0xFEE0, 0xFEDF, 0xFEE1, 0xFEE2, 0xFEE4, 0xFEE3, 0xFEE5, 0xFEE6, 0xFEE8, 0xFEE7, 0xFEE9, 0xFEEA, 0xFEEC, 0xFEEB,
  0xFEED, 0xFEEE, 0, 0, 0xFEEF, 0xFEF0, 0, 0, 0xFEF1, 0xFEF2, 0xFEF4, 0xFEF3
};

//*
//* Get Arabic Character Class
//*
static byte AFT_ARABIC_GETCLASS(int * string, int pos, int length, int direction){
  
  byte j=0;
  while (1) {
    if (pos==0&&direction<0) return AFT_ARABIC_CLASS_NONE;
    pos += direction;
    if (pos >= length) return AFT_ARABIC_CLASS_NONE;

    if (string[pos]>=0x0620 && string[pos] < 0x0700)    j = AFT_ARABIC[string[pos] - 0x0620];
    else if (string[pos]>=0x0750 && string[pos]<0x0780) j = AFT_ARABIC_SUP[string[pos] - 0x0750];
    else if (string[pos]>=0x07C0 && string[pos]<0x0800) j = AFT_ARABIC_NKO[string[pos] - 0x07C0];
    else if (string[pos]==0x200D) return AFT_ARABIC_CLASS_CAUSING;
    else return AFT_ARABIC_CLASS_NONE;

    if (!direction||j!=AFT_ARABIC_CLASS_TRANSPARENT) return j;
  }
  return AFT_ARABIC_CLASS_NONE;
}

//*
//* Get Arabic Character Properties
//*
static byte AFT_ARABIC_GETPROP(int *string, byte *prop, int length){
  byte  cp, cc, cn;
  int   i;
  if (!string||!prop||length==0) return 0;

  for (i = 0; i < length; i++) {
    cp = AFT_ARABIC_GETCLASS(string, i, length, -1);
    cc = AFT_ARABIC_GETCLASS(string, i, length,  0);
    cn = AFT_ARABIC_GETCLASS(string, i, length,  1);
    
    if (cc==AFT_ARABIC_CLASS_TRANSPARENT) {
  	  prop[i] = AFT_ARABIC_PROP_ISOLATED;
  	  continue;
  	}
    if (cp==AFT_ARABIC_CLASS_CAUSING || cp==AFT_ARABIC_CLASS_DUAL){
      if (cc==AFT_ARABIC_CLASS_RIGHT){
        prop[i] = AFT_ARABIC_PROP_FINAL;
  	    continue;
      }
    }
    if (cp==AFT_ARABIC_CLASS_CAUSING || cp==AFT_ARABIC_CLASS_DUAL){
      if (cc==AFT_ARABIC_CLASS_DUAL){
        if (cn==AFT_ARABIC_CLASS_CAUSING||cn==AFT_ARABIC_CLASS_RIGHT||cn==AFT_ARABIC_CLASS_DUAL){
          prop[i] = AFT_ARABIC_PROP_MEDIAL;
  	      continue;
        }
      }
    }
    if (cp==AFT_ARABIC_CLASS_CAUSING || cp==AFT_ARABIC_CLASS_DUAL){
      if (cc==AFT_ARABIC_CLASS_DUAL){
        if (!(cn==AFT_ARABIC_CLASS_CAUSING||cn==AFT_ARABIC_CLASS_RIGHT||cn==AFT_ARABIC_CLASS_DUAL)){
          prop[i] = AFT_ARABIC_PROP_FINAL;
  	      continue;
        }
      }
    }
    if (!(cp==AFT_ARABIC_CLASS_CAUSING||cp==AFT_ARABIC_CLASS_DUAL)){
      if (cc==AFT_ARABIC_CLASS_DUAL){
        if (cn==AFT_ARABIC_CLASS_CAUSING||cn==AFT_ARABIC_CLASS_RIGHT||cn==AFT_ARABIC_CLASS_DUAL){
          prop[i] = AFT_ARABIC_PROP_INITIAL;
  	      continue;
        }
      }
    }
    prop[i] = AFT_ARABIC_PROP_ISOLATED;
  }
  return 1;
}

//*
//* Is Character was Arabic Character?
//*
byte AFT_ISARABIC(int c){
  if (c >= 0x0620 && c < 0x0700)      return 1;
  else if (c >= 0x0750 && c < 0x0780) return 1;
  else if (c >= 0x07C0 && c < 0x0800) return 1;
  else if (c == 0x200D) return 1;
	return 0;
}

//*
//* Read and Convert Arabic Unicode Chars
//*
byte aft_read_arabic(int * soff, const char * src, const char ** ss, int * string, byte * prop, int maxlength, int * outlength, int * move){
  if (!AFT_ISARABIC(*soff)) return 0;

  int off=*soff;
  int   i=0;
  memset(string,  0,  sizeof(int) *maxlength);
  memset(prop,    0,  sizeof(byte)*maxlength);
  
  const char * read_buffer = src;
  const char * readed_buffer= src;
  int readed_off = off;
  int last_movesz= 0;
  do{
    if (i>=maxlength) break;
    string[i++] = off;
    int movesz= 0;
    readed_buffer = read_buffer;
    readed_off    = off;
    off=utf8c(read_buffer,&read_buffer,&movesz);
    *move+=movesz;
    last_movesz=movesz;
  }while(AFT_ISARABIC(off));
  
  if (ss!=NULL) *ss = readed_buffer; 
  *soff       = readed_off;
  *outlength  = i;
  *move-=last_movesz;
  
  //-- FETCH ARABIC PROP
  AFT_ARABIC_GETPROP(string,prop,i);
  
  int j=0;
  for (j=0;j<i;j++){
    int cs = string[j];
    byte ps= prop[j];
    if ((cs>=0x622)&&(cs<=0x64A)){
      int psub = ((cs-0x622)*4);
      if (ps==AFT_ARABIC_PROP_INITIAL)      psub+=3;
      else if (ps==AFT_ARABIC_PROP_MEDIAL)  psub+=2;
      else if (ps==AFT_ARABIC_PROP_FINAL)   psub+=1;
      int csub = AFT_ARABIC_PRES[psub];
      if (csub!=0) string[j]=csub;
    }
  }
  return 1;
}

/**************************[ GLYPH CACHE MANAGEMENT ]***************************/
//*
//* Create Glyph Cache for given face
//*
byte aft_createglyph(AFTFACEP f){
  if (!aft_initialized) return 0;
  if (f==NULL) return 0;
  f->cache_n  = f->face->num_glyphs;
  int sz      = f->cache_n * sizeof(AFTGLYPH);
  f->cache    = (AFTGLYPHP) malloc(sz);
  memset(f->cache,0,sz);
  return 1;
}

//*
//* Close Glyph Cache for given face
//*
byte aft_closeglyph(AFTFACEP f){
  if (!aft_initialized) return 0;
  if (f==NULL) return 0;
  if (f->cache!=NULL){
    long i=0;
    for (i=0;i<f->cache_n;i++){
      if (f->cache[i].init){
        FT_Done_Glyph(f->cache[i].g);
        f->cache[i].init=0;
      }
    }
    free(f->cache);
    f->cache=NULL;
    f->cache_n=0;
  }
  return 1;
}

//*
//* Cache Readed Glyph
//*
byte aft_cacheglyph(AFTFACEP f, long id){
  if (!aft_initialized) return 0;
  if (f==NULL) return 0;
  if (f->cache_n<id) return 0;

  if (!f->cache[id].init){
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
long aft_id(AFTFACEP * f, int c, byte isbig){
  if (!aft_initialized) return 0;
  if (c==0xfeff) return 0;
  AFTFAMILYP m = (isbig!=0)?&aft_big:&aft_small;
  if (!m->init) return 0;
  if (m->facen>0){
    aft_waitlock();
    long id = 0;
    int  i  = 0;
    for (i=0;i<m->facen;i++){
      id = FT_Get_Char_Index(m->faces[i].face,c);
      if (id!=0){
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
int aft_kern(int c, int p, byte isbig){
  if (!aft_initialized) return 0;
  if ((c==0xfeff)||(p==0xfeff)) return 0;
  AFTFAMILYP m = (isbig!=0)?&aft_big:&aft_small;
  if (!m->init) return 0;
      
  AFTFACEP cf=NULL;
  AFTFACEP pf=NULL;
  long  up = aft_id(&pf,p,isbig);
  long  uc = aft_id(&cf,c,isbig);
  if (up&&uc&&cf&&pf){
    if (cf==pf){
      if (cf->kern==1){
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
byte aft_free(AFTFAMILYP m){
  if (!aft_initialized) return 0;
  if (m==NULL) return 0;
  if (!m->init) return 0;

  int fn = m->facen;
  m->facen=0;
  m->init=0;
  if (fn>0){
    int i;
    for (i=0;i<fn;i++){
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
byte aft_load(const char * source_name, int size, byte isbig,char * relativeto){
  if (!aft_initialized) return 0;
  
  const char * zip_paths = source_name;
  char  vc=0;
  char  zpaths[10][256];
  int   count   = 0;
  int   zpath_n = 0;
  
  while ((vc=*zip_paths++)){
    if ((zpath_n>=255)||(count>=10)) break;
    if (zpath_n==0) count++;
    if (vc==';'){
      zpaths[count-1][zpath_n]  =0;
      zpath_n=0;
    }
    else{
      zpaths[count-1][zpath_n++]=vc;
      zpaths[count-1][zpath_n]  =0;
    }
  }

  //-- Calculating Size
  if (!size)    size  = 12; //-- Default Font Size
  if (count>10) count = 10; //-- Maximum Font per Family
  byte m_s = size;
  byte m_p = ceil((agdp() * m_s) / 2);
  byte m_h = ceil(m_p * 1.1);
  byte m_y = (m_h-m_p)*2;
  
  //-- Load Faces
  int i=0;
  int c=0;
  FT_Face ftfaces[10];
  char *  ftmem[10];
  
  for (i=0;i<count;i++){
    if (strlen(zpaths[i])>0){
      char zpath[256];
      snprintf(zpath,256,"%s%s",relativeto,zpaths[i]);
      AZMEM mem;
      if (az_readmem(&mem,zpath,1)){
        if (FT_New_Memory_Face(aft_lib,mem.data,mem.sz,0,&ftfaces[c])==0){
          if (FT_Set_Pixel_Sizes(ftfaces[c], 0, m_p)==0){
            ftmem[c]=mem.data;
            c++;
          }
          else{
            FT_Done_Face(ftfaces[c]);
            free(mem.data);
          }
        }
        else
          free(mem.data);
      }
    }
  }
  
  if (c>0){
    aft_waitlock();
    AFTFAMILYP m = (isbig!=0)?&aft_big:&aft_small;

    //-- Cleanup Font
    aft_free(m);
  
    m->s = m_s;
    m->p = m_p;
    m->h = m_h;
    m->y = m_y;
  
    m->faces = malloc(sizeof(AFTFACE) * c);
    memset(m->faces,0,sizeof(AFTFACE) * c);
    for (i=0;i<c;i++){
      m->faces[i].face=ftfaces[i];
      m->faces[i].mem =ftmem[i];
      m->faces[i].kern=FT_HAS_KERNING(m->faces[i].face)?1:0;
      aft_createglyph(&(m->faces[i]));
    }
    
    m->facen = c;
    m->init  = 1;
    LOGS("(%i) Freetype fonts loaded as Font Family\n",c);
    aft_unlock();
    return 1;
  }
  
  LOGS("No Freetype fonts loaded. Using png font.\n");
  return 0;  
}

//*
//* Open Freetype Library
//*
byte aft_open(){
  if (aft_initialized) return 0;
  aft_big.init=0;
  aft_small.init=0;
  if (FT_Init_FreeType( &aft_lib )==0){
    aft_initialized=1;
    return 1;
  }
  return 0;
}

//*
//* Is Font Ready?
//*
byte aft_fontready(byte isbig){
  if (!aft_initialized) return 0;
  AFTFAMILYP m = (isbig)?&aft_big:&aft_small;
  if (!m->init) return 0;
  return 1;
}

//*
//* Close Freetype Library
//*
byte aft_close(){
  if (!aft_initialized) return 0;
  
  //-- Release All Font Family  
  aft_free(&aft_big);
  aft_free(&aft_small);
  
  if (FT_Done_FreeType( aft_lib )==0){
    aft_initialized = 0;
    return 1;
  }
  
  return 0;
}

//*
//* Font Width - No Auto Unlock
//*
int aft_fontwidth_lock(int c,byte isbig,AFTGLYPHP * ch,byte * onlock){
  if (!aft_initialized) return 0;
  if (c==0xfeff) return 0;

  AFTFACEP   f = NULL;
  long uc      = aft_id(&f, c, isbig);
  
  if (f==NULL) return 0;
  if (f->cache==NULL) return 0;
  if (uc>f->cache_n) return 0;
  
  aft_waitlock();
  *onlock=1;
  
  if (f->cache[uc].init){
    if (ch!=NULL) *ch=&f->cache[uc];
    return f->cache[uc].w;
  }
  
  if (FT_Load_Glyph(f->face,uc,FT_LOAD_DEFAULT)==0){
    if (aft_cacheglyph(f,uc)){
      if (ch!=NULL) *ch=&f->cache[uc];
      return f->cache[uc].w;
    }
    return 0;
  }
  return 0;
}

//*
//* Font Width - Auto Unlock
//*
int aft_fontwidth(int c,byte isbig){
  if (!aft_initialized) return 0;
  byte onlock=0;
  int w=aft_fontwidth_lock(c,isbig,NULL,&onlock);
  if (onlock) aft_unlock();
  return w;
}

//*
//* Space Width
//*
int aft_spacewidth(byte isbig){
  if (!aft_initialized) return 0;
  return aft_fontwidth(' ',isbig);
}

//*
//* Font Height
//*
byte aft_fontheight(byte isbig){
  if (!aft_initialized) return 0;
  AFTFAMILYP m      = (isbig)?&aft_big:&aft_small;
  if (!m->init) return 0;
  return m->h;
}

//*
//* Draw Font
//*
byte aft_drawfont(CANVAS * _b, byte isbig, int fpos, int xpos, int ypos, color cl,byte underline,byte bold){
  if (!aft_initialized) return 0;
  
  //-- Is Default Canvas?
  if (_b==NULL) _b=agc();
  
  //-- Get Font Glyph
  AFTFAMILYP m      = (isbig)?&aft_big:&aft_small;
  if (!m->init) return 0;
  
  AFTGLYPHP ch      = NULL;
  byte      onlock  = 0;
  int       fw      = aft_fontwidth_lock(fpos,isbig,&ch,&onlock);
  int       fh      = aft_fontheight(isbig);
  
  //-- Check Validity
  if ((fw==0)||(ch==NULL)){
    if (onlock) aft_unlock();
    return 0;
  }  
  if (!ch->init){
    if (onlock) aft_unlock();
    return 0;
  }
  
  //-- Copy & Render
  FT_Glyph glyph;
  FT_Glyph_Copy(ch->g,&glyph);
  FT_Glyph_To_Bitmap(&glyph,FT_RENDER_MODE_NORMAL,0,1);
  
  //-- Prepare Raster Glyph
  FT_BitmapGlyph  bit = (FT_BitmapGlyph) glyph;
  
  //-- Draw
  int xx, yy;
  int fhalf=ceil(fh/2);
	for (yy=0; yy < bit->bitmap.rows; yy++) {
	  for (xx=0; xx < bit->bitmap.width; xx++) {
      byte a = bit->bitmap.buffer[ (yy * bit->bitmap.pitch) + xx ];
      if (a>0){
        int bx = xpos+bit->left+xx;
        int by = (ypos+yy+fh-m->y)-bit->top;
        ag_subpixel(_b,bx,by,cl,a);
        if (bold){
          ag_subpixel(_b,bx-1,by-1,cl,a/4);
          ag_subpixel(_b,bx,  by-1,cl,a/2);
          ag_subpixel(_b,bx+1,by-1,cl,a/4);
          ag_subpixel(_b,bx-1,by,cl,a/2);
          ag_subpixel(_b,bx,by,cl,a);
        }
			}
    }
	}
	
	//-- Release Glyph
	FT_Done_Glyph(glyph);
	
	//-- Draw Underline
	if (underline){
	  int usz = ceil(m->p/12);
	  int ux,uy;
	  for (uy=m->p-usz;uy<m->p;uy++){
	    for (ux=0;ux<fw;ux++){
        ag_setpixel(_b,xpos+ux,ypos+uy,cl);
	    }
	  }
  }
  
  //-- Unlock
  if (onlock) aft_unlock();
  return 1;
}