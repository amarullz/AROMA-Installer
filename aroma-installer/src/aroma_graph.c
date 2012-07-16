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
 * Graph, Framebuffer, Color Calculators, Canvas, and Drawings
 *
 */

#include <signal.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <pthread.h>
#include "aroma.h"

/*****************************[ GLOBAL VARIABLES ]*****************************/
int                             ag_fb   = 0;       //-- FrameBuffer Handler
dword                           ag_fbsz = 0;
word*                           ag_fbuf = NULL;    //-- FrameBuffer Direct Memory
byte*                           ag_fbuf32 = NULL;
word*                           ag_b = NULL;       //-- FrameBuffer Cache Memory
dword*                          ag_bf32 = NULL;
word*                           ag_bz = NULL;      //-- FrameBuffer Cache Memory
dword*                          ag_bz32 = NULL;
CANVAS                          ag_c;           //-- FrameBuffer Main Canvas
struct fb_fix_screeninfo        ag_fbf;         //-- FrameBuffer Info
struct fb_var_screeninfo        ag_fbv;
byte                            ag_32;          //-- FrameBuffer Type 32/16bit
pthread_t                       ag_pthread;     //-- FrameBuffer Thread Variables
byte                            ag_isrun;
byte                            ag_16strd;
int                             ag_16w;
PNGFONTS                        AG_SMALL_FONT;  //-- Fonts Variables
PNGFONTS                        AG_BIG_FONT;
int                             ag_dp;          //-- Device Pixel
byte                            agclp;

/****************************[ DECLARED FUNCTIONS ]*****************************/
static void *ag_thread(void *cookie);
void ag_refreshrate();

/*******************[ CALCULATING ALPHA COLOR WITH NEON ]***********************/
dword ag_calchighlight(color c1,color c2){
  color vc1 = ag_calculatealpha(c1,0xffff,40);
  color vc2 = ag_calculatealpha(ag_calculatealpha(c1,c2,110),0xffff,20);
  return MAKEDWORD(vc1,vc2);
}
dword ag_calcpushlight(color c1,color c2){
  color vc1 = ag_calculatealpha(c1,0xffff,20);
  color vc2 = ag_calculatealpha(ag_calculatealpha(c1,c2,100),0xffff,10);
  return MAKEDWORD(vc1,vc2);
}
color ag_calpushad(color c_g){
  byte sg_r = ag_r(c_g);
  byte sg_g = ag_g(c_g);
  byte sg_b = ag_b(c_g);
  sg_r = floor(sg_r*0.6);
  sg_g = floor(sg_g*0.6);
  sg_b = floor(sg_b*0.6);
  return ag_rgb(sg_r,sg_g,sg_b);
}
color ag_calculatecontrast(color c,float intensity){
  return ag_rgb(
        (byte) min(ag_r(c)*intensity,255),
        (byte) min(ag_g(c)*intensity,255),
        (byte) min(ag_b(c)*intensity,255)
  );
}
//-- Calculate 2 Pixel
color ag_calculatealpha(color dcl,color scl,byte l){
  if (scl==dcl) return scl;
  else if (l==0) return dcl;
  else if (l==255) return scl;
  byte  ralpha = 255 - l;
  byte r = (byte) (((((int) ag_r(dcl)) * ralpha) + (((int) ag_r(scl)) * l)) >> 8);
  byte g = (byte) (((((int) ag_g(dcl)) * ralpha) + (((int) ag_g(scl)) * l)) >> 8);
  byte b = (byte) (((((int) ag_b(dcl)) * ralpha) + (((int) ag_b(scl)) * l)) >> 8);
  return ag_rgb(r,g,b);  
}

dword ag_calculatealpha32(dword dcl,dword scl,byte l){
  if (scl==dcl) return scl;
  else if (l==0) return dcl;
  else if (l==255) return scl;
  byte  ralpha = 255 - l;
  byte r = (byte) (((((int) ag_r32(dcl)) * ralpha) + (((int) ag_r32(scl)) * l)) >> 8);
  byte g = (byte) (((((int) ag_g32(dcl)) * ralpha) + (((int) ag_g32(scl)) * l)) >> 8);
  byte b = (byte) (((((int) ag_b32(dcl)) * ralpha) + (((int) ag_b32(scl)) * l)) >> 8);
  return ag_rgb32(r,g,b);
}

dword ag_calculatealphaTo32(color dcl,color scl,byte l){
  if (scl==dcl) return ag_rgbto32(scl);
  else if (l==0) return ag_rgbto32(dcl);
  else if (l==255) return ag_rgbto32(scl);
  byte  ralpha = 255 - l;
  byte r = (byte) (((((int) ag_r(dcl)) * ralpha) + (((int) ag_r(scl)) * l)) >> 8);
  byte g = (byte) (((((int) ag_g(dcl)) * ralpha) + (((int) ag_g(scl)) * l)) >> 8);
  byte b = (byte) (((((int) ag_b(dcl)) * ralpha) + (((int) ag_b(scl)) * l)) >> 8);
  return ag_rgb32(r,g,b);
}

dword ag_calculatealpha16to32(color dcl,dword scl,byte l){
  if (scl==ag_rgbto32(dcl)) return scl;
  else if (l==0) return ag_rgbto32(dcl);
  else if (l==255) return scl;
  byte  ralpha = 255 - l;
  byte r = (byte) (((((int) ag_r(dcl)) * ralpha) + (((int) ag_r32(scl)) * l)) >> 8);
  byte g = (byte) (((((int) ag_g(dcl)) * ralpha) + (((int) ag_g32(scl)) * l)) >> 8);
  byte b = (byte) (((((int) ag_b(dcl)) * ralpha) + (((int) ag_b32(scl)) * l)) >> 8);
  return ag_rgb32(r,g,b);
}
void ag_changecolorspace(int r, int g, int b, int a){
  if (ag_32){
    ag_fbv.red.offset   = r;
    ag_fbv.green.offset = g;
    ag_fbv.blue.offset  = b;
    ag_fbv.transp.offset= a;
    
    ag_blank(NULL); //-- 32bit Use Blank

    int x,y;
    for (y=0;y<ag_fbv.yres;y++){
      int yp = y * ag_fbv.xres;
      int yd = (ag_fbf.line_length*y);
      for (x=0;x<ag_fbv.xres;x++){
        int xy = yp+x;
        int dxy= yd+(x*agclp);
        ag_bf32[xy] = ag_rgb32(
          ag_fbuf32[dxy+(ag_fbv.red.offset>>3)],
          ag_fbuf32[dxy+(ag_fbv.green.offset>>3)],
          ag_fbuf32[dxy+(ag_fbv.blue.offset>>3)]);
        ag_setpixel(&ag_c,x,y,ag_rgbto16(ag_bf32[xy]));
      }
    }
    
  }
}

/*********************************[ FUNCTIONS ]********************************/
//-- INITIALIZING AMARULLZ GRAPHIC
byte ag_init(){
  if (ag_fb>0) return 0;
  
  //-- Open Framebuffer
  ag_fb = open(AROMA_FRAMEBUFFER, O_RDWR, 0);
  
  if (ag_fb>0){
    //-- Init Info from IO
    ioctl(ag_fb, FBIOGET_FSCREENINFO, &ag_fbf);
    ioctl(ag_fb, FBIOGET_VSCREENINFO, &ag_fbv);
    
    //-- Init 32 Buffer
    ag_canvas(&ag_c,ag_fbv.xres,ag_fbv.yres);
    ag_dp = floor( min(ag_fbv.xres,ag_fbv.yres) / 160);
    
    //-- Init Frame Buffer Size
    agclp    = (ag_fbv.bits_per_pixel>>3);
    ag_fbsz  = (ag_fbv.xres * ag_fbv.yres * ((agclp==3)?4:agclp));
    
    //-- Init Frame Buffer
    if (ag_fbv.bits_per_pixel==16){
      ag_32   = 0;
      ag_fbuf = (word*) mmap(0,ag_fbf.smem_len,PROT_READ|PROT_WRITE,MAP_SHARED,ag_fb,0);
      ag_b    = (word*) malloc(ag_fbsz);
      ag_bz   = (word*) malloc(ag_fbsz);
      
      //-- Resolution with Stride
      ag_16strd = 0;
      ag_16w    = ag_fbf.line_length/2;
      if (ag_16w!=ag_fbv.xres){
        if (ag_16w/2==ag_fbv.xres){
          ag_16strd = 0;
          ag_16w    = ag_fbv.xres;
        }
        else{
          ag_16strd=1;
        }
      }
      
      if (ag_16strd==0){
        //-- Can Use memcpy
        memcpy(ag_b,ag_fbuf,ag_fbsz);
        memcpy(ag_c.data,ag_fbuf,ag_fbsz);
      }
      else{
        //-- Should Bit per bit
        int x,y;
        for (y=0;y<ag_fbv.yres;y++){
          int yp = y * ag_fbv.xres;
          int yd = (ag_16w*y);
          for (x=0;x<ag_fbv.xres;x++){
            int xy = yp+x;
            int dxy= yd+x;
            ag_b[xy] = ag_fbuf[dxy];
            ag_setpixel(&ag_c,x,y,ag_b[xy]);
          }
        }
      }
      
      
    }
    else{
      ag_32     = 1;
      
      //-- Memory Allocation
      ag_fbuf32 = (byte*) mmap(0,ag_fbf.smem_len,PROT_READ|PROT_WRITE,MAP_SHARED,ag_fb,0);
      ag_bf32   = (dword*) malloc(ag_fbsz);
      ag_bz32   = (dword*) malloc(ag_fbsz);
      memset(ag_bf32,0,ag_fbsz);
      ag_blank(NULL); //-- 32bit Use Blank

      int x,y;
      for (y=0;y<ag_fbv.yres;y++){
        int yp = y * ag_fbv.xres;
        int yd = (ag_fbf.line_length*y);
        for (x=0;x<ag_fbv.xres;x++){
          int xy = yp+x;
          int dxy= yd+(x*agclp);
          ag_bf32[xy] = ag_rgb32(
            ag_fbuf32[dxy+(ag_fbv.red.offset>>3)],
            ag_fbuf32[dxy+(ag_fbv.green.offset>>3)],
            ag_fbuf32[dxy+(ag_fbv.blue.offset>>3)]);
          ag_setpixel(&ag_c,x,y,ag_rgbto16(ag_bf32[xy]));
        }
      }
    }
    
    //-- Refresh Draw Lock Thread
    ag_isrun = 1;
    pthread_create(&ag_pthread, NULL, ag_thread, NULL);
    
    return 1;
  }
  return 0;
}
void ag_close_thread(){
  ag_isrun=0;
  pthread_join(ag_pthread,NULL);
  pthread_detach(ag_pthread);
}

//-- RELEASE AMARULLZ GRAPHIC
void ag_close(){
  if (ag_fbv.bits_per_pixel!=16){
    if (ag_bf32!=NULL) free(ag_bf32);
    if (ag_bz32!=NULL) free(ag_bz32);
    if (ag_fbuf32!=NULL) munmap(ag_fbuf32,ag_fbsz);
  }
  else if (ag_fbv.bits_per_pixel==16){
    if (ag_b!=NULL) free(ag_b);
    if (ag_bz!=NULL) free(ag_bz);
    if (ag_fbuf!=NULL) munmap(ag_fbuf,ag_fbsz);
  }
  
  //-- Cleanup Canvas & FrameBuffer
  ag_ccanvas(&ag_c);
  close(ag_fb);
  ag_fb = 0;
}

//-- Draw Main Canvas Into FrameBuffer
byte ag_isbusy  = 0;
byte ag_refreshlock=0;
int  ag_busypos = 0;
int  ag_busywinW= 0;
long ag_lastbusy= 0;

//-- Refresh Thread
static void *ag_thread(void *cookie){
  while(ag_isrun){
    if(ag_isbusy!=2){
      usleep(166000);
      if (!ag_isrun) break;
      if (!ag_refreshlock) ag_refreshrate();
    }
    else{
      usleep(16600);
      if (!ag_isrun) break;
      ag_refreshrate();
    }
  }
}


//-- Sync Display
void ag_copybusy(char * wait){
  CANVAS tmpc;
  ag_canvas(&tmpc,agw(),agh());
  ag_draw(&tmpc,&ag_c,0,0);
  ag_rectopa(&tmpc,0,0,agw(),agh(),0x0000,180);
  //char * wait = "Please Wait...";
  int pad     = agdp()*50;
  int txtW    = ag_txtwidth(wait,0);
  int txtH    = ag_fontheight(0);
  int txtX    = (agw()/2)-(txtW/2);
  int txtY    = (agh()/2)-(txtH/2)-(agdp()*2);
  int winH    = txtH+(pad*2);
  int winY    = (agh()/2)-(winH/2);
  int winH2   = winH/2;
  ag_busywinW = agw()/3;
  int i;
  for (i=0;i<winH;i++){
    int alp;
    if (i<winH2)
      alp = ((i*255)/winH2);
    else
      alp = (((winH-i)*255)/winH2);
    alp=min(alp,255);
    ag_rectopa(&tmpc,0,winY+i,agw(),1,0x0000,alp);
  }
  ag_text(&tmpc,txtW,txtX,txtY,wait,0xffff,0);
  
  int bs_x = (agw()/2) - (ag_busywinW/2);
  int bs_y = (agh()/2) + ag_fontheight(0) - (agdp()*2);
  int bs_h = agdp()*2;
  ag_roundgrad(&tmpc,bs_x-3,bs_y-3,ag_busywinW+6,bs_h+6,ag_rgb(140,140,140),ag_rgb(90,90,90),3);
  ag_roundgrad(&tmpc,bs_x-2,bs_y-2,ag_busywinW+4,bs_h+4,0,0,2);
  
  if (ag_32==1){    
    int x,y;
    for (y=0;y<ag_fbv.yres;y++){
      int yp = y * ag_fbv.xres;
      for (x=0;x<ag_fbv.xres;x++){
        int xy  = yp+x;
        color c = tmpc.data[xy];
        ag_bz32[xy] = ag_rgb32(ag_r(c),ag_g(c),ag_b(c));
      }
    }
  }
  else{
    memcpy(ag_bz,tmpc.data,ag_fbsz);
  }
  ag_ccanvas(&tmpc);
}
void ag_setbusy(){
  if (ag_isbusy==0){
    ag_isbusy   = 1;
    ag_lastbusy = alib_tick();
  } 
}
void ag_setbusy_withtext(char * text){
  ag_copybusy(text);
  ag_isbusy=2;
}
void ag_busyprogress(){
  ag_busypos--; //=agdp();
  if (ag_busypos<0) ag_busypos=ag_busywinW;
  int bs_x = (agw()/2) - (ag_busywinW/2);
  int bs_y = (agh()/2) + ag_fontheight(0) - (agdp()*2);
  int bs_h = agdp()*2;
  int bs_w = ag_busywinW;
  int bs_w2= bs_w/2;
  int x,y;
  if (ag_32==1){
    if (agclp==4){      
      for (x=bs_x;x<bs_x+bs_w;x++){
        if ((x+ag_busypos)%(bs_h*2)<bs_h){
          int i=x-bs_x;
          int alp;
          if (i<bs_w2)
            alp = ((i*255)/bs_w2);
          else
            alp = (((bs_w-i)*255)/bs_w2);
          alp=min(alp,255);
          for (y=bs_y;y<bs_y+bs_h;y++){
            int yp = y * ag_fbv.xres;
            int xy  = yp+x;
            int dxy = (ag_fbf.line_length*y)+(x*agclp);
            
            *((dword*) (ag_fbuf32+dxy)) =
              (alp << ag_fbv.red.offset)|
              (alp << ag_fbv.green.offset)|
              (alp << ag_fbv.blue.offset);
          }
        }
      }
    }
    else{
      for (x=bs_x;x<bs_x+bs_w;x++){
        if ((x+ag_busypos)%(bs_h*2)<bs_h){
          int i=x-bs_x;
          int alp;
          if (i<bs_w2)
            alp = ((i*255)/bs_w2);
          else
            alp = (((bs_w-i)*255)/bs_w2);
          alp=min(alp,255);
          for (y=bs_y;y<bs_y+bs_h;y++){
            int yp = y * ag_fbv.xres;
            int xy  = yp+x;
            int dxy = (ag_fbf.line_length*y)+(x*agclp);
            ag_fbuf32[dxy+(ag_fbv.red.offset>>3)]  =alp;
            ag_fbuf32[dxy+(ag_fbv.green.offset>>3)]=alp;
            ag_fbuf32[dxy+(ag_fbv.blue.offset>>3)] =alp;
          }
        }
      }
    }
  }
  else{
    for (x=bs_x;x<bs_x+bs_w;x++){
      if ((x+ag_busypos)%(bs_h*2)<bs_h){
        int i=x-bs_x;
        int alp;
        if (i<bs_w2)
          alp = ((i*255)/bs_w2);
        else
          alp = (((bs_w-i)*255)/bs_w2);
        alp=min(alp,255);
    
        for (y=bs_y;y<bs_y+bs_h;y++){
          int yp = y * ag_16w;
          int xy  = yp+x;
          ag_fbuf[xy]=ag_rgb(alp,alp,alp);
        }
      }
    }
  }
}
void ag32fbufcopy(dword * bfbz){
  int x,y;
  if (agclp==4){
    for (y=0;y<ag_fbv.yres;y++){
      int yp = y * ag_fbv.xres;
      int yd = (ag_fbf.line_length*y);
      for (x=0;x<ag_fbv.xres;x++){
        int xy = yp+x;
        *((dword*) (ag_fbuf32+yd+(x*agclp))) =
            (ag_r32(bfbz[xy]) << ag_fbv.red.offset)|
            (ag_g32(bfbz[xy]) << ag_fbv.green.offset)|
            (ag_b32(bfbz[xy]) << ag_fbv.blue.offset);
      }
    }
  }
  else{
    for (y=0;y<ag_fbv.yres;y++){
      int yp = y * ag_fbv.xres;
      int yd = (ag_fbf.line_length*y);
      for (x=0;x<ag_fbv.xres;x++){
        int xy = yp+x;
        int dxy= yd+(x*agclp);
        ag_fbuf32[dxy+(ag_fbv.red.offset>>3)]   = ag_r32(bfbz[xy]);
        ag_fbuf32[dxy+(ag_fbv.green.offset>>3)] = ag_g32(bfbz[xy]);
        ag_fbuf32[dxy+(ag_fbv.blue.offset>>3)]  = ag_b32(bfbz[xy]);
      }
    }
  }
}/*
void ag16fbufcopy(word * bfbz){
  int x,y;
  for (y=0;y<ag_fbv.yres;y++){
    int yp = y * ag_fbv.xres;
    int yd = (ag_16w*y);
    for (x=0;x<ag_fbv.xres;x++){
      int xy = yp+x;
      int dxy= yd+x;
      ag_fbuf[dxy]=bfbz[xy];
    }
  }
}*/
void ag16fbufcopy(word * bfbz){
  int x,y;
  for (y=0;y<ag_fbv.yres;y++){
    int yp    = y * ag_fbv.xres;
    int ypos  = y * ag_fbf.line_length;
    for (x=0;x<ag_fbv.xres;x++){
      int xy = yp+x;
      int xp = ypos + (x * agclp);
      word * fbf = (word *) (((byte *) ag_fbuf) +xp);
      *fbf = bfbz[xy];
    }
  }
}
void ag_refreshrate(){
  //-- Wait For Draw
  fsync(ag_fb);
  
  //-- Copy Data
  if (ag_32==1){
    if (ag_isbusy==0){
      ag32fbufcopy(ag_bf32);
      //memcpy(ag_fbuf32,ag_bf32,ag_fbsz);
    }
    else if(ag_isbusy==2){
      ag32fbufcopy(ag_bz32);
      //memcpy(ag_fbuf32,ag_bz32,ag_fbsz);
      ag_busyprogress();
    }
    else if(ag_lastbusy<alib_tick()-50){
      ag_copybusy("Please Wait...");
      ag_isbusy=2;
    }
  }
  else{
    if (ag_isbusy==0){
      if (ag_16strd==0){
        //-- Can Use memcpy
        memcpy(ag_fbuf,ag_b,ag_fbsz);
      }
      else{
        ag16fbufcopy(ag_b);
      }
    }
    else if(ag_isbusy==2){
      if (ag_16strd==0){
        //-- Can Use memcpy
        memcpy(ag_fbuf,ag_bz,ag_fbsz);
      }
      else{
        ag16fbufcopy(ag_bz);
      }
      ag_busyprogress();
    }
    else if(ag_lastbusy<alib_tick()-50){
      ag_copybusy("Please Wait...");
      ag_isbusy=2;
    }
  }
  
  //-- Force Refresh Display
  ag_fbv.yoffset   = 0;
  ag_fbv.activate |= FB_ACTIVATE_NOW | FB_ACTIVATE_FORCE;
  ioctl(ag_fb, FBIOPUT_VSCREENINFO, &ag_fbv);  
}

byte ag_sync_locked = 0;
//-- Sync Display
void ag_sync(){
  //-- Always On Footer
  // ag_draw_foot();
  ag_isbusy = 0;
  if (!ag_sync_locked){
    ag_refreshlock=1;
    if (ag_32==1){
      int x,y;
      for (y=0;y<ag_fbv.yres;y++){
        int yp = y * ag_fbv.xres;
        for (x=0;x<ag_fbv.xres;x++){
          int xy  = yp+x;
          color c = ag_c.data[xy];
          ag_bf32[xy] = ag_rgb32(ag_r(c),ag_g(c),ag_b(c));
        }
      }
    }
    else{
      memcpy(ag_b,ag_c.data,ag_fbsz);
    }
    ag_refreshrate();
    ag_refreshlock=0;
  }
}
void ag_sync_force(){
  if (ag_sync_locked)
    ag_sync_locked = 0;
  else
    ag_sync();
}
static void *ag_sync_fade_thread(void * cookie){
  int frame = (int) cookie;
  ag_isbusy = 0;
  ag_sync_locked = 1;
  ag_refreshlock = 1;
  if (ag_32==0){
    int i,x,y;
    for (i=0;(i<(frame/2))&&ag_sync_locked;i++){
      byte perc = (255 / frame) * i;
      byte ralpha = 255 - perc;
      for (y=0;y<agh();y++){
        int yp = y * agw();
        byte er= 0;
        byte eg= 0;
        byte eb= 0;
        for (x=0;x<agw();x++){
          int xy = yp+x;
          color * s = agxy(NULL,x,y);
          color   d = ag_b[xy];
          if (s[0]!=d){
            byte r = min(((byte) (((((int) ag_r(d)) * ralpha) + (((int) ag_r(s[0]))*perc)) >> 8))+er,255);
            byte g = min(((byte) (((((int) ag_g(d)) * ralpha) + (((int) ag_g(s[0]))*perc)) >> 8))+eg,255);
            byte b = min(((byte) (((((int) ag_b(d)) * ralpha) + (((int) ag_b(s[0]))*perc)) >> 8))+eb,255);
            byte nr= ag_close_r(r);
            byte ng= ag_close_g(g);
            byte nb= ag_close_b(b);
            ag_b[xy] = ag_rgb(nr,ng,nb);
            er     = r-nr;
            eg     = g-ng;
            eb     = b-nb;
          }else{
            er= 0;
            eg= 0;
            eb= 0;
          }
        }
      }
      ag_refreshrate();
    }
  }
  else{
    int i,x,y;
    for (i=0;(i<(frame/2))&&(ag_sync_locked);i++){
      int perc = (255 / frame) * i;
      for (y=0;y<agh();y++){
        int yp = y * agw();
        for (x=0;x<agw();x++){
          int xy = yp+x;
          color * s = agxy(NULL,x,y);
          dword   d = ag_bf32[xy];
          ag_bf32[xy]  = ag_calculatealpha16to32(s[0],d,255-perc);
        }
      }
      ag_refreshrate();
    }
  }
  ag_refreshlock = 0;
  ag_sync_locked = 0;
  ag_sync();  
}
void ag_sync_fade_wait(int frame){
  ag_sync_fade_thread((void *) frame);
}
void ag_sync_fade(int frame){
  pthread_t threadsyncfade;
  pthread_create(&threadsyncfade,NULL, ag_sync_fade_thread, (void *) frame);
  pthread_detach(threadsyncfade);
}
byte ag_blur_h(CANVAS * d,CANVAS * s, int radius){
  if (radius<1) return 0;
  if (s==NULL) return 0;
  if (d==NULL) d=&ag_c;
  
  int x, y, k;
  int rad=radius*2;
  int radd=rad+1;
  for (y=0;y<s->h;y++){
    dword r = 0; dword g = 0; dword b = 0;    
    for (k=0;(k<=radius)&&(k<s->w);k++){
      color * cl = agxy(s,k,y);
      if (cl!=NULL){
        r += ag_r(cl[0]); g += ag_g(cl[0]); b += ag_b(cl[0]);
      }
    }
    
    //-- Dither Engine
    float vr = r/radd;
    float vg = g/radd;
    float vb = b/radd;
    byte  nr = ag_close_r(round(vr));
    byte  ng = ag_close_g(round(vg));
    byte  nb = ag_close_b(round(vb));
    float er = vr-nr;
    float eg = vg-ng;
    float eb = vb-nb;
    
    //-- Save
    ag_setpixel(d,0,y,ag_rgb(nr,ng,nb));
    
    
    for (x=1;x<s->w;x++){
      if (x>radius){
        color * cl = agxy(s,x-radius-1,y);
        r -= ag_r(cl[0]); g -= ag_g(cl[0]); b -= ag_b(cl[0]);
      }
      if (x<s->w-(radius+1)){
        color * cl = agxy(s,x+radius,y);
        r += ag_r(cl[0]); g += ag_g(cl[0]); b += ag_b(cl[0]);
      }
      
      //-- Dither Engine
      vr = min((r/radd)+er,255);
      vg = min((g/radd)+eg,255);
      vb = min((b/radd)+eb,255);
      nr = ag_close_r(round(vr));
      ng = ag_close_g(round(vg));
      nb = ag_close_b(round(vb));
      er = vr-nr;
      eg = vg-ng;
      eb = vb-nb;
      
      //-- Save
      ag_setpixel(d,x,y,ag_rgb(nr,ng,nb));
    }
  }
  return 1;
}
byte ag_blur_v(CANVAS * d,CANVAS * s, int radius){
  if (radius<1) return 0;
  if (s==NULL) return 0;
  if (d==NULL) d=&ag_c;
  
  int x, y, k;
  int rad=radius*2;
  int radd=rad+1;
  
  for (x=0;x<s->w;x++){
    dword r = 0; dword g = 0; dword b = 0;
    for (k=0;(k<=radius)&&(k<s->h);k++){
      color * cl = agxy(s,x,k);
      if (cl!=NULL){
        r += ag_r(cl[0]); g += ag_g(cl[0]); b += ag_b(cl[0]);
      }
    }
    
    //-- Dither Engine
    float vr = r/radd;
    float vg = g/radd;
    float vb = b/radd;
    byte  nr = ag_close_r(round(vr));
    byte  ng = ag_close_g(round(vg));
    byte  nb = ag_close_b(round(vb));
    float er = vr-nr;
    float eg = vg-ng;
    float eb = vb-nb;
    //-- Save
    ag_setpixel(d,x,0,ag_rgb(nr,ng,nb));
    
    for (y=1;y<s->h;y++){
      if (y>radius){
        color * cl = agxy(s,x,y-radius-1);
        r -= ag_r(cl[0]); g -= ag_g(cl[0]); b -= ag_b(cl[0]);
      }
      if (y<s->h-(radius+1)){
        color * cl = agxy(s,x,y+radius);
        r += ag_r(cl[0]); g += ag_g(cl[0]); b += ag_b(cl[0]);
      }
      
      //-- Dither Engine
      vr = min((r/radd)+er,255);
      vg = min((g/radd)+eg,255);
      vb = min((b/radd)+eb,255);
      nr = ag_close_r(round(vr));
      ng = ag_close_g(round(vg));
      nb = ag_close_b(round(vb));
      er = vr-nr;
      eg = vg-ng;
      eb = vb-nb;
      
      //--Save
      ag_setpixel(d,x,y,ag_rgb(nr,ng,nb));
    }
  }
  return 1;
}
byte ag_blur(CANVAS * d,CANVAS * s, int radius){
  if (radius<1) return 0;
  CANVAS tmp;
  ag_canvas(&tmp,s->w,s->h);
  ag_blur_h(&tmp,s,radius);
  ag_blur_v(d,&tmp,radius);
  ag_ccanvas(&tmp);
  return 1;
}
//-- CREATE CANVAS
void ag_canvas(CANVAS * c,int w,int h){
  c->w      = w;
  c->h      = h;
  c->sz     = (w*h*2);
  c->data   = (color *) malloc(c->sz);
  memset(c->data,0,c->sz);
}

//-- RELEASE CANVAS
void ag_ccanvas(CANVAS * c){
  if (c->data) free(c->data);
  c->data=NULL;
}

//-- Get Main Canvas
CANVAS * agc(){
  return &ag_c;
}

//-- Clear Canvas
void ag_blank(CANVAS * c){
  if (c==NULL) c=&ag_c;
  memset(c->data,0,c->sz);
}

//-- Width
int agw(){
  return ag_fbv.xres;
}

//-- Height
int agh(){
  return ag_fbv.yres;
}

int agdp(){
  return ag_dp;
}

//-- Convert String to Color
color strtocolor(char * c){
  if (c[0]!='#') return 0;
  char out[9]={'0','x'};
  int  i;
  if (strlen(c)==7){
    for (i=1;i<7;i++){
      out[i+1]=c[i];
    }
  }
  else if (strlen(c)==4){
    for (i=0;i<3;i++){
      out[(i*2)+2]=c[i+1];
      out[(i*2)+3]=c[i+1];
    }
  }
  else
    return 0;
  out[8]=0;
  dword ul = strtoul(out,NULL,0);
  return ag_rgb(ag_b32(ul),ag_g32(ul),ag_r32(ul));
}

//-- Draw Canvas To Canvas Extra
byte ag_draw_ex(CANVAS * dc,CANVAS * sc, int dx, int dy, int sx, int sy, int sw, int sh){
  if (sc==NULL) return 0;
  if (dc==NULL) dc=&ag_c;
  if (dx>=dc->w) return 0;
  if (dy>=dc->h) return 0;
  if (sx<0){
    dx+=abs(sx);
    sw-=abs(sx);
    sx=0;
  }
  if (sy<0){
    dy+=abs(sy);
    sh-=abs(sy);
    sy=0;
  }
  if (sw+sx>=sc->w) sw-=(sw+sx) - sc->w;
  if (sh+sy>=sc->h) sh-=(sh+sy) - sc->h;
  if ((sw<=0)||(sh<=0)) return 0;
  int sr_w = sw;
  int sr_h = sh;
  int sr_x = sx;
  int sr_y = sy;
  int ds_x = dx;
  int ds_y = dy;
  if (dx<0){
    int ndx = abs(dx);
    sr_x+= abs(ndx);
    sr_w-= ndx;
    ds_x = 0;
  }
  if (dy<0){
    int ndy = abs(dy);
    sr_y+= ndy;
    sr_h-= ndy;
    ds_y = 0;
  }
  if (sr_w+dx>dc->w) sr_w-=(sr_w+dx) - dc->w;
  if (sr_h+dy>dc->h) sr_h-=(sr_h+dy) - dc->h;
  int y;
  int pos_sr_x = sr_x*2;
  int pos_ds_x = ds_x*2;
  int pos_sc_w = sc->w*2;
  int pos_dc_w = dc->w*2;
  int copy_sz  = sr_w*2;
  byte * src   = ((byte *) sc->data);
  byte * dst   = ((byte *) dc->data);
  for (y=0;y<sr_h;y++){
    memcpy(
      dst + ((ds_y+y)*pos_dc_w)+pos_ds_x,
      src + ((sr_y+y)*pos_sc_w)+pos_sr_x,
      copy_sz
    );
  }
  return 1;
}

//-- Draw Canvas To Canvas
byte ag_draw(CANVAS * dc,CANVAS * sc,int dx, int dy){
  if (sc==NULL) return 0;
  return ag_draw_ex(dc,sc,dx,dy,0,0,sc->w,sc->h);
}

//-- Pixel
color * agxy(CANVAS *_b, int x, int y){
  if (_b==NULL) _b=&ag_c;
  if ((x<0)||(y<0)) return NULL;
  if ((x>=_b->w)||(y>=_b->h)) return NULL;
  return _b->data + ((y * _b->w) + x);
}

//-- SetPixel
byte ag_setpixel(CANVAS *_b,int x, int y,color cl){
  color * c = agxy(_b,x,y);
  if (c==NULL) return 0;
  c[0]=cl;
  return 1;
}

byte ag_spixel(CANVAS *_b,float x, float y, color cl){
  if (_b==NULL) _b=&ag_c;
  int fx=floor(x);
  int fy=floor(y);
  float ax=x-fx;
  float ay=y-fy;
  float sz=ax+ay;
  if (sz==0)
    return ag_setpixel(_b,fx,fy,cl);
  ag_subpixel(_b, fx    ,fy,   cl,  (byte) ((((1-ax)+(1-ay))  * 255) / 4));
  ag_subpixel(_b, fx+1  ,fy,   cl,  (byte) (((ax+(1-ay))      * 255) / 4));
  ag_subpixel(_b, fx    ,fy+1, cl,  (byte) ((((1-ax)+ay)      * 255) / 4));
  ag_subpixel(_b, fx+1  ,fy+1, cl,  (byte) (((ax+ay)          * 255) / 4));
}

//-- SubPixel
byte ag_subpixel(CANVAS *_b,int x, int y, color cl,byte l){
  if (_b==NULL) _b=&ag_c;
  if (l>=255) return ag_setpixel(_b,x,y,cl);
  if (l<=0) return 1;
  
  color * c = agxy(_b,x,y);
  if (c==NULL) return 0;
  c[0]   =  ag_calculatealpha(c[0],cl,l);
  return 1;
}

//-- SubPixelGet
color ag_subpixelget(CANVAS *_b,int x, int y, color cl,byte l){
  if (_b==NULL) _b=&ag_c;
  if (l>=255) return cl;
  color * c = agxy(_b,x,y);
  if (c==NULL) return 0;
  return ag_calculatealpha(c[0],cl,l);
}
//-- SubPixelGet32
dword ag_subpixelget32(CANVAS *_b,int x, int y, dword cl,byte l){
  if (_b==NULL) _b=&ag_c;
  if (l>=255) return cl;
  color * c = agxy(_b,x,y);
  if (c==NULL) return 0;
    
  return ag_calculatealpha16to32(c[0],cl,l);
}
//-- Draw Rectangle
byte ag_rect(CANVAS *_b,int x, int y, int w, int h, color cl){
  if (_b==NULL) _b=&ag_c;
    
  //-- FIXING
  int x2 = x+w; if (x2>_b->w) x2=_b->w; 
  int y2 = y+h; if (y2>_b->h) y2=_b->h; 
  if (x<0) x=0; if (y<0) y=0;
  w=x2-x; h=y2-y;
  
  //-- LOOPS
  int xx, yy;
  for (yy=y;yy<y2;yy++){
    int i = yy * _b->w;
    for (xx=x;xx<x2;xx++){
      _b->data[i + xx] = cl;
    }
  }
  
  return 1;
}
//-- Draw Rectangle
byte ag_rectopa(CANVAS *_b,int x, int y, int w, int h, color cl,byte l){
  if (_b==NULL) _b=&ag_c;
    
  //-- FIXING
  int x2 = x+w; if (x2>_b->w) x2=_b->w; 
  int y2 = y+h; if (y2>_b->h) y2=_b->h; 
  if (x<0) x=0; if (y<0) y=0;
  w=x2-x; h=y2-y;
  
  byte ll = 255-l;
  int sr  = ag_r(cl);
  int sg  = ag_g(cl);
  int sb  = ag_b(cl);
  
  //-- LOOPS
  int xx, yy;
  for (yy=y;yy<y2;yy++){
    byte er= 0;
    byte eg= 0;
    byte eb= 0;
    for (xx=x;xx<x2;xx++){
      color * cv = agxy(_b,xx,yy);
      if (cv[0]!=cl){
        byte  ralpha = 255 - l;
        byte r = min(((byte) (((((int) ag_r(cv[0])) * ll) + (sr*l)) >> 8))+er,255);
        byte g = min(((byte) (((((int) ag_g(cv[0])) * ll) + (sg*l)) >> 8))+eg,255);
        byte b = min(((byte) (((((int) ag_b(cv[0])) * ll) + (sb*l)) >> 8))+eb,255);
        byte nr= ag_close_r(r);
        byte ng= ag_close_g(g);
        byte nb= ag_close_b(b);
        er     = r-nr;
        eg     = g-ng;
        eb     = b-nb;
        cv[0]  = ag_rgb(nr,ng,nb);
      }else{
        er= 0;
        eg= 0;
        eb= 0;
      }
    }
  }
  
  return 1;
}
//-- Draw Rounded Gradient Rectangle
#define ag_rndsave(a,b,c) a=min( a+((byte) (((b+c)  * 255) / 4)) , 255)
byte ag_roundgrad(CANVAS *_b,int x, int y, int w, int h, color cl1, color cl2, int roundsz){
  return ag_roundgrad_ex(_b,x,y,w,h,cl1,cl2,roundsz,1,1,1,1);
}
byte ag_roundgrad_ex(CANVAS *_b,int x, int y, int w, int h, color cl1, color cl2, int roundsz, byte tlr, byte trr, byte blr, byte brr){
  if (_b==NULL) _b=&ag_c;
  if ((tlr==2)||(trr==2)||(blr==2)||(brr==2)){
    if (tlr==2) tlr==1;
    if (trr==2) trr==1;
    if (blr==2) blr==1;
    if (brr==2) brr==1;
  }
  else{
    if (roundsz>h/2) roundsz=h/2;
    if (roundsz>w/2) roundsz=w/2;
  }
  
  if (roundsz<0) roundsz=0;

  //-- ANTIALIAS ROUNDED
  int rndsz;
  byte * rndata;
  if (roundsz>0){
    rndsz  = roundsz*roundsz;
    rndata = malloc(rndsz);
    memset(rndata,0,rndsz);
    float inc = 180;
    float incz= 40/roundsz;
    if (roundsz>40) incz=1;
    while (inc<=270){
      float rd  = (inc * M_PI / 180);
      float xp  = roundsz+(sin(rd)*roundsz); // X Axis
      float yp  = roundsz+(cos(rd)*roundsz); // Y Axis
      int fx    = floor(xp);
      int fy    = floor(yp);
      float ax  = xp-fx;
      float ay  = yp-fy;
      float sz  = ax+ay;
      if ((fx>=0)&&(fy>=0)&&(fx<roundsz)&&(fy<roundsz)){
        ag_rndsave(rndata[fx+fy*roundsz],1-ax,1-ay);
        if (fx<roundsz-1) ag_rndsave(rndata[fx+1+fy*roundsz],ax,1-ay);
        if (fy<roundsz-1) ag_rndsave(rndata[fx+(1+fy)*roundsz],1-ax,ay);
        if ((fx<roundsz-1)&&(fy<roundsz-1)) ag_rndsave(rndata[(fx+1)+(1+fy)*roundsz],ax,ay);
      }
      inc += incz;
    }
    int rndx, rndy;
    for (rndy=0;rndy<roundsz;rndy++){
      byte alpy=0;
      byte alpf=0;
      for (rndx=0;rndx<roundsz;rndx++){
        byte alpx=rndata[rndx+rndy*roundsz];
        if ((alpy<alpx)&&(!alpf)) alpy=alpx;
        else if (alpf||(alpy>alpx)){
          alpf=1;
          rndata[rndx+rndy*roundsz]=255;
        }
      }
    }
  }
  
  //-- FIXING
  int x2 = x+w;
  int y2 = y+h;
  /*int x2 = x+w; if (x2>_b->w) x2=_b->w; 
  int y2 = y+h; if (y2>_b->h) y2=_b->h; 
  if (x<0) x=0; if (y<0) y=0;
  w=x2-x; h=y2-y;*/
  
  //-- QUARTZ ERRORS BUFFER
  int xx,yy;
  int qz      = w * h * 3;
  byte * qe   = (byte*) malloc(qz);
  memset(qe,0,qz);
  
  //-- LOOPS
  for (yy=y;yy<y2;yy++){
    //-- Vertical Pos
    int z   = yy * _b->w;
    //int zq  = (yy-y) * w;
    
    //-- Calculate Row Color
    byte falpha = (byte) min((((float) 255/h) * (yy-y)),255);
    dword linecolor = ag_calculatealphaTo32(cl1,cl2,falpha);
    byte r = ag_r32(linecolor);
    byte g = ag_g32(linecolor);
    byte b = ag_b32(linecolor);
    
    for (xx=x;xx<x2;xx++){
      int qx = (((yy-y)) * w + (xx-x)) * 3;
      //int xy = z+xx;
      
      color * dx = agxy(_b,xx,yy);
      if (dx!=NULL){
        int absy = yy-y;
        dword curpix=ag_rgb32(r,g,b);
        if (roundsz>0){
          // tlr, trr, blr, brr //
          if ((tlr)&&(xx-x<roundsz)&&(absy<roundsz)){
            int absx = xx-x;
            curpix=ag_subpixelget32(_b,xx,yy,curpix,rndata[absy*roundsz+absx]);
          }
          else if ((trr)&&(xx>=(w+x)-roundsz)&&(absy<roundsz)){
            int absx = roundsz-((xx+roundsz)-(x+w))-1;
            curpix=ag_subpixelget32(_b,xx,yy,curpix,rndata[absy*roundsz+absx]);
          }
          else if ((blr)&&(xx-x<roundsz)&&(yy>=(h+y)-roundsz)){
            int absx = xx-x;
            int abyy = roundsz-((yy+roundsz)-(y+h))-1;
            curpix=ag_subpixelget32(_b,xx,yy,curpix,rndata[abyy*roundsz+absx]);
          }
          else if ((brr)&&(xx>=(w+x)-roundsz)&&(yy>=(h+y)-roundsz)){
            int absx = roundsz-((xx+roundsz)-(x+w))-1;
            int abyy = roundsz-((yy+roundsz)-(y+h))-1;
            curpix=ag_subpixelget32(_b,xx,yy,curpix,rndata[abyy*roundsz+absx]);
          }
        }
        
        //-- Amarullz Dithering
        byte old_r = (byte) min(((int) ag_r32(curpix)) + ((int) qe[qx]),  255);
        byte old_g = (byte) min(((int) ag_g32(curpix)) + ((int) qe[qx+1]),255);
        byte old_b = (byte) min(((int) ag_b32(curpix)) + ((int) qe[qx+2]),255);
        byte new_r = ag_close_r(old_r);
        byte new_g = ag_close_g(old_g);
        byte new_b = ag_close_b(old_b);
        byte err_r = old_r - new_r;
        byte err_g = old_g - new_g;
        byte err_b = old_b - new_b;
        
        if (xx-x<w-1) qe[qx+4] += err_g; // Save Green QE
        if (yy-y<h-1){
          qx = ((yy-y+1) * w + (xx-x)) * 3;
          qe[qx] += err_r; // Save Red QE
          if (xx-x<w-1) qe[qx+5] += err_b; // Save Blue QE
        }
      
        dx[0] = ag_rgb( ((byte) new_r), ((byte) new_g), ((byte) new_b) );
        // _b->data[xy] = ag_rgb( ((byte) new_r), ((byte) new_g), ((byte) new_b) );
      }
    }
  }
  if (roundsz>0) free (rndata);
  free (qe);
  return 1;
}

/******************************[ FONT FUNCTIONS ]******************************/
//-- Load Small Font
byte ag_loadsmallfont(char * fontname){
  apng_closefont(&AG_SMALL_FONT);
  return apng_loadfont(&AG_SMALL_FONT,fontname);
}
//-- Load Big Font
byte ag_loadbigfont(char * fontname){
  apng_closefont(&AG_BIG_FONT);
  return apng_loadfont(&AG_BIG_FONT,fontname);
}
void ag_closefonts(){
  apng_closefont(&AG_BIG_FONT);
  apng_closefont(&AG_SMALL_FONT);
}
//-- Draw Character
byte ag_drawchar_ex(CANVAS *_b,int x, int y, char c, color cl, byte isbig, byte underline, byte bold){
  if (_b==NULL) _b=&ag_c;
  int yy,xx;
  y++;
  int cd = ((int) c)-32;
  if (cd<0) return 0;
  if (cd==137) cd = 95;  
  if (cd>95) return 0;
  PNGFONTS * fnt = isbig?&AG_BIG_FONT:&AG_SMALL_FONT;
  return apng_drawfont(_b,fnt,cd,x,y,cl,underline,bold);
}
byte ag_drawchar(CANVAS *_b,int x, int y, char c, color cl, byte isbig){
  return ag_drawchar_ex(_b,x, y, c, cl, isbig,0,0);
}
//-- Calculate Font Width
byte ag_fontwidth(char c,byte isbig){
  PNGFONTS * fnt = isbig?&AG_BIG_FONT:&AG_SMALL_FONT;
  int cd = ((int) c)-32;
  if (cd<0) return 0;
  if (cd==137) cd = 95;  
  if (cd>95) return 0;
  return fnt->fw[cd];
}
int ag_tabwidth(int x, byte isbig){
  PNGFONTS * fnt = isbig?&AG_BIG_FONT:&AG_SMALL_FONT;
  int spacesz = fnt->fw[0]*8;
  return (spacesz-(x%spacesz));
}

//-- Colorset
static char ag_colorsets[28][14]={
  "#winbg",
  "#winbg_g",
  "#winfg",
  "#winfg_gray",
  "#dialogbg",
  "#dialogbg_g",
  "#dialogfg",
  "#textbg",
  "#textfg",
  "#textfg_gray",
  "#controlbg",
  "#controlbg_g",
  "#controlfg",
  "#selectbg",
  "#selectbg_g",
  "#selectfg",
  "#titlebg",
  "#titlebg_g",
  "#titlefg",
  "#dlgtitlebg",
  "#dlgtitlebg_g",
  "#dlgtitlefg",
  "#scrollbar",
  "#navbg",
  "#navbg_g",
  "#border",
  "#border_g",
  "#progressglow"
};
//-- get Color By Index
color ag_getcolorset(int color_index){
  color cl=0;
  switch(color_index){
    case 0:  cl=acfg()->winbg; break;
    case 1:  cl=acfg()->winbg_g; break;
    case 2:  cl=acfg()->winfg; break;
    case 3:  cl=acfg()->winfg_gray; break;
    case 4:  cl=acfg()->dialogbg; break;
    case 5:  cl=acfg()->dialogbg_g; break;
    case 6:  cl=acfg()->dialogfg; break;
    case 7:  cl=acfg()->textbg; break;
    case 8:  cl=acfg()->textfg; break;
    case 9:  cl=acfg()->textfg_gray; break;
    case 10: cl=acfg()->controlbg; break;
    case 11: cl=acfg()->controlbg_g; break;
    case 12: cl=acfg()->controlfg; break;
    case 13: cl=acfg()->selectbg; break;
    case 14: cl=acfg()->selectbg_g; break;
    case 15: cl=acfg()->selectfg; break;
    case 16: cl=acfg()->titlebg; break;
    case 17: cl=acfg()->titlebg_g; break;
    case 18: cl=acfg()->titlefg; break;
    case 19: cl=acfg()->dlgtitlebg; break;
    case 20: cl=acfg()->dlgtitlebg_g; break;
    case 21: cl=acfg()->dlgtitlefg; break;
    case 22: cl=acfg()->scrollbar; break;
    case 23: cl=acfg()->navbg; break;
    case 24: cl=acfg()->navbg_g; break;
    case 25: cl=acfg()->border; break;
    case 26: cl=acfg()->border_g; break;
    case 27: cl=acfg()->progressglow; break;
  };
  return cl;
}
byte ag_check_escape(char * soff, const char ** ssource, char * buf, byte realescape, byte * o){
  const char * s = *ssource;
  char off = *soff;
  int  i=0;
  char tb[15];
  
  if ((off=='\\')&&(*s=='<')){ *soff = *s++; *ssource=s; if (o!=NULL) *o=1; }
  else if ((off=='<')&&((*s=='u')||(*s=='b')||(*s=='q')||(*s=='*')||(*s=='@')||(*s=='#')||(*s=='/'))){
    const char * sv = s;
    memset(tb,0,15);
    byte foundlt = 0;
    for (i=0;i<15;i++){
      char cv=*sv++;
      if (cv=='>'){
        tb[i]   = 0;
        foundlt = 1;
        break;
      }
      tb[i]=cv;
    }
    if (foundlt){
      if (tb[0]=='#'){
        int ci=0;
        for (ci=0;ci<28;ci++){
          if (strcmp(tb,ag_colorsets[ci])==0){
            if (buf!=NULL){
              if (realescape){
                snprintf(buf,15,tb);
              }
              else{
                color ccolor=ag_getcolorset(ci);
                snprintf(buf,8,"#%02x%02x%02x",ag_r(ccolor),ag_g(ccolor),ag_b(ccolor));
              }
            }
            *ssource=sv;
            return 1;
          }
        }
      }
      
      if (
          (strcmp(tb,"u")==0)||
          (strcmp(tb,"/u")==0)||
          (strcmp(tb,"b")==0)||
          (strcmp(tb,"/b")==0)||
          (strcmp(tb,"q")==0)||
          (strcmp(tb,"/q")==0)||
          (strcmp(tb,"*")==0)||
          (strcmp(tb,"/*")==0)||
          (strcmp(tb,"/#")==0)||
          (strcmp(tb,"/@")==0)||
          
          //-- ALIGN
          (strcmp(tb,"@left")==0)||
          (strcmp(tb,"@right")==0)||
          (strcmp(tb,"@center")==0)||
          (strcmp(tb,"@fill")==0)||
          
          ((tb[0]=='#') && ((strlen(tb)==4)||(strlen(tb)==7)))
      ){
        if (buf!=NULL) sprintf(buf,"%s",tb);
        *ssource=sv;
        return 1;
      }
    }
  }
  return 0;
}
//-- Calculate 1 Line Text Width
int ag_txtwidth(const char *s, byte isbig){
  int w = 0;
  int x = 0;
  int  i=0;
  char tb[8];
  char off;
  while((off = *s++)){
    if (ag_check_escape(&off,&s,NULL,1,NULL)) continue;
    if (off=='\t')
      w+=ag_tabwidth(w,isbig);
    else
      w+=ag_fontwidth(off,isbig);
  }    
  return w;
}
int ag_fontheight(byte isbig){
  PNGFONTS * fnt = isbig?&AG_BIG_FONT:&AG_SMALL_FONT;
  return fnt->fh;
}
//-- Draw Text
byte ag_text(CANVAS *_b,int maxwidth,int x,int y, const char *s, color cl_def,byte isbig){
  return ag_text_ex(_b,maxwidth,x,y,s,cl_def,isbig,0);
}
byte ag_textf(CANVAS *_b,int maxwidth,int x,int y, const char *s, color cl_def,byte isbig){
  return ag_text_ex(_b,maxwidth,x,y,s,cl_def,isbig,1);
}
byte ag_text_ex(CANVAS *_b,int maxwidth,int x,int y, const char *s, color cl_def,byte isbig,byte forcecolor){
  return ag_text_exl(_b,maxwidth,x,y,s,cl_def,isbig,forcecolor,1);
}
byte ag_texts(CANVAS *_b,int maxwidth,int x,int y, const char *s, color cl_def,byte isbig){
  return ag_text_exl(_b,maxwidth,x,y,s,cl_def,isbig,0,0);
}
byte ag_textfs(CANVAS *_b,int maxwidth,int x,int y, const char *s, color cl_def,byte isbig){
  return ag_text_exl(_b,maxwidth,x,y,s,cl_def,isbig,1,0);
}

//############################ NEW TEXT HANDLER
int ag_txt_getline(const char * s, int maxwidth_ori, byte isbig, byte * ischangealign, int * indent, int * next_indent, byte * endofstring){
  char tb[15];//-- Escape Data
  char c=0;   //-- Current Char
  byte o=0;   //-- Previous Char
  int  l=0;   //-- Line String Length
  int  w=0;   //-- Current Width
  int  p=-1;  //-- Previous Space Pos
  int  maxwidth = maxwidth_ori - indent[0];
  int  indentsz = ag_fontwidth(' ',isbig)+ag_fontwidth(0xa9,isbig);
  byte fns=0; //-- No Space Exists
  while ((c=*s++)){
    if (ag_check_escape(&c,&s,tb,1,&o)){
      if (w>0){
        if  (
              (strcmp(tb,"/@")==0)||
              (strcmp(tb,"@left")==0)||
              (strcmp(tb,"@right")==0)||
              (strcmp(tb,"@center")==0)||
              (strcmp(tb,"@fill")==0)
            ){
          if (ischangealign!=NULL) ischangealign[0]=1;
          if (*s=='\n') return (l+3+strlen(tb));
          return l;
        }
        else if ((strcmp(tb,"/q")==0)||(strcmp(tb,"/*")==0)){
          next_indent[0]=indent[0]-indentsz;
          if (next_indent[0]<0) next_indent[0] = 0;
          
          if (fns){
            if (ischangealign!=NULL) ischangealign[0]=1;
            if (*s=='\n') return (l+3+strlen(tb));
            return l;
          }
          else{
            indent[0]=next_indent[0];
            maxwidth = maxwidth_ori - indent[0];
          }
        }
        else if ((strcmp(tb,"q")==0)||(strcmp(tb,"*")==0)) {
          next_indent[0]=indent[0]+indentsz;
          if (next_indent[0]>indentsz*5) next_indent[0] = indentsz*5;
          
          if (fns){
            if (ischangealign!=NULL) ischangealign[0]=1;
            if (*s=='\n') return (l+3+strlen(tb));
            return l;
          }
          else{
            indent[0]=next_indent[0];
            maxwidth = maxwidth_ori - indent[0];
          }
        }
      }
      else if ((strcmp(tb,"/q")==0)||(strcmp(tb,"/*")==0)){
        w=0;
        indent[0]-=indentsz;
        if (indent[0]<0) indent[0] = 0;
        next_indent[0]=indent[0];
        maxwidth = maxwidth_ori - indent[0];
      }
      else if ((strcmp(tb,"q")==0)||(strcmp(tb,"*")==0)){
        w=0;
        indent[0]+=indentsz;
        if (indent[0]>indentsz*5) indent[0] = indentsz*5;
        next_indent[0]=indent[0];
        maxwidth = maxwidth_ori - indent[0];
      }
      l+=2+strlen(tb);
      p=l;
    }
    else{
      if (c=='\n'){
        if (ischangealign!=NULL) ischangealign[0]=1;
        return l+1;
      }
      else if (c=='\t')
        w+=ag_tabwidth(w,isbig);
      else
        w+=ag_fontwidth(c,isbig);
      
      if (w>maxwidth){
        if (p==-1)
          return l;
        return p;
      }
      else if ((c==' ')||(c=='\t')){
        l++;
        p=l;
      }
      else if (c=='<'){
        l++;
        if (o) l++;
        fns=1;
      }
      else{
        l++;
        fns=1;
      }
    }
    o = 0;
  }
  endofstring[0]=1;
  return l;
}
char * ag_substring(const char * s, int len){
  if (len<1) return NULL;

  char * ln = malloc(len+1);
  memset(ln,0,len+1);
  
  int i;
  for (i=0;i<len;i++){
    if (s[i]=='\n') break;
    ln[i]=s[i];
  }
  return ln;
}
int ag_txtheight(int maxwidth, const char *s, byte isbig){
  PNGFONTS * fnt = isbig?&AG_BIG_FONT:&AG_SMALL_FONT;
  if (!fnt->loaded) return 0;
  int  fheight = fnt->fh;
  
  int indent= 0;
  int lines = 0;
  while (*s!=0){
    int next_indent = indent;
    byte eos = 0;
    int line_width  = ag_txt_getline(s,maxwidth,isbig,NULL,&indent,&next_indent,&eos);
    if (line_width==0) break;
    lines++;
    s+=line_width;
    indent=next_indent;
    if (eos) break;
  }
  return (lines*fheight);
}

byte ag_text_exl(CANVAS *_b,int maxwidth,int x,int y, const char *s, color cl_def,byte isbig,byte forcecolor,byte multiline){
  PNGFONTS * fnt = isbig?&AG_BIG_FONT:&AG_SMALL_FONT;
  if (!fnt->loaded) return 0;
  if (_b==NULL) _b=&ag_c;
  if (!maxwidth) maxwidth = _b->w-x;

  int  fheight = fnt->fh;
  
  char tb[8];         //-- Escape Data
  byte bold = 0;      //-- Bold
  byte undr = 0;      //-- Underline
  byte algn = 0;      //-- Alignment
  color cl  = cl_def; //-- Current Color
  int  cx   = x;
  int indent= 0;
  while (*s!=0){
    byte chalign   = 0;
    int next_indent= indent;
    byte eos = 0;
    int line_width = ag_txt_getline(s,maxwidth,isbig,&chalign,&indent,&next_indent,&eos);
    if (line_width==0) break;
    
    char * bf=ag_substring(s,line_width);
    if (bf!=NULL){
      const char * line_string  = ai_rtrim(bf);
      int lwpx                  = ag_txtwidth(line_string,isbig);
      int ldpx                  = (maxwidth-indent)-lwpx;
      char off                  = 0;
      
      //-- Alignment
      if (algn==1)
        cx=ldpx/2 + x + indent;
      else if (algn==2)
        cx=ldpx + x + indent;
      else
        cx=x + indent;
      
      int first_cx = cx;
      
      int sp_n    = 0;    //-- space count
      int * sp_v  = NULL; //-- space add sz
      if (chalign==0){
        if (algn==3){
          sp_n=0;
          char vc = 0;
          byte vf =0;
          const char * lstr = line_string;
          while((vc = *lstr++)){
            if (!ag_check_escape(&vc,&lstr,NULL,1,NULL)){
              if (vc=='\t'){
                sp_n = 0;
                break;
              }
              else if (vf){
                if (vc==' ') sp_n++;
              }
              else if(vc!=' ') vf = 1;
            }
          }
        }
        if (sp_n>0){
          sp_v    = malloc(sizeof(int) * sp_n);
          memset(sp_v,0,sizeof(int) * sp_n);
          int pn  = 0;
          int pz  = lwpx;
          while (pz<maxwidth-indent){
            sp_v[pn]++;
            pz++;
            if (++pn>sp_n-1) pn=0;
          }
        }
      }
      
      byte first_space=0;
      int  space_pos  =0;
      while((off = *line_string++)){
        if (ag_check_escape(&off,&line_string,tb,0,NULL)){
          if (strcmp(tb,"/#")==0){
            if (!forcecolor) cl=cl_def;
          }
          else if ((tb[0]=='#')&&((strlen(tb)==4)||(strlen(tb)==7))){
            if (!forcecolor) cl=strtocolor(tb);
          }
          else if (strcmp(tb,"*")==0){
            if (indent>0){
              int vcx = (first_space)?cx:first_cx;
              ag_drawchar_ex(_b,vcx-(ag_fontwidth(' ',isbig)+ag_fontwidth(0xa9,isbig)),y,0xa9,cl,isbig,0,0);
              if (!first_space) cx = first_cx;
            }
          }
          else if (strcmp(tb,"/u")==0)      undr=0;
          else if (strcmp(tb,"u")==0)       undr=1;
          else if (strcmp(tb,"/b")==0)      bold=0;
          else if (strcmp(tb,"b")==0)       bold=1;
          else if (strcmp(tb,"@center")==0){
            algn=1;
            cx = ldpx/2 + x + indent;
            first_cx = cx;
          }
          else if (strcmp(tb,"@right")==0){
            algn=2;
            cx = ldpx + x + indent;
            first_cx = cx;
          }
          else if (strcmp(tb,"@fill")==0){
            algn=3;
            cx = x + indent;
            first_cx = cx;
            
            if (chalign==0){
              sp_n=0;
              char vc = 0;
              byte vf =0;
              const char * lstr = line_string;
              while((vc = *lstr++)){
                if (!ag_check_escape(&vc,&lstr,NULL,1,NULL)){
                  if (vc=='\t'){
                    sp_n = 0;
                    break;
                  }
                  else if (vf){
                    if (vc==' ') sp_n++;
                  }
                  else if(vc!=' ') vf = 1;
                }
              }
              
              if (sp_n>0){
                sp_v    = malloc(sizeof(int) * sp_n);
                memset(sp_v,0,sizeof(int) * sp_n);
                int pn  = 0;
                int pz  = lwpx;
                while (pz<maxwidth-indent){
                  sp_v[pn]++;
                  pz++;
                  if (++pn>sp_n-1) pn=0;
                }
              }
            }
          }
          else if ((strcmp(tb,"@left")==0)||(strcmp(tb,"/@")==0)){
            algn=0;
            cx = x + indent;
            first_cx = cx;
          }
        }
        else{          
          int fwidth = 0;
          if (off=='\t'){
            fwidth = ag_tabwidth(cx-x,isbig);
          }
          else{
            fwidth = ag_fontwidth(off,isbig);
            ag_drawchar_ex(_b,cx,y,off,cl,isbig,undr,bold);
          }
          
          if (first_space){
            if(off==' '){
              if (sp_n>space_pos){
                fwidth+=sp_v[space_pos];
                space_pos++;
              }
            }
          }
          else if(off!=' ') first_space = 1;
          
          cx+= fwidth;
        }
      }
      
      if (sp_v!=NULL) free(sp_v);
      free(bf);
    }
    
    if (!multiline) break;
    
    indent=next_indent;
    y+=fheight;
    s+=line_width;
    
    if (eos) break;
  }
  return 1;
}

//############################ OLD TEXT HANDLER
//-- Calculate Text Width
/*****
int ag_txtheight_(int maxwidth, const char *s, byte isbig){
  PNGFONTS * fnt = isbig?&AG_BIG_FONT:&AG_SMALL_FONT;
  if (!fnt->loaded) return 0;
  char off;
  int  curx = 0;
  int  fheight = fnt->fh;
  int  y=0;
  int  prevspace=0;
  byte onlongtext=0;
  while((off = *s++)){
    if (ag_check_escape(&off,&s,NULL,1)) continue;
    if (off=='\n'){
      curx = 0;
      y+=fheight;
      onlongtext = 0;
    }
    else{
      int nextspacew= 0;
      if (!onlongtext){
        const char * ss = s;
        char cf;
        while ((cf=*ss++)){
          if (ag_check_escape(&cf,&ss,NULL,1)) continue;
          if (cf=='\t')
            nextspacew+=ag_tabwidth(curx+nextspacew,isbig);
          else
            nextspacew+=ag_fontwidth(cf,isbig);
          if ((cf=='\t')||(cf==' ')||(cf=='\n')) break;
        }
      }
      if (nextspacew>maxwidth){
        if (curx>0){
          curx = 0;
          y+=fheight;
        }
        onlongtext = 1;
        nextspacew = 0;
      }
      if (curx+nextspacew>=maxwidth) {
        curx = 0;
        y+=fheight;
        prevspace=1;
        onlongtext = 0;
      }
      if ((prevspace==0)||((off!=' ')&&(off!='\t'))){
        int fwidth = 0;
        if (off=='\t')
          fwidth = ag_tabwidth(curx,isbig);
        else
          fwidth = ag_fontwidth(off,isbig);
        curx+= fwidth;
        if (curx>maxwidth) {
          curx = 0;
          y+=fheight;
          prevspace=1;
          onlongtext = 0;
        }
      }
      else{
        prevspace=0;
        if (off=='\t')
          curx = ag_tabwidth(0,isbig);
      }
    }
  }
  if (curx==0) return y;
  return (y+fheight);
}

byte ag_text_exl_(CANVAS *_b,int maxwidth,int x,int y, const char *s, color cl_def,byte isbig,byte forcecolor,byte multiline){
  if (_b==NULL) _b=&ag_c;
  if (!maxwidth) maxwidth = _b->w-x;
  
  PNGFONTS * fnt = isbig?&AG_BIG_FONT:&AG_SMALL_FONT;
  if (!fnt->loaded) return 0;
  char off;
  int  curx = x;
  int  fheight = fnt->fh;
  int  prevspace=0;
  int  i=0;
  char tb[8];
  color cl            = cl_def;
  byte  is_underline  = 0;
  byte  is_bold       = 0;
  byte  onlongtext    = 0;
  
  while((off = *s++)){
    if (ag_check_escape(&off,&s,tb,0)){
      if (strcmp(tb,"/#")==0){
        if (!forcecolor) cl=cl_def;
      }
      else if (strcmp(tb,"/u")==0){
        is_underline=0;
      }
      else if (strcmp(tb,"u")==0){
        is_underline=1;
      }
      else if (strcmp(tb,"/b")==0){
        is_bold=0;
      }
      else if (strcmp(tb,"b")==0){
        is_bold=1;
      }
      else if ((tb[0]=='#')&&((strlen(tb)==4)||(strlen(tb)==7))){
        if (!forcecolor) cl=strtocolor(tb);
      }
      continue;
    }
    
    if (off=='\n'){
      if (!multiline) break;
      curx = x;
      y+=fheight;
      onlongtext = 0;
    }
    else{
      int nextspacew= 0;
      if ((!onlongtext)&&(multiline)){
        const char * ss = s;
        char cf;
        while ((cf=*ss++)){
          if (ag_check_escape(&cf,&ss,NULL,1)) continue;
          if (cf=='\t')
            nextspacew+=ag_tabwidth(curx+nextspacew-x,isbig);
          else
            nextspacew+=ag_fontwidth(cf,isbig);
          if ((cf=='\t')||(cf==' ')||(cf=='\n')) break;
        }
      }
      if (nextspacew>maxwidth){
        if (curx-x>0){
          if (!multiline) break;
          curx = x;
          y+=fheight;
        }
        onlongtext = 1;
        nextspacew = 0;
      }
      if ((curx+nextspacew>=_b->w)||(curx-x+nextspacew>=maxwidth)) {
        if (!multiline) break;
        curx = x;
        y+=fheight;
        prevspace=1;
        onlongtext = 0;
      }
      if ((prevspace==0)||((off!=' ')&&(off!='\t'))){
        int fwidth = 0;
        if (off=='\t'){
          fwidth = ag_tabwidth(curx-x,isbig);
        }
        else{
          fwidth = ag_fontwidth(off,isbig);
          ag_drawchar_ex(_b,curx,y,off,cl,isbig,is_underline,is_bold);
        }
        curx+= fwidth;
        if ((curx>_b->w)||(curx-x>maxwidth)) {
          if (!multiline) break;
          curx = x;
          y+=fheight;
          prevspace=1;
          onlongtext = 0;
        }
      }
      else{
        prevspace=0;
        if (off=='\t'){
          curx = x+ag_tabwidth(0,isbig);
        }
      }
    }
  }
  return 1;
}
*****/