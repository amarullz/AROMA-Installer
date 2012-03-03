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
static AFTFONT                aft_big;            // Big Font Face
static AFTFONT                aft_small;          // Small Font Face
static byte                   aft_locked=0;

void aft_waitlock(){
  while (aft_locked) usleep(50);
  aft_locked=1;
}
void aft_unlock(){
  aft_locked=0;
}

//*
//* GLYPH
//*
void aft_createglyph(AFTFONTP f){
  f->cn  = f->face->num_glyphs;
  int sz = f->cn * sizeof(AFTGLYPH);
  f->c   = (AFTGLYPHP) malloc(sz);
  memset(f->c,0,sz);
}
void aft_closeglyph(AFTFONTP f){
  if (f->c!=NULL){
    long i=0;
    for (i=0;i<f->cn;i++){
      if (f->c[i].init){
        FT_Done_Glyph(f->c[i].g);
        f->c[i].init=0;
      }
    }
    free(f->c);
    f->c=NULL;
  }
}
void aft_cacheglyph(AFTFONTP f, long id){
  if (!f->c[id].init){
    FT_Get_Glyph(f->face->glyph, &f->c[id].g);
    f->c[id].w    = f->face->glyph->advance.x >> 6;
    f->c[id].init = 1;
  }
}

//*
//* Close Font
//*
byte aft_closefont(AFTFONTP f){
  if (f->init){
    LOGS("Freetype Close : %s\n",f->zp);
    f->init = 0;
    free(f->mem.data);
    aft_closeglyph(f);
    if (FT_Done_Face(f->face)==0) return 1;
  }
  return 0;
}

//*
//* Open Freetype Library
//*
byte aft_open(){
  sprintf(aft_small.zp,"");
  sprintf(aft_big.zp,"");
  if (FT_Init_FreeType( &aft_lib )==0) return 1;
  return 0;
}

//*
//* Close Freetype Library
//*
byte aft_close(){
  aft_closefont(&aft_big);
  aft_closefont(&aft_small);
  if (FT_Done_FreeType( aft_lib )==0) return 1;
  return 0;
}

//*
//* Font Width
//*
byte aft_fontwidth_lock(int c,byte isbig,AFTGLYPHP * ch,byte * onlock){
  if (c==0xfeff) return 0;

  AFTFONTP f = &aft_small;
  if (isbig!=0) f = &aft_big;
    
  if (f->init==0) return 0;
    
  long uc = FT_Get_Char_Index(f->face,c);
  
  if (f->c[uc].init){
    if (ch!=NULL) *ch=&f->c[uc];
    return f->c[uc].w;
  }
  
  aft_waitlock();
  *onlock=1;
  if (FT_Load_Glyph(f->face,uc,FT_LOAD_DEFAULT)==0){
    aft_cacheglyph(f,uc);
    if (ch!=NULL) *ch=&f->c[uc];
    return f->c[uc].w;
  }
  return 0;
}
byte aft_fontwidth(int c,byte isbig){
  byte onlock=0;
  byte w=aft_fontwidth_lock(c,isbig,NULL,&onlock);
  if (onlock) aft_unlock();
  return w;
}



//*
//* Set Font Size
//*
byte aft_setfontsize(byte size, byte isbig){
  AFTFONTP f = &aft_small;
  if (isbig!=0) f = &aft_big;
  
  if (f->init==0) return 0;

  if (!size) size=12;
  f->s = size;
  f->p = (agdp() * f->s) / 2;
  f->h = ceil(f->p * 1.2);
  f->y = (f->h-f->p); // / 2;
  
  aft_waitlock();
  if (FT_Set_Pixel_Sizes(f->face, 0, f->p)==0){
    aft_unlock();
    return 1;
  }
  aft_unlock();
  return 0;
}

//*
//* Space Width
//*
byte aft_spacewidth(byte isbig){
  return aft_fontwidth(' ',isbig);
}

//*
//* Font Height
//*
byte aft_fontheight(byte isbig){
  if (isbig) return aft_big.h;
  return aft_small.h;
}


//*
//* Load Font
//*
byte aft_loadfont(char * zpath, byte size, byte isbig){
  AFTFONTP f = &aft_small;
  if (isbig!=0) f = &aft_big;
  
  char tmpfile[256];
  snprintf(tmpfile,255,"%s/.font_freetype.%i",AROMA_TMP,isbig);
  
  if ((strcmp(f->zp,zpath)!=0)||(f->init==0)){
    /*
    if (!az_extract(zpath,tmpfile)){
      f->init = 0;
      return 0;
    }
    */
    
    snprintf(f->zp,255,zpath);
  
    //-- Release Font
    aft_closefont(f);
    
    if (!az_readmem(&f->mem,zpath,1)) return 0;
    int error = FT_New_Memory_Face(
                  aft_lib,
                  f->mem.data,
                  f->mem.sz,
                  0,
                  &f->face);
    
    //int error = FT_New_Face( aft_lib, tmpfile, 0, &f->face );
    //int error = FT_New_Face( aft_lib, "/data/arial.ttf", 0, &f->face );
    
    //-- Set Size
    if (error==0){
      f->init = 1;
      aft_setfontsize(size, isbig);
      aft_createglyph(f);
      
      /*
        aft_closeglyph(AFTGLYPHP * c, long num_of_glyph)
        f->fw = malloc(f->face->num_glyphs);
        memset(f->fw,0,f->face->num_glyphs);
      */
      LOGS("Freetype Loaded : %s - %i Glyph\n",zpath,f->face->num_glyphs);
    }
    else{
      LOGS("Freetype Error Load : %s\n",zpath);
      free(f->mem.data);
      f->init = 0;
      snprintf(f->zp,255,"");
      //unlink(tmpfile);
      return 0;
    }
  }
  else if(size!=f->s){
    aft_setfontsize(size, isbig);
    aft_closeglyph(f);
    aft_createglyph(f);
    LOGS("Freetype Reloaded : %s\n",zpath);
  }
  
  return 1;
}

//*
//* Draw Font
//*
byte aft_drawfont(CANVAS * _b, byte isbig, int fpos, int xpos, int ypos, color cl,byte underline,byte bold){
  //-- Is Default Canvas?
  if (_b==NULL) _b=agc();

  //-- Font Type 
  AFTFONTP f = &aft_small;
  if (isbig!=0) f = &aft_big;
  if (f->init==0) return 0;
  
  //-- Get Font Size
  byte   onlock=0;
  AFTGLYPHP ch;
  byte   fw  = aft_fontwidth_lock(fpos,isbig,&ch,&onlock);
  int    fh  = f->h;
  
  if ((fw==0)||(!ch->init)){
    if (onlock) aft_unlock();
    return 0;
  }
  
  if ( ch->g->format != FT_GLYPH_FORMAT_BITMAP ){                                                              
    FT_Glyph_To_Bitmap(&ch->g,FT_RENDER_MODE_NORMAL,0,1);  
  }
  FT_BitmapGlyph  bit = (FT_BitmapGlyph) ch->g;
  int xx, yy;
  int fhalf=ceil(fh/2);
	for (yy=0; yy < bit->bitmap.rows; yy++) {
	  for (xx=0; xx < bit->bitmap.width; xx++) {
      byte a = bit->bitmap.buffer[ (yy * bit->bitmap.pitch) + xx ];
      if (a>0){
        int bx = xpos+bit->left+xx;
        int by = (ypos+yy+fhalf+f->y)-bit->top;
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
	
	if (underline){
	  int usz = ceil(f->p/12);
	  int ux,uy;
	  for (uy=f->p-usz;uy<f->p;uy++){
	    for (ux=0;ux<fw;ux++){
        ag_setpixel(_b,xpos+ux,ypos+uy,cl);
	    }
	  }
  }
  
  if (onlock) aft_unlock();
  return 1;
}