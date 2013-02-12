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
 * Main AROMA Installer HEADER
 *
 */

#ifndef __AROMA_H__
#define __AROMA_H__

//
// Common Headers, Always Used
//
#include <stdio.h>
#include <stdlib.h>
//#include <math.h>
#include <string.h>
#include <unistd.h>
#include "libs/amath.h"
#include <sys/wait.h>

//
// ARM NEON - Testing Only
//
#ifdef __ARM_NEON__
  #include <arm_neon.h>
#endif


// #define _AROMA_NODEBUG
#define _AROMA_NODEBUG

//#######################################################//
//##                                                   ##//
//##               LIST OF DEFINITIONS                 ##//
//##                                                   ##//
//#######################################################//

//
// Common Data Type
//
#define byte              unsigned char
#define dword             unsigned int
#define word              unsigned short
#define color             unsigned short


//
// AROMA Main Configurations
//
#define AROMA_NAME        "AROMA INSTALLER"
#define AROMA_VERSION     "1.64"
#define AROMA_BUILD       "120414-040"
#define AROMA_BUILD_CN    "Cempaka"
#define AROMA_BUILD_L     "Bandung - Indonesia"
#define AROMA_BUILD_A     "<support@amarullz.com>"
#define AROMA_BUILD_URL   "http://www.amarullz.com/"
#define AROMA_COPY        "(c) 2012 by amarullz xda-developers"

//-- Temporary Dir - Move from /tmp/aroma-data to /tmp/aroma symlink to /tmp/aroma-data for backward compatibility
#define AROMA_SYSTMP      "/tmp"
//#define AROMA_SYSTMP      "/data"
#define AROMA_TMP         AROMA_SYSTMP "/aroma"
#define AROMA_TMP_S       AROMA_SYSTMP "/aroma-data"

#define AROMA_DIR         "META-INF/com/google/android/aroma"
#define AROMA_CFG         "META-INF/com/google/android/aroma-config"
#define AROMA_UPDATESCRPT "META-INF/com/google/android/updater-script"
#define AROMA_ORIB        "META-INF/com/google/android/update-binary-installer"
#define AROMA_FRAMEBUFFER "/dev/graphics/fb0"
#define AROMA_INSTALL_LOG (AROMA_TMP "/.install.log")
#define AROMA_INSTALL_TXT (AROMA_TMP "/.install.txt")
#define AROMA_THEME_CNT   24

//
// AROMA Canvas Structure
//
typedef struct{
	int     w;       // Width
	int     h;       // Height
	int     sz;      // Data Size
	color * data;    // Data 
} CANVAS;


//
// AROMA PNG Canvas Structure
//
typedef struct {
  int     w;       // Width
  int     h;       // Height
  int     s;       // Buffer Size
  byte    c;       // Channels
  byte *  r;       // Red Channel
  byte *  g;       // Green Channel
  byte *  b;       // Blue Channel
  byte *  a;       // Alpha Channel
} PNGCANVAS, * PNGCANVASP;


//
// AROMA PNG Font Canvas Structure
//
typedef struct {
  byte    loaded;    // Font is Loaded 
  int     fx[96];    // Font X Positions
  byte    fw[96];    // Font Width
  byte    fh;        // Font Height
  int     w;         // Png Width
  int     h;         // Png Height
  int     s;         // Buffer Size
  byte    c;         // Channels
  byte *  d;         // Fonts Alpha Channel
} PNGFONTS;


//
// AROMA ZIP Memory Structure
//
typedef struct{
	int sz;         // Data Size
	byte *data;     // Data 
} AZMEM;


//
// AROMA Touch & Event Structure
//
typedef struct{
	int   x;        // Touch X
	int   y;        // Touch Y
	int   d;        // Down State
	int   k;        // Key Code
	dword msg;      // Window Message for postmessage
} ATEV;


//
// Math Macro
//
#define max(a,b) ((a>=b)?a:b)
#define min(a,b) ((a<=b)?a:b)
#define LOWORD(l) ((word)(l))
#define HIWORD(l) ((word)(((dword)(l) >> 16) & 0xFFFF))
#define MAKEDWORD(a, b) ((dword) (((word) (a)) | ((dword) ((word) (b))) << 16))

//
// AROMA Graphic Pixel Macro
//
#define ag_r(rgb)	          ((byte) (((((word)(rgb))&0xF800))>>8) ) 
#define ag_g(rgb)	          ((byte) (((((word)(rgb))&0x07E0))>>3) ) 
#define ag_b(rgb)	          ((byte) (((((word)(rgb))&0x001F))<<3) ) 
#define ag_rgb(r,g,b)       ((color) ((r >> 3) << 11)| ((g >> 2) << 5)| ((b >> 3) << 0))
#define ag_rgba32(r,g,b,a)  ((dword)((((a)&0xff)<<24)|(((b)&0xff)<<16)|(((g)&0xff)<<8)|((r)&0xff)))
#define ag_rgb32(r,g,b)     ag_rgba32(r,g,b,0xff)
#define ag_r32(rgb)         ((byte) (rgb)) 
#define ag_g32(rgb)         ((byte) (((word) (rgb))>>8))
#define ag_b32(rgb)         ((byte) ((rgb) >> 16)) 
#define ag_a32(rgb)         ((byte) (((dword) (rgb))>>24))
#define ag_close_r(r)       (((byte) r)>>3<<3)
#define ag_close_g(g)       (((byte) g)>>2<<2)
#define ag_close_b(b)       ag_close_r(b)
#define ag_rgbto32(rgb)     (ag_rgba32(ag_r(rgb),ag_g(rgb),ag_b(rgb),0xff))
#define ag_rgbto16(rgb)     (ag_rgb(ag_r32(rgb),ag_g32(rgb),ag_b32(rgb)))


//
// AROMA Touch Event Code
//
#define ATEV_DOWN     1
#define ATEV_UP       2
#define ATEV_LEFT     3
#define ATEV_RIGHT    4
#define ATEV_SELECT   5
#define ATEV_BACK     6
#define ATEV_MENU     7
#define ATEV_HOME     8
#define ATEV_MOUSEDN  9
#define ATEV_MOUSEUP  10
#define ATEV_MOUSEMV  11
#define ATEV_SEARCH   12
#define ATEV_MESSAGE  30
#define KEY_CENTER    232


//
// AROMA Kinetic Library Structures
//
#define AKINETIC_HISTORY_LENGTH     10
#define AKINETIC_DAMPERING          0.98              // Gravity
typedef struct  {
  byte    isdown;                                     // Is Touch Down
  double  velocity;                                   // Fling Velocity
  int     previousPoints[AKINETIC_HISTORY_LENGTH];    // Touch Y Pos History
  long    previousTimes[AKINETIC_HISTORY_LENGTH];     // Touch Time History
  byte    history_n;                                  // Number of Touch History
} AKINETIC;


//
// AROMA Window Message - In DWORD (4bytes)
//
// m = message, d = draw, l = don't lost focus, h = high value
#define aw_msg(m,d,l,h) ((dword)((((h)&0xff)<<24)|(((l)&0xff)<<16)|(((d)&0xff)<<8)|((m)&0xff)))
#define aw_gm(msg)      ((byte) (msg)) 
#define aw_gd(msg)      ((byte) (((word) (msg))>>8))
#define aw_gl(msg)      ((byte) ((msg) >> 16)) 
#define aw_gh(msg)      ((byte) (((dword) (msg))>>24))


//
// AROMA Main Configuration Structure
//
typedef struct  {
  // Colors
  color winbg;                // Window Background
  color winbg_g;              // Window Background Gradient
  color winfg;                // Window Foreground
  color winfg_gray;           // Window Foreground
  color dialogbg;             // Dialog Background
  color dialogbg_g;           // Dialog Background Gradient
  color dialogfg;             // Dialog Foreground
  color textbg;               // Text / List Background
  color textfg;               // Text / List Font Color
  color textfg_gray;          // List Grayed Font Color ( List Description )
  color controlbg;            // Control/Button Background
  color controlbg_g;          // Control/Button Background Gradient
  color controlfg;            // Control/Button Font Color
  color selectbg;             // Selected Item/Control Background
  color selectbg_g;           // Selected Item/Control Background Gradient
  color selectfg;             // Selected Item/Control Font Color
  color titlebg;              // Title Background
  color titlebg_g;            // Title Background Gradient
  color titlefg;              // Title Font Color
  color dlgtitlebg;           // Dialog Title Background
  color dlgtitlebg_g;         // Dialog Title Background Gradient
  color dlgtitlefg;           // Dialog Title Font Color
  color navbg;                // Scrollbar Color
  color navbg_g;              // Navigation Bar Background
  color scrollbar;            // Navigation Bar Background Gradient
  color border;               // Border Color
  color border_g;             // Border Color Gradient
  color progressglow;         // Progress Bar Glow Color
  
  // Property
  byte  roundsz;              // Control Rounded Size
  byte  btnroundsz;           // Button Control Rounded Size
  byte  winroundsz;           // Window Rounded Size
  
  // Transition
  byte  fadeframes;           // Number of Frame used for Fade Transition
  
  // Common Text
  char  text_ok[32];          // OK
  char  text_next[32];        // Next >
  char  text_back[32];        // < Back
  
  char  text_yes[32];         // Yes
  char  text_no[32];          // No
  char  text_about[32];       // About
  char  text_calibrating[32]; // Calibration Tools
  char  text_quit[32];        // Quit
  char  text_quit_msg[64];   // Quit Message
  char  text_save_logs[32];   // Save Logs
  
  // ROM Text
  char rom_name[64];          // ROM Name
  char rom_version[64];       // ROM Version
  char rom_author[64];        // ROM Author
  char rom_device[64];        // ROM Device Name
  char rom_date[64];          // ROM Date
  
  // CUSTOM KEY
  int ckey_up;
  int ckey_down;
  int ckey_select;
  int ckey_back;
  int ckey_menu;
  
  // THEME
  PNGCANVASP theme[AROMA_THEME_CNT];
  byte       theme_9p[AROMA_THEME_CNT];
  char themename[64];
} AC_CONFIG;



//
// AROMA Window Control Callback Typedef
//
typedef dword (*AC_ONINPUT)(void *,int,ATEV *);
typedef void  (*AC_ONBLUR)(void *);
typedef byte  (*AC_ONFOCUS)(void *);
typedef void  (*AC_ONDRAW)(void *);
typedef void  (*AC_ONDESTROY)(void *);


//
// AROMA Window Structure
//
typedef struct{
  byte          isActived;    // Active & Showed
	CANVAS *      bg;           // Background Canvas
	CANVAS        c;            // Window drawing canvas
	void**        controls;     // Child Controls
	int           controln;     // Number of Controls
	int           threadnum;    // Number of running thread
	int           focusIndex;   // Child Focus Index
	int           touchIndex;   // Child Touch Index
} AWINDOW, *AWINDOWP;


//
// AROMA Control Structure
//
typedef struct{
  AWINDOWP      win;          // Parent Window
  AC_ONDESTROY  ondestroy;    // On Destroy Callback
  AC_ONINPUT    oninput;      // On Input Callback
  AC_ONDRAW     ondraw;       // On Draw Callback
  AC_ONBLUR     onblur;       // On Blur Callback
  AC_ONFOCUS    onfocus;      // On Focus Callback
  int           x;            // Control X
  int           y;            // Control Y
  int           w;            // Control Width
  int           h;            // Control Height
  byte          forceNS;      // Force to Stop Scroll
  void *        d;            // Control Specific Data
} ACONTROL, *ACONTROLP;


//#######################################################//
//##                                                   ##//
//##                LIST OF FUNCTIONS                  ##//
//##                                                   ##//
//#######################################################//

//
// AROMA Root Functions
//
FILE *    apipe();        // Recovery pipe to communicate the command
byte      aui_start();    // Start AROMA UI
char*     getArgv(int id);
void      a_reboot(byte type);


//
// AROMA Zip Functions
//
byte      az_init(const char * filename);                               // Init Zip Archive
void      az_close();                                                   // Release Zip Archive
byte      az_readmem(AZMEM * out,const char * zpath, byte bytesafe);    // Read Zip Item into Memory
byte      az_extract(const char * zpath, const char * dest);            // Extract Zip Item into Filesystem


//-- UI Functions
char * aui_parsepropstring(char * buffer,char *key);
char * aui_readfromzip(char * name);
void aui_drawnav(CANVAS * bg,int x, int y, int w, int h);

//-- .9.png struct
typedef struct{
  int x;  //-- Strect X
  int y;  //-- Strect Y
  int w;  //-- Strect Width  
  int h;  //-- Strect Height
  
  int t;  //-- Padding Top
  int l;  //-- Padding Left
  int b;  //-- Padding Bottom
  int r;  //-- Padding Right
} APNG9, *APNG9P;

//
// AROMA PNG Functions
//
byte      apng_load(PNGCANVAS * pngcanvas,char* imgname);         // Load PNG From Zip Item
void      apng_close(PNGCANVAS * pngcanvas);                            // Release PNG Memory
byte      apng_draw(CANVAS * _b, PNGCANVAS * p, int xpos, int ypos);    // Draw PNG Into Canvas
byte apng_stretch(
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
byte apng9_calc(PNGCANVAS * p, APNG9P v,byte with_pad);
byte apng9_draw(
  CANVAS * _b,
  PNGCANVAS * p,
  int dx,
  int dy,
  int dw,
  int dh,
  APNG9P v,
  byte with_pad
);

//
// AROMA PNG Font Functions
//
byte      apng_loadfont(PNGFONTS * pngfont,const char* imgname);        // Load PNG Font From Zip Item
byte      apng_drawfont(CANVAS * _b, PNGFONTS * p, byte fpos,           // Draw PNG Font Into Canvas
            int xpos, int ypos, color cl, byte underline, byte bold);
byte      apng_draw_ex(CANVAS * _b, PNGCANVAS * p, int xpos,            // Draw PNG Font Into Canvas
            int ypos, int sxpos, int sypos,int sw, int sh);             // With Extra Arguments

//
// AROMA Graphic Function
//
CANVAS *  agc();          // Get Main AROMA Graph Canvas
byte      ag_init();      // Init AROMA Graph and Framebuffers
void      ag_close_thread(); // Close Graph Thread
void      ag_close();     // Close AROMA Graph and Framebuffers
void      ag_changecolorspace(int r, int g, int b, int a); // Change Color Space

void      ag_sync();                        // Sync Main Canvas with Framebuffer
int       agw();                            // Get Display X Resolution
int       agh();                            // Get Display Y Resolution
int       agdp();                           // Get Device Pixel Size (WVGA = 3, HVGA = 2)
void      ag_sync_fade(int frame);          // Transition Sync - Async
void      ag_sync_fade_wait(int frame);     // Transition Sync - Sync
void      ag_sync_force();                  // Force to Sync
void      ag_setbusy();                     // Set Display to show Please Wait Progress
void      ag_setbusy_withtext(char * text); // Display Busy Progress with Custom Text

//
// AROMA Canvas Functions
//
void ag_canvas(CANVAS * c,int w,int h);   // Create Canvas
void ag_ccanvas(CANVAS * c);              // Release Canvas
void ag_blank(CANVAS * c);                // Set Blank into Canvas memset(0)


//
// AROMA Canvas Manipulation Functions
//
color *   agxy(CANVAS *_b, int x, int y);                             // Get Pixel Pointer
byte      ag_setpixel(CANVAS *_b,int x, int y,color cl);              // Set Pixel Color
byte      ag_subpixel(CANVAS *_b,int x, int y, color cl,byte l);      // Set Pixel Color with Opacity


//
// AROMA Canvas Drawing Functions
//
byte      ag_rect(CANVAS *_b,int x, int y, int w, int h, color cl);   // Draw Solid Rectangle
byte      ag_rectopa(CANVAS *_b,int x, int y, int w, int h,           // Draw Solid Rectangle with Opacity
            color cl,byte l);
byte      ag_draw(CANVAS * dc,CANVAS * sc,int dx, int dy);            // Draw Canvas to Canvas
byte      ag_draw_ex(CANVAS * dc,CANVAS * sc, int dx, int dy,         // Draw Canvas to Canvas + Extra Arguments
            int sx, int sy, int sw, int sh);
byte      ag_roundgrad(CANVAS *_b,int x, int y, int w, int h,         // Draw Rounded & Gradient Rectangle
            color cl1, color cl2, int roundsz);
byte      ag_roundgrad_ex(CANVAS *_b,int x, int y, int w, int h,      // Draw Rounded & Gradient Rectangle
            color cl1, color cl2, int roundsz, byte tlr,              // With Extra Arguments
            byte trr, byte blr, byte brr);


//
// AROMA Color Calculator Functions
//
color     ag_subpixelget(CANVAS *_b,int x, int y, color cl,byte l);   // Calculate Color Opacity with Canvas Pixel
color     ag_calculatealpha(color dcl,color scl,byte l);              // Calculate 2 Colors with Opacity
color     strtocolor(char * c);                                       // Convert String Hex Color #fff,#ffffff to color
dword     ag_calchighlight(color c1,color c2);
dword     ag_calcpushlight(color c1,color c2);
color     ag_calpushad(color c_g);
color     ag_calculatecontrast(color c,float intensity);

//
// AROMA PNG Font Functions
//
int   ag_fontheight(byte isbig);                      // Get Font Height
byte  ag_loadsmallfont(char * fontname);              // Load Small Font From Zip
byte  ag_loadbigfont(char * fontname);                // Load Big Font From Zip
void  ag_closefonts();                                // Release Big & Small Fonts
byte  ag_drawchar(CANVAS *_b,int x, int y, char c,    // Draw Character into Canvas
        color cl, byte isbig);
byte ag_drawchar_ex(CANVAS *_b,int x, int y, char c, color cl, byte isbig,byte underline,byte bold);
byte  ag_text(CANVAS *_b,int maxwidth,int x,int y,    // Draw String into Canvas
        const char *s, color cl,byte isbig);
byte  ag_textf(CANVAS *_b,int maxwidth,int x,int y,    // Draw String into Canvas
        const char *s, color cl,byte isbig);          // Force Default Color

byte ag_text_ex(CANVAS *_b,int maxwidth,int x,int y,  // Draw String into Canvas
        const char *s, color cl_def,byte isbig,       // With Extra Arguments
        byte forcecolor);
int   ag_txtheight(int maxwidth,                      // Calculate String Height to be drawn
        const char *s, byte isbig);
int   ag_txtwidth(const char *s, byte isbig);         // Calculate String Width to be drawn
int  ag_tabwidth(int x, byte isbig);
byte ag_fontwidth(char c,byte isbig);                // Calculate font width for 1 character
byte ag_texts(CANVAS *_b,int maxwidth,int x,int y, const char *s, color cl_def,byte isbig);
byte ag_textfs(CANVAS *_b,int maxwidth,int x,int y, const char *s, color cl_def,byte isbig);
byte ag_text_exl(CANVAS *_b,int maxwidth,int x,int y, const char *s, color cl_def,byte isbig,byte forcecolor,byte multiline);
//
// AROMA EVENTS & Input Functions
//   NOTE: Contains Others Works
//         Modified from "minui/events.c"
//         Copyright (C) 2007 The Android Open Source Project
//         Licensed under the Apache License
//
byte    atouch_gethack();
void    atouch_sethack(byte t);
struct  input_event;
void    atouch_set_calibrate(float dx, int ax, float dy, int ay);
int     atouch_wait(ATEV *atev);
int     atouch_wait_ex(ATEV *atev, byte calibratingtouch);
void    atouch_send_message(dword msg);
int     vibrate(int timeout_ms);
void    ui_init();
int     ev_init(void);
void    ev_exit(void);
int     ev_get(struct input_event *ev, unsigned dont_wait);
int     ui_wait_key();
int     ui_key_pressed(int key);
void    ui_clear_key_queue();
int     touchX();
int     touchY();
int     ontouch();
void    set_key_pressed(int key,char val);
int     atmsg();

//
// AROMA System Library Functions
//
char * ai_rtrim(char * chr);
char * ai_trim(char * chr);
byte  ismounted(char * path);
byte alib_disksize(const char * path, unsigned long * ret, int division);
int   alib_diskusage(const char * path);
byte alib_diskfree(const char * path, unsigned long * ret, int division);
void  alib_exec(char * cmd, char * arg);
void  create_directory(const char *path);
int   remove_directory(const char *path);
long  alib_tick();

//
// AROMA Kinetic Calculator Functions
//
void  akinetic_downhandler(AKINETIC * p, int mouseY);
int   akinetic_movehandler(AKINETIC * p, int mouseY);
byte  akinetic_uphandler(AKINETIC * p, int mouseY);
int   akinetic_fling(AKINETIC * p);
int   akinetic_fling_dampered(AKINETIC * p, float dampersz);

//
// Customization Functions
//
AC_CONFIG * acfg();           // Get Config Structure
void        acfg_init();      // Set Default Config
void acfg_init_ex(byte themeonly);

//
// AROMA Start Main Installer
//
int aroma_start_install(
  CANVAS * bg,
  int cx, int cy, int cw, int ch,
  int px, int py, int pw, int ph,
  CANVAS * cvf, int imgY, int chkFY, int chkFH
);

//
// AROMA THEME MANAGER
//
void        atheme_releaseall();
void        atheme_release(char * key);
PNGCANVASP  atheme_create(char * key, char * path);
PNGCANVASP  atheme(char * key);
int         atheme_id(char * key);
char *      atheme_key(int id);
byte        atheme_id_draw(int id, CANVAS * _b, int x, int y, int w, int h);
byte        atheme_draw(char * key, CANVAS * _b, int x, int y, int w, int h);

//
// AROMA Window Management System Functions
//
AWINDOWP  aw(CANVAS * bg);                                  // Create New Window
void      aw_destroy(AWINDOWP win);                         // Destroy Window
void      aw_show(AWINDOWP win);                            // Show Window
void      aw_draw(AWINDOWP win);                            // Redraw Window
void      aw_add(AWINDOWP win,ACONTROLP ctl);               // Add Control into Window
void      aw_post(dword msg);                               // Post Message
dword     aw_dispatch(AWINDOWP win);                        // Dispatch Event, Message & Input
byte      aw_touchoncontrol(ACONTROLP ctl, int x, int y);   // Calculate Touch Position
byte      aw_setfocus(AWINDOWP win,ACONTROLP ctl);          // Set Focus into Control
void      aw_set_on_dialog(byte d);
void      atouch_plaincalibrate();
void      atouch_restorecalibrate();
void      aw_calibtools(AWINDOWP parent);
//
// AROMA Window Dialog Controls
//
void aw_alert(AWINDOWP parent,char * titlev,char * textv,char * img,char * ok_text);
byte aw_confirm(AWINDOWP parent, char * titlev,char * textv,char * img,char * yes_text,char * no_text);
void aw_textdialog(AWINDOWP parent,char * title,char * text,char * ok_text);
void aw_about_dialog(AWINDOWP parent);
byte aw_showmenu(AWINDOWP parent);

//
// AROMA Window Threading Functions
//
void ac_regbounce(
  ACONTROLP       ctl,
  int *           scrollY,
  int             maxScrollY
);
void ac_regfling(
  ACONTROLP       ctl,
  AKINETIC *      akin,
  int *           scrollY,
  int             maxScrollY
);
void ac_regpushwait(
  ACONTROLP     ctl,
  int *         moveY,
  int *         flagpointer,
  int           flagvalue
);
void ac_regscrollto(
  ACONTROLP       ctl,
  int *           scrollY,
  int             maxScrollY,
  int             requestY,
  int *           requestHandler,
  int             requestValue
);

//
// AROMA Controls Functions
//
void actext_rebuild(ACONTROLP ctl,int x,int y,int w,int h,char * text,byte isbig,byte toBottom);
void actext_appendtxt(ACONTROLP ctl,char * txt);
ACONTROLP actext(
  AWINDOWP win,
  int x,
  int y,
  int w,
  int h,
  char * text,
  byte isbig
);
ACONTROLP acbutton(
  AWINDOWP win,
  int x,
  int y,
  int w,
  int h,
  char * text,
  byte isbig,
  byte touchmsg
);
ACONTROLP accheck(
  AWINDOWP win,
  int x,
  int y,
  int w,
  int h
);
byte accheck_add(ACONTROLP ctl,char * title, char * desc, byte checked);
byte accheck_addgroup(ACONTROLP ctl,char * title, char * desc);
int accheck_itemcount(ACONTROLP ctl);
byte accheck_ischecked(ACONTROLP ctl, int index);
byte accheck_isgroup(ACONTROLP ctl, int index);
int accheck_getgroup(ACONTROLP ctl, int index);
int accheck_getgroupid(ACONTROLP ctl, int index);
ACONTROLP acopt(
  AWINDOWP win,
  int x,
  int y,
  int w,
  int h
);
byte acopt_addgroup(ACONTROLP ctl,char * title, char * desc);
byte acopt_add(ACONTROLP ctl,char * title, char * desc, byte selected);
int acopt_getselectedindex(ACONTROLP ctl,int group);
int acopt_getgroupid(ACONTROLP ctl, int index);
ACONTROLP accb(
  AWINDOWP win,
  int x,
  int y,
  int w,
  int h,
  char * textv,
  byte checked
);
byte accb_ischecked(ACONTROLP ctl);
ACONTROLP acmenu(
  AWINDOWP win,
  int x,
  int y,
  int w,
  int h,
  byte touchmsg
);
byte acmenu_add(ACONTROLP ctl,char * title, char * desc, char * img);
int acmenu_getselectedindex(ACONTROLP ctl);

//**********[ AROMA LOGGING ]**********//
#define _AROMA_DEBUG_TAG "aroma"
#ifndef _AROMA_NODEBUG
#define LOGS(...) fprintf(stdout, _AROMA_DEBUG_TAG "/s: " __VA_ARGS__)
#define LOGE(...) fprintf(stdout, _AROMA_DEBUG_TAG "/e: " __VA_ARGS__)
#define LOGW(...) fprintf(stdout, _AROMA_DEBUG_TAG "/w: " __VA_ARGS__)
#define LOGI(...) fprintf(stdout, _AROMA_DEBUG_TAG "/i: " __VA_ARGS__)
#define LOGV(...) fprintf(stdout, _AROMA_DEBUG_TAG "/v: " __VA_ARGS__)
#define LOGD(...) fprintf(stdout, _AROMA_DEBUG_TAG "/d: " __VA_ARGS__)
#else
#define LOGS(...) fprintf(stdout, _AROMA_DEBUG_TAG "/s: " __VA_ARGS__)
#define LOGE(...) fprintf(stdout, _AROMA_DEBUG_TAG "/e: " __VA_ARGS__)
#define LOGW(...) /**/
#define LOGI(...) /**/
#define LOGV(...) /**/
#define LOGD(...) /**/
#endif
#define STRINGIFY(x) #x
#define EXPAND(x) STRINGIFY(x)


#endif // __AROMA_H__