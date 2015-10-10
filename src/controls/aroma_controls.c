/*
 * Copyright (C) 2011-2012 Ahmad Amarullah ( http://amarullz.com/ )
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
 * AROMA UI: Main AROMA UI Window
 *
 */
#include <sched.h>
#include "../aroma.h"

/***************************[ GLOBAL VARIABLES ]**************************/
static AC_CONFIG acfg_var;
static byte      on_dialog_window = 0;

/***************************[ CONFIG FUNCTIONS ]**************************/
AC_CONFIG * acfg() {
  return &acfg_var;
}
void acfg_reset_text() {
  snprintf(acfg_var.text_ok, 64, "OK");
  snprintf(acfg_var.text_next, 64, "Next");
  snprintf(acfg_var.text_back, 64, "Back");
  snprintf(acfg_var.text_yes, 64, "Yes");
  snprintf(acfg_var.text_no, 64, "No");
  snprintf(acfg_var.text_about, 64, "About");
  snprintf(acfg_var.text_calibrating, 64, "Calibrating Tools");
  snprintf(acfg_var.text_quit, 64, "Quit Installation");
  snprintf(acfg_var.text_quit_msg, 128, "Are you sure to quit the installer?");
  snprintf(acfg_var.icon_next, 128, "@next");
  snprintf(acfg_var.icon_back, 128, "@back");
}
void acfg_init_ex(byte themeonly) {
  acfg_var.winbg        = ag_rgb(0xf0, 0xf0, 0xf0);
  acfg_var.winbg_g      = ag_rgb(0xee, 0xee, 0xee);
  acfg_var.dialogbg     = acfg_var.winbg;
  acfg_var.dialogbg_g   = acfg_var.winbg_g;
  acfg_var.textbg       = ag_rgb(0xff, 0xff, 0xff);
  acfg_var.textfg       = ag_rgb(0x00, 0x00, 0x00);
  acfg_var.textfg_gray  = ag_rgb(0x88, 0x88, 0x88);
  acfg_var.winfg_gray   = acfg_var.textfg_gray;
  acfg_var.winfg        = acfg_var.textfg;
  acfg_var.dialogfg     = acfg_var.textfg;
  acfg_var.controlbg    = ag_rgb(0xf5, 0xf5, 0xf5);
  acfg_var.controlbg_g  = ag_rgb(0xdf, 0xdf, 0xdf);
  acfg_var.controlfg    = ag_rgb(0x44, 0x44, 0x44);
  acfg_var.selectbg     = ag_rgb(158, 228, 32);
  acfg_var.selectbg_g   = ag_rgb(106, 160, 44);
  acfg_var.selectfg     = ag_rgb(0xff, 0xff, 0xff);
  acfg_var.titlebg      = ag_rgb(0x22, 0x22, 0x22);
  acfg_var.titlebg_g    = ag_rgb(0x11, 0x11, 0x11);
  acfg_var.titlefg      = ag_rgb(0xff, 0xff, 0xff);
  acfg_var.dlgtitlebg   = acfg_var.titlebg;
  acfg_var.dlgtitlebg_g = acfg_var.titlebg_g;
  acfg_var.dlgtitlefg   = acfg_var.titlefg;
  acfg_var.navbg        = ag_rgb(0xf5, 0xf5, 0xf5);
  acfg_var.navbg_g      = ag_rgb(0xdf, 0xdf, 0xdf);
  acfg_var.scrollbar    = ag_rgb(0x66, 0x66, 0x66);
  acfg_var.border       = ag_rgb(0xcc, 0xcc, 0xcc);
  acfg_var.border_g     = ag_rgb(0x99, 0x99, 0x99);
  acfg_var.progressglow = acfg_var.selectbg;
  acfg_var.winroundsz   = 3;
  acfg_var.roundsz      = 2;
  acfg_var.btnroundsz   = 2;
  acfg_var.fadeframes   = 5;
  acfg_var.vibrate      = 5;
  set_vibrate_rate(acfg()->vibrate);
  snprintf(acfg_var.themename, 64, "");
  
  if (themeonly == 0) {
    acfg_reset_text();
    snprintf(acfg_var.rom_name, 128, AROMA_NAME);
    snprintf(acfg_var.rom_version, 128, AROMA_VERSION);
    snprintf(acfg_var.rom_author, 128, AROMA_BUILD_A);
    snprintf(acfg_var.rom_device, 128, "Not Defined");
    snprintf(acfg_var.rom_date, 128, AROMA_BUILD);
    acfg_var.ckey_up      = 0;
    acfg_var.ckey_down    = 0;
    acfg_var.ckey_select  = 0;
    acfg_var.ckey_back    = 0;
    acfg_var.ckey_menu    = 0;
  }
  
  atheme_releaseall();
}
void acfg_init() {
  acfg_init_ex(0);
}

/***************************[ THEME ]**************************/
static char theme_name[AROMA_THEME_CNT][27] = {
  "img.background",
  "img.titlebar",
  "img.navbar",
  "img.dialog",
  "img.dialog.titlebar",
  "img.progress",
  "img.prograss.fill",
  "img.selection",
  "img.selection.push",
  "img.button",
  "img.button.focus",
  "img.button.push",
  "img.checkbox",
  "img.checkbox.focus",
  "img.checkbox.push",
  "img.checkbox.on",
  "img.checkbox.on.focus",
  "img.checkbox.on.push",
  "img.radio",
  "img.radio.focus",
  "img.radio.push",
  "img.radio.on",
  "img.radio.on.focus",
  "img.radio.on.push"
};
void atheme_releaseall() {
  int i = 0;
  
  for (i = 0; i < AROMA_THEME_CNT; i++) {
    if (acfg_var.theme[i] != NULL) {
      apng_close(acfg_var.theme[i]);
      free(acfg_var.theme[i]);
    }
    
    acfg_var.theme[i]   = NULL;
    acfg_var.theme_9p[i] = 0;
  }
}
void atheme_release(char * key) {
  int i = 0;
  
  for (i = 0; i < AROMA_THEME_CNT; i++) {
    if (strcmp(theme_name[i], key) == 0) {
      if (acfg_var.theme[i] != NULL) {
        apng_close(acfg_var.theme[i]);
        free(acfg_var.theme[i]);
        acfg_var.theme[i] = NULL;
        acfg_var.theme_9p[i] = 0;
      }
      
      return;
    }
  }
  
  return;
}
PNGCANVASP atheme_create(char * key, char * path) {
  int id = atheme_id(key);
  
  if (id != -1) {
    PNGCANVAS * ap = malloc(sizeof(PNGCANVAS));
    
    if (apng_load(ap, path)) {
      if (acfg_var.theme[id] != NULL) {
        apng_close(acfg_var.theme[id]);
        free(acfg_var.theme[id]);
        acfg_var.theme[id] = NULL;
        acfg_var.theme_9p[id] = 0;
      }
      
      acfg_var.theme[id]  = ap;
      int ln = strlen(path) - 1;
      acfg_var.theme_9p[id] = 0;
      
      if (ln > 2) {
        if ((path[ln] == '9') && (path[ln - 1] == '.')) {
          acfg_var.theme_9p[id] = 1;
        }
      }
      
      return ap;
    }
    
    free(ap);
  }
  
  return NULL;
}
byte atheme_draw(char * key, CANVAS * _b, int x, int y, int w, int h) {
  return atheme_id_draw(atheme_id(key), _b, x, y, w, h);
}
byte atheme_id_draw(int id, CANVAS * _b, int x, int y, int w, int h) {
  if (id < 0) {
    return 0;
  }
  
  if (id >= AROMA_THEME_CNT) {
    return 0;
  }
  
  if (acfg_var.theme[id] != NULL) {
    if (acfg_var.theme_9p[id]) {
      return apng9_draw(_b, acfg_var.theme[id], x, y, w, h, NULL, 1);
    }
    else {
      return apng_stretch(
               _b,
               acfg_var.theme[id],
               x, y, w, h,
               0, 0, acfg_var.theme[id]->w, acfg_var.theme[id]->h);
    }
  }
  
  return 0;
}
PNGCANVASP atheme(char * key) {
  int i = 0;
  
  for (i = 0; i < AROMA_THEME_CNT; i++) {
    if (strcmp(theme_name[i], key) == 0) {
      return acfg_var.theme[i];
    }
  }
  
  return NULL;
}
int atheme_id(char * key) {
  int i = 0;
  
  for (i = 0; i < AROMA_THEME_CNT; i++) {
    if (strcmp(theme_name[i], key) == 0) {
      return i;
    }
  }
  
  return -1;
}
char * atheme_key(int id) {
  if (id < 0) {
    return NULL;
  }
  
  if (id >= AROMA_THEME_CNT) {
    return NULL;
  }
  
  return theme_name[id];
}


/***************************[ WINDOW FUNCTIONS ]**************************/
//-- CREATE WINDOW
AWINDOWP aw(CANVAS * bg) {
  ag_setbusy();
  //sleep(4);
  //-- Create Window
  AWINDOWP win = (AWINDOWP) malloc(sizeof(AWINDOW));
  
  if (win == NULL) {
    return NULL;
  }
  
  //-- Create Canvas & Draw BG
  ag_canvas(&win->c, agw(), agh());
  ag_draw(&win->c, bg, 0, 0);
  //-- Initializing Variables
  win->bg           = bg;
  win->controls     = NULL;
  win->controln     = 0;
  win->threadnum    = 0;
  win->focusIndex   = -1;
  win->touchIndex   = -1;
  win->isActived    = 0;
  //-- RETURN
  return win;
}
void aw_set_on_dialog(byte d) {
  on_dialog_window = d;
}
//-- DESTROY WINDOW
void aw_destroy(AWINDOWP win) {
  ag_setbusy();
  //-- Set To Unactive
  win->isActived = 0;
  //-- Wait Thread To Closed
  int threadwait_n = 0;
  
  while (win->threadnum > 0) {
    usleep(500);
    
    if (threadwait_n++ > 1000) {
      break;
    }
  }
  
  //-- Cleanup Controls
  if (win->controln > 0) {
    int i;
    ACONTROLP * controls = (ACONTROLP *) win->controls;
    
    for (i = win->controln - 1; i >= 0; i--) {
      controls[i]->ondestroy((void *) controls[i]);
      free(controls[i]);
    }
    
    free(win->controls);
  }
  
  //-- Cleanup Window
  ag_ccanvas(&win->c);
  free(win);
}

//-- Add Control Into Window
void aw_add(AWINDOWP win, ACONTROLP ctl) {
  if (win->controln > 0) {
    int i;
    void ** tmpctls   = win->controls;
    win->controls     = malloc( sizeof(ACONTROLP) * (win->controln + 1) );
    
    for (i = 0; i < win->controln; i++) {
      win->controls[i] = tmpctls[i];
    }
    
    win->controls[win->controln] = (void *) ctl;
    free(tmpctls);
  }
  else {
    win->controls    = malloc(sizeof(ACONTROLP));
    win->controls[0] = (void *) ctl;
  }
  
  win->controln++;
}

//-- Draw Window
void aw_draw(AWINDOWP win) {
  if (!win->isActived) {
    return;
  }
  
  ag_draw(NULL, &win->c, 0, 0);
  ag_sync();
}

//-- Redraw Window & Controls
void aw_redraw_ex(AWINDOWP win, byte syncnow) {
  if (!win->isActived) {
    return;
  }
  
  if (win->controln > 0) {
    int i;
    
    for (i = 0; i < win->controln; i++) {
      ACONTROLP ctl = (ACONTROLP) win->controls[i];
      
      if (ctl->ondraw != NULL) {
        ctl->ondraw(ctl);
      }
    }
  }
  
  if (syncnow) {
    ag_draw(NULL, &win->c, 0, 0);
  }
}

void aw_redraw(AWINDOWP win) {
  aw_redraw_ex(win, 1);
}

//-- Show Window
void aw_show_ex2(AWINDOWP win, byte anitype, int x, int pos, int w, int h, ACONTROLP firstFocus) {
  win->threadnum    = 0;
  win->isActived    = 1;
  
  //-- Find First Focus
  if (win->controln > 0) {
    int i;
    
    if (firstFocus != NULL) {
      for (i = 0; i < win->controln; i++) {
        ACONTROLP ctl = (ACONTROLP) win->controls[i];
        
        if (firstFocus == ctl) {
          if (ctl->onfocus != NULL) {
            if (ctl->onfocus(ctl)) {
              win->focusIndex = i;
              break;
            }
          }
        }
      }
    }
    
    if (win->focusIndex == -1) {
      for (i = 0; i < win->controln; i++) {
        ACONTROLP ctl = (ACONTROLP) win->controls[i];
        
        if (ctl->onfocus != NULL) {
          if (ctl->onfocus(ctl)) {
            win->focusIndex = i;
            break;
          }
        }
      }
    }
  }
  
  if (acfg()->fadeframes > 0) {
    if (anitype == 0) {
      //-- Fade
      aw_redraw(win);
      ag_sync_fade(acfg_var.fadeframes);
    }
    else if (anitype == 1) {
      //-- Bottom Top
      aw_redraw_ex(win, 0);
      int drawh  = agh() - pos;
      int anisz = floor(((float) drawh) / acfg()->fadeframes);
      int i;
      
      for (i = 1; i <= acfg()->fadeframes; i++) {
        ag_draw_ex(NULL, &win->c, 0, agh() - (anisz * i), 0, pos, agw(), drawh);
        ag_sync();
      }
      
      ag_draw(NULL, &win->c, 0, 0);
      ag_sync();
    }
    else if (anitype == 2) {
      //-- Right Left
      aw_redraw_ex(win, 0);
      int drawh  = agh() - pos;
      int anisz = floor(((float) agw()) / acfg()->fadeframes);
      int i;
      CANVAS cbg;
      ag_canvas(&cbg, agw(), agh());
      ag_draw(&cbg, agc(), 0, 0);
      
      for (i = 1; i <= acfg()->fadeframes; i++) {
        ag_draw_ex(NULL, &cbg, 0 - (anisz * i), pos, 0, pos, agw(), drawh);
        ag_draw_ex(NULL, &win->c, agw() - (anisz * i), pos, 0, pos, agw(), drawh);
        ag_sync();
      }
      
      ag_ccanvas(&cbg);
      ag_draw(NULL, &win->c, 0, 0);
      ag_sync();
    }
    else if (anitype == 3) {
      //-- Left Right
      aw_redraw_ex(win, 0);
      int drawh  = agh() - pos;
      int anisz = floor(((float) agw()) / acfg()->fadeframes);
      int i;
      CANVAS cbg;
      ag_canvas(&cbg, agw(), agh());
      ag_draw(&cbg, agc(), 0, 0);
      
      for (i = 1; i <= acfg()->fadeframes; i++) {
        ag_draw_ex(NULL, &cbg, (anisz * i), pos, 0, pos, agw(), drawh);
        ag_draw_ex(NULL, &win->c, 0 - (agw() - (anisz * i)), pos, 0, pos, agw(), drawh);
        ag_sync();
      }
      
      ag_ccanvas(&cbg);
      ag_draw(NULL, &win->c, 0, 0);
      ag_sync();
    }
    else if (anitype == 5) {
      //-- Scale
      byte fadesz = acfg()->fadeframes;
      ag_sync();
      aw_redraw_ex(win, 0);
      CANVAS cbg;
      ag_canvas(&cbg, agw(), agh());
      ag_draw(&cbg, agc(), 0, 0);
      int xc = w / 2;
      int yc = h / 2;
      int i;
      CANVAS * tmpb = (CANVAS *) malloc(sizeof(CANVAS) * fadesz);
      memset(tmpb, 0, sizeof(CANVAS)*fadesz);
      
      for (i = 1; i <= fadesz; i++) {
        /* Calculating Scale */
        byte scale  = (i * 0xff) / fadesz;
        scale = (scale * (0x200 - scale)) >> 8;
        byte scale2 = ((scale * 0x80) >> 8) + 0x80;
        int wtarget = (w * scale2) >> 8;
        int htarget = (h * scale2) >> 8;
        ag_canvas(&tmpb[i - 1], w, h);
        ag_draw_ex(&tmpb[i - 1], &cbg, 0, 0, x, pos, w, h);
        ag_draw_strecth_ex(
          &tmpb[i - 1],
          &win->c,
          xc - wtarget / 2, yc - htarget / 2, wtarget, htarget,
          x, pos, w, h, scale, 1
        );
      }
      
      ag_ccanvas(&cbg);
      
      for (i = 0; i < fadesz; i++) {
        ag_draw(NULL, &tmpb[i], x, pos);
        ag_ccanvas(&tmpb[i]);
        ag_sync();
      }
      
      free(tmpb);
      ag_draw(NULL, &win->c, 0, 0);
      ag_sync();
    }
    else if (anitype == 6) {
      //-- Stack Right to Left
      int fadesz = acfg()->fadeframes;
      ag_sync();
      aw_redraw_ex(win, 0);
      CANVAS cbg;
      ag_canvas(&cbg, agw(), agh());
      ag_draw(&cbg, agc(), 0, 0);
      int xc = w / 2;
      int yc = h / 2;
      int i;
      CANVAS * tmpb = (CANVAS *) malloc(sizeof(CANVAS) * fadesz);
      memset(tmpb, 0, sizeof(CANVAS)*fadesz);
      
      for (i = 1; i <= fadesz; i++) {
        byte scale  = (i * 0xff) / fadesz;
        byte scale2 = ((scale * 0x80) >> 8) + 0x80;
        int wtarget = (w * scale2) >> 8;
        int htarget = (h * scale2) >> 8;
        int xtarget = (w * scale) >> 8;
        ag_canvas(&tmpb[i - 1], w, h);
        ag_draw_strecth_ex(
          &tmpb[i - 1],
          &win->c,
          w - wtarget, yc - htarget / 2, wtarget, htarget,
          x, pos, w, h, scale, 0
        );
        ag_draw_ex(&tmpb[i - 1], &cbg, 0, 0, x + xtarget, pos, w - xtarget, h);
      }
      
      ag_ccanvas(&cbg);
      
      for (i = 0; i < fadesz; i++) {
        ag_draw(NULL, &tmpb[i], x, pos);
        ag_ccanvas(&tmpb[i]);
        ag_sync();
      }
      
      free(tmpb);
      ag_draw(NULL, &win->c, 0, 0);
      ag_sync();
    }
    else if (anitype == 7) {
      //-- Stack Right to Left
      int fadesz = acfg()->fadeframes;
      ag_sync();
      aw_redraw_ex(win, 0);
      CANVAS cbg;
      ag_canvas(&cbg, agw(), agh());
      ag_draw(&cbg, agc(), 0, 0);
      int xc = w / 2;
      int yc = h / 2;
      int i;
      CANVAS * tmpb = (CANVAS *) malloc(sizeof(CANVAS) * fadesz);
      memset(tmpb, 0, sizeof(CANVAS)*fadesz);
      
      for (i = 1; i <= fadesz; i++) {
        byte scale  = (i * 0xff) / fadesz;
        byte scale2 = ((scale * 0x80) >> 8) + 0x80;
        int wtarget = (w * scale2) >> 8;
        int htarget = (h * scale2) >> 8;
        int xtarget = (w * scale) >> 8;
        ag_canvas(&tmpb[i - 1], w, h);
        ag_draw_strecth_ex(
          &tmpb[i - 1],
          &cbg,
          w - wtarget, yc - htarget / 2, wtarget, htarget,
          x, pos, w, h, scale, 0
        );
        ag_draw_ex(&tmpb[i - 1], &win->c, 0, 0, x + xtarget, pos, w - xtarget, h);
      }
      
      ag_ccanvas(&cbg);
      
      for (i = fadesz - 1; i >= 0; i--) {
        ag_draw(NULL, &tmpb[i], x, pos);
        ag_ccanvas(&tmpb[i]);
        ag_sync();
      }
      
      free(tmpb);
      ag_draw(NULL, &win->c, 0, 0);
      ag_sync();
    }
    else {
      //-- No Effect
      aw_redraw(win);
      ag_sync();
    }
  }
  else {
    //-- No Effect
    aw_redraw(win);
    ag_sync();
  }
  
  ui_clear_key_queue_ex();
}
void aw_show_ex(AWINDOWP win, byte anitype, int pos, ACONTROLP firstFocus) {
  aw_show_ex2(win, anitype, 0, pos, agw(), agh(), firstFocus);
}
//-- Show Window
void aw_show(AWINDOWP win) {
  aw_show_ex(win, 0, 0, NULL);
}

//-- Show Window
/*
void aw_show(AWINDOWP win) {
  win->threadnum    = 0;
  win->isActived    = 1;

  //-- Find First Focus
  if (win->controln > 0) {
    int i;

    for (i = 0; i < win->controln; i++) {
      ACONTROLP ctl = (ACONTROLP) win->controls[i];

      if (ctl->onfocus != NULL) {
        if (ctl->onfocus(ctl)) {
          win->focusIndex = i;
          break;
        }
      }
    }
  }

  aw_redraw(win);
  ag_sync_fade(acfg_var.fadeframes);
}
*/

//-- Post Message
void aw_post(dword msg) {
  atouch_send_message(msg);
}

//-- Check Mouse Event
byte aw_touchoncontrol(ACONTROLP ctl, int x, int y) {
  int wx  = ctl->x;
  int wx2 = wx + ctl->w;
  int wy  = ctl->y;
  int wy2 = wy + ctl->h;
  
  if ((x >= wx) && (x < wx2) && (y >= wy) && (y < wy2)) {
    return 1;
  }
  
  return 0;
}

//-- Set Focus
byte aw_setfocus(AWINDOWP win, ACONTROLP ctl) {
  if (!win->isActived) {
    return 0;
  }
  
  int i;
  
  for (i = 0; i < win->controln; i++) {
    ACONTROLP fctl = (ACONTROLP) win->controls[i];
    
    if (fctl == ctl) {
      if (fctl->onfocus != NULL) {
        if (fctl->onfocus(fctl)) {
          int pf = win->focusIndex;
          win->focusIndex = i;
          
          if ((pf != -1) && (pf != i)) {
            ACONTROLP pctl = (ACONTROLP) win->controls[pf];
            pctl->onblur(pctl);
          }
          
          aw_draw(win);
          return 1;
        }
      }
    }
  }
  
  return 0;
}

//-- Dispatch Messages
dword aw_dispatch(AWINDOWP win) {
  dword msg;
  int i;
  // ui_clear_key_queue();
  
  while (1) {
    //-- Wait For Event
    ATEV        atev;
    int action  = atouch_wait(&atev);
    //-- Reset Message Value
    msg         = aw_msg(0, 0, 0, 0);
    
    //-- Check an Action Value
    switch (action) {
      case ATEV_MESSAGE: {
          msg = atev.msg;
        }
        break;
        
      case ATEV_MENU: {
          if (!atev.d) {
            if (!on_dialog_window) {
              byte resmenu = aw_showmenu(win);
              
              if (resmenu == 2) {
                msg = aw_msg(4, 0, 0, 0);
              }
            }
            else if (on_dialog_window == 2) {
              msg = aw_msg(5, 0, 0, 0);
            }
          }
        }
        break;
        
      case ATEV_BACK: {
          if (!atev.d) {
            msg = aw_msg(5, 0, 0, 0);
          }
        }
        break;
        
      case ATEV_DOWN:
      case ATEV_RIGHT:
        if (!atev.d) {
          if (win->focusIndex != -1) {
            ACONTROLP ctl = (ACONTROLP) win->controls[win->focusIndex];
            
            if (ctl->oninput != NULL) {
              msg = ctl->oninput((void *)ctl, action, &atev);
            }
            
            if (aw_gl(msg) == 0) {
              for (i = win->focusIndex + 1; i < win->controln; i++) {
                ACONTROLP fctl = (ACONTROLP) win->controls[i];
                
                if (fctl->onfocus != NULL) {
                  if (fctl->onfocus(fctl)) {
                    win->focusIndex = i;
                    ctl->onblur(ctl);
                    aw_draw(win);
                    break;
                  }
                }
              }
            }
          }
        }
        
        break;
        
      case ATEV_UP:
      case ATEV_LEFT:
        if (!atev.d) {
          if (win->focusIndex != -1) {
            ACONTROLP ctl = (ACONTROLP) win->controls[win->focusIndex];
            
            if (ctl->oninput != NULL) {
              msg = ctl->oninput((void *)ctl, action, &atev);
            }
            
            if (aw_gl(msg) == 0) {
              for (i = win->focusIndex - 1; i >= 0; i--) {
                ACONTROLP fctl = (ACONTROLP) win->controls[i];
                
                if (fctl->onfocus != NULL) {
                  if (fctl->onfocus(fctl)) {
                    win->focusIndex = i;
                    ctl->onblur(ctl);
                    aw_draw(win);
                    break;
                  }
                }
              }
            }
          }
        }
        
        break;
        
      case ATEV_SELECT: {
          if (win->focusIndex != -1) {
            ACONTROLP ctl = (ACONTROLP) win->controls[win->focusIndex];
            
            if (ctl->oninput != NULL) {
              msg = ctl->oninput((void *)ctl, action, &atev);
            }
          }
        }
        break;
        
      case ATEV_MOUSEDN: {
          if (win->controln > 0) {
            int i;
            
            for (i = win->controln - 1; i >= 0; i--) {
              ACONTROLP ctl = (ACONTROLP) win->controls[i];
              
              if (aw_touchoncontrol(ctl, atev.x, atev.y)) {
                if (ctl->oninput != NULL) {
                  msg             = ctl->oninput((void *)ctl, action, &atev);
                  win->touchIndex = i;
                  break;
                }
              }
            }
          }
        }
        break;
        
      case ATEV_MOUSEUP: {
          if (win->touchIndex != -1) {
            ACONTROLP ctl = (ACONTROLP) win->controls[win->touchIndex];
            
            if (ctl->oninput != NULL) {
              msg             = ctl->oninput((void *)ctl, action, &atev);
            }
            
            win->touchIndex   = -1;
          }
        }
        break;
        
      case ATEV_MOUSEMV: {
          if (win->touchIndex != -1) {
            ACONTROLP ctl = (ACONTROLP) win->controls[win->touchIndex];
            
            if (ctl->oninput != NULL) {
              msg             = ctl->oninput((void *)ctl, action, &atev);
            }
          }
        }
        break;
    }
    
    if (aw_gm(msg) == 200) {
      if (!atev.d) {
        if (!on_dialog_window) {
          byte resmenu = aw_showmenu(win);
          
          if (resmenu == 2) {
            msg = aw_msg(4, 0, 0, 0);
          }
        }
        else if (on_dialog_window == 2) {
          msg = aw_msg(5, 0, 0, 0);
        }
      }
    }
    
    if (aw_gd(msg) == 1) {
      aw_draw(win);
    }
    
    if (aw_gm(msg) != 0) {
      return msg;
    }
  }
  
  return msg;
}
CANVAS * aw_muteparent(AWINDOWP win) {
  if (win == NULL) {
    //-- Set Temporary
    CANVAS * tmpbg = (CANVAS *) malloc(sizeof(CANVAS));
    ag_canvas(tmpbg, agw(), agh());
    ag_draw(tmpbg, agc(), 0, 0);
    return tmpbg;
  }
  else {
    win->isActived = 0;
    return NULL;
  }
}
CANVAS * aw_maskparent() {
  CANVAS * tmpbg = (CANVAS *) malloc(sizeof(CANVAS));
  ag_canvas(tmpbg, agw(), agh());
  ag_draw(tmpbg, agc(), 0, 0);
  ag_rectopa(tmpbg, 0, 0, agw(), agh(), 0x0000, 180);
  ag_draw(agc(), tmpbg, 0, 0);
  return tmpbg;
}
void aw_unmuteparent(AWINDOWP win, CANVAS * p) {
  if (win == NULL) {
    if (p != NULL) {
      ag_draw(NULL, p, 0, 0);
      //ag_sync_fade(acfg_var.fadeframes);
      ag_sync();
      ag_ccanvas(p);
      free(p);
    }
  }
  else {
    if (p != NULL) {
      ag_ccanvas(p);
      free(p);
    }
    
    win->isActived = 1;
    ag_draw(NULL, &win->c, 0, 0);
    //ag_sync_fade(acfg_var.fadeframes);
    ag_sync();
  }
}
void aw_unmaskparent(AWINDOWP win, CANVAS * p, CANVAS * maskc, int x, int y, int w, int h) {
  if (maskc != NULL) {
    CANVAS * wincanvas = NULL;
    
    if (win == NULL) {
      if (p != NULL) {
        wincanvas = p;
      }
      else {
        if (maskc != NULL) {
          ag_ccanvas(maskc);
          free(maskc);
        }
        
        aw_unmuteparent(win, p);
        return;
      }
    }
    else {
      wincanvas = &win->c;
    }
    
    int fadesz = acfg()->fadeframes;
    
    if (fadesz > 0) {
      //-- Current Canvas
      CANVAS cbg;
      ag_canvas(&cbg, agw(), agh());
      ag_draw(&cbg, agc(), 0, 0);
      int xc    = w / 2;
      int yc    = h / 2;
      int i;
      CANVAS * tmpb = (CANVAS *) malloc(sizeof(CANVAS) * fadesz);
      memset(tmpb, 0, sizeof(CANVAS)*fadesz);
      
      for (i = 1; i <= fadesz; i++) {
        /* Calculating Scale */
        byte scale  = (i * 0xff) / fadesz;
        scale = (scale * (0x200 - scale)) >> 8;
        byte scale2 = ((scale * 0x80) >> 8) + 0x80;
        int wtarget = (w * scale2) >> 8;
        int htarget = (h * scale2) >> 8;
        ag_canvas(&tmpb[i - 1], w, h);
        ag_draw_ex(&tmpb[i - 1], maskc, 0, 0, x, y, w, h);
        ag_draw_strecth_ex(
          &tmpb[i - 1],
          &cbg,
          xc - wtarget / 2, yc - htarget / 2, wtarget, htarget,
          x, y, w, h, scale, 1
        );
      }
      
      ag_ccanvas(&cbg);
      
      for (i = fadesz - 1; i >= 0; i--) {
        ag_draw(NULL, &tmpb[i], x, y);
        ag_ccanvas(&tmpb[i]);
        ag_sync();
      }
      
      free(tmpb);
    }
    
    ag_ccanvas(maskc);
    free(maskc);
    aw_unmuteparent(win, p);
  }
  else {
    aw_unmuteparent(win, p);
  }
}
void aw_textdialog(AWINDOWP parent, char * titlev, char * text, char * ok_text) {
  // actext(hWin,txtX,txtY,txtW,txtH,text,0);
  CANVAS * tmpc = aw_muteparent(parent);
  //-- Set Mask
  on_dialog_window = 1;
  CANVAS * maskc = aw_maskparent();
  //ag_sync();
  //ag_sync_fade(acfg_var.fadeframes);
  char title[64];
  snprintf(title, 64, "%s", titlev);
  int pad   = agdp() * 4;
  int winW  = agw() - (pad * 2);
  int txtW  = winW - (pad * 2);
  int txtX  = pad * 2;
  int btnH  = agdp() * 20;
  int titW  = ag_txtwidth(title, 1);
  int titH  = ag_fontheight(1) + (pad * 2);
  PNGCANVASP winp = atheme("img.dialog");
  PNGCANVASP titp = atheme("img.dialog.titlebar");
  APNG9      winv;
  APNG9      titv;
  int vtitY = -1;
  int vpadB = pad;
  int vimgX = pad * 2;
  
  if (titp != NULL) {
    if (apng9_calc(titp, &titv, 1)) {
      int tmptitH = titH - (pad * 2);
      titH        = tmptitH + (titv.t + titv.b);
      vtitY       = titv.t;
    }
  }
  
  if (winp != NULL) {
    if (apng9_calc(winp, &winv, 1)) {
      txtW = winW - (winv.l + winv.r);
      txtX = pad  + (winv.l);
      vimgX = pad  + (winv.l);
      vpadB = winv.b;
    }
  }
  
  byte imgE = 0;
  int imgW = 0;
  int imgH = 0;
  int txtH    = agh() / 2;
  int infH    = txtH;
  //-- Calculate Window Size & Position
  int winH    = titH + infH + btnH + (pad * 2) + vpadB;
  int winX    = pad;
  int winY    = (agh() / 2) - (winH / 2);
  //-- Calculate Title Size & Position
  int titX    = (agw() / 2) - (titW / 2);
  int titY    = winY + pad;
  
  if (vtitY != -1) {
    titY = winY + vtitY;
  }
  
  //-- Calculate Text Size & Position
  int infY    = winY + titH + pad;
  int txtY    = infY;
  //-- Calculate Button Size & Position
  int btnW    = winW / 2;
  int btnY    = infY + infH + pad;
  int btnX    = (agw() / 2) - (btnW / 2);
  //-- Initializing Canvas
  CANVAS alertbg;
  ag_canvas(&alertbg, agw(), agh());
  ag_draw(&alertbg, agc(), 0, 0);
  
  //-- Draw Window
  if (!atheme_draw("img.dialog", &alertbg, winX, winY, winW, winH)) {
    ag_roundgrad(&alertbg, winX - 1, winY - 1, winW + 2, winH + 2, acfg_var.border, acfg_var.border_g, (acfg_var.roundsz * agdp()) + 1);
    ag_roundgrad(&alertbg, winX, winY, winW, winH, acfg_var.dialogbg, acfg_var.dialogbg_g, acfg_var.roundsz * agdp());
  }
  
  //-- Draw Title
  if (!atheme_draw("img.dialog.titlebar", &alertbg, winX, winY, winW, titH)) {
    ag_roundgrad_ex(&alertbg, winX, winY, winW, titH, acfg_var.dlgtitlebg, acfg_var.dlgtitlebg_g, acfg_var.roundsz * agdp(), 1, 1, 0, 0);
  }
  
  ag_textf(&alertbg, titW, titX + 1, titY + 1, title, acfg_var.dlgtitlebg_g, 1);
  ag_text(&alertbg, titW, titX, titY, title, acfg_var.dlgtitlefg, 1);
  AWINDOWP hWin   = aw(&alertbg);
  actext(hWin, txtX, txtY, txtW, txtH, text, 0);
  ACONTROLP okbtn = acbutton(hWin, btnX, btnY, btnW, btnH, (ok_text == NULL ? acfg_var.text_ok : ok_text), 0, 5);
  // aw_show_ex(hWin, 5, 0, okbtn);
  aw_show_ex2(hWin, 5, winX - 1, winY - 1, winW + 2, winH + 2, okbtn);
  /*
  aw_show(hWin);
  aw_setfocus(hWin, okbtn);
  */
  byte ondispatch = 1;
  
  while (ondispatch) {
    dword msg = aw_dispatch(hWin);
    
    switch (aw_gm(msg)) {
      case 5:
        ondispatch = 0;
        break;
    }
  }
  
  aw_destroy(hWin);
  ag_ccanvas(&alertbg);
  on_dialog_window = 0;
  aw_unmaskparent(parent, tmpc, maskc, winX - 1, winY - 1, winW + 2, winH + 2);
}
void aw_alert(AWINDOWP parent, char * titlev, char * textv, char * img, char * ok_text) {
  CANVAS * tmpc = aw_muteparent(parent);
  //-- Set Mask
  on_dialog_window = 1;
  CANVAS * maskc = aw_maskparent();
  //ag_sync();
  //ag_sync_fade(acfg_var.fadeframes);
  char title[32];
  char text[512];
  snprintf(title, 32, "%s", titlev);
  snprintf(text, 512, "%s", textv);
  int pad   = agdp() * 4;
  int winW  = agw() - (pad * 2);
  int txtW  = winW - (pad * 2);
  int txtX  = pad * 2;
  int btnH  = agdp() * 20;
  int titW  = ag_txtwidth(title, 1);
  int titH  = ag_fontheight(1) + (pad * 2);
  PNGCANVASP winp = atheme("img.dialog");
  PNGCANVASP titp = atheme("img.dialog.titlebar");
  APNG9      winv;
  APNG9      titv;
  int vtitY = -1;
  int vpadB = -1;
  int vimgX = pad * 2;
  
  if (titp != NULL) {
    if (apng9_calc(titp, &titv, 1)) {
      int tmptitH = titH - (pad * 2);
      titH        = tmptitH + (titv.t + titv.b);
      vtitY       = titv.t;
    }
  }
  
  if (winp != NULL) {
    if (apng9_calc(winp, &winv, 1)) {
      txtW = winW - (winv.l + winv.r);
      txtX = pad  + (winv.l);
      vimgX = pad  + (winv.l);
      vpadB = winv.b;
    }
  }
  
  //-- Load Icon
  PNGCANVAS ap;
  byte imgE = 0;
  int imgW = 0;
  int imgH = 0;
  
  if (apng_load(&ap, img)) {
    imgE      = 1;
    imgW      = min(ap.w, agdp() * 30);
    imgH      = min(ap.h, agdp() * 30);
    int imgA  = pad + imgW;
    txtX     += imgA;
    txtW     -= imgA;
  }
  
  int txtH    = ag_txtheight(txtW, text, 0);
  int infH    = ((imgE) && (txtH < imgH)) ? imgH : txtH;
  //-- Calculate Window Size & Position
  int winH    = titH + infH + btnH + (pad * 3);
  
  if (vpadB != -1) {
    winH    = titH + infH + btnH + (pad * 2) + vpadB;
  }
  
  int winX    = pad;
  int winY    = (agh() / 2) - (winH / 2);
  //-- Calculate Title Size & Position
  int titX    = (agw() / 2) - (titW / 2);
  int titY    = winY + pad;
  
  if (vtitY != -1) {
    titY = winY + vtitY;
  }
  
  //-- Calculate Text Size & Position
  int infY    = winY + titH + pad;
  int txtY    = infY + ((infH - txtH) / 2);
  int imgY    = infY;
  //-- Calculate Button Size & Position
  int btnW    = winW / 2;
  int btnY    = infY + infH + pad;
  int btnX    = (agw() / 2) - (btnW / 2);
  //-- Initializing Canvas
  CANVAS alertbg;
  ag_canvas(&alertbg, agw(), agh());
  ag_draw(&alertbg, agc(), 0, 0);
  
  //-- Draw Window
  if (!atheme_draw("img.dialog", &alertbg, winX, winY, winW, winH)) {
    ag_roundgrad(&alertbg, winX - 1, winY - 1, winW + 2, winH + 2, acfg_var.border, acfg_var.border_g, (acfg_var.roundsz * agdp()) + 1);
    ag_roundgrad(&alertbg, winX, winY, winW, winH, acfg_var.dialogbg, acfg_var.dialogbg_g, acfg_var.roundsz * agdp());
  }
  
  //-- Draw Title
  if (!atheme_draw("img.dialog.titlebar", &alertbg, winX, winY, winW, titH)) {
    ag_roundgrad_ex(&alertbg, winX, winY, winW, titH, acfg_var.dlgtitlebg, acfg_var.dlgtitlebg_g, acfg_var.roundsz * agdp(), 1, 1, 0, 0);
  }
  
  ag_textf(&alertbg, titW, titX + 1, titY + 1, title, acfg_var.dlgtitlebg_g, 1);
  ag_text(&alertbg, titW, titX, titY, title, acfg_var.dlgtitlefg, 1);
  
  //-- Draw Image
  if (imgE) {
    apng_draw_ex(&alertbg, &ap, vimgX, imgY, 0, 0, imgW, imgH);
    apng_close(&ap);
  }
  
  //-- Draw Text
  ag_textf(&alertbg, txtW, txtX + 1, txtY + 1, text, acfg_var.dialogbg, 0);
  ag_text(&alertbg, txtW, txtX, txtY, text, acfg_var.dialogfg, 0);
  AWINDOWP hWin   = aw(&alertbg);
  acbutton(hWin, btnX, btnY, btnW, btnH, (ok_text == NULL ? acfg_var.text_ok : ok_text), 0, 5);
  // aw_show_ex(hWin, 5, 0, NULL);
  aw_show_ex2(hWin, 5, winX - 1, winY - 1, winW + 2, winH + 2, NULL);
  /*
  aw_show(hWin);
  */
  byte ondispatch = 1;
  
  while (ondispatch) {
    dword msg = aw_dispatch(hWin);
    
    switch (aw_gm(msg)) {
      case 5:
        ondispatch = 0;
        break;
    }
  }
  
  aw_destroy(hWin);
  ag_ccanvas(&alertbg);
  on_dialog_window = 0;
  aw_unmaskparent(parent, tmpc, maskc, winX - 1, winY - 1, winW + 2, winH + 2);
}
byte aw_confirm(AWINDOWP parent, char * titlev, char * textv, char * img, char * yes_text, char * no_text) {
  CANVAS * tmpc = aw_muteparent(parent);
  //-- Set Mask
  on_dialog_window = 1;
  CANVAS * maskc = aw_maskparent();
  //ag_sync();
  //ag_sync_fade(acfg_var.fadeframes);
  char title[64];
  char text[512];
  snprintf(title, 64, "%s", titlev);
  snprintf(text, 512, "%s", textv);
  int pad   = agdp() * 4;
  int winW  = agw() - (pad * 2);
  int txtW  = winW - (pad * 2);
  int txtX  = pad * 2;
  int btnH  = agdp() * 20;
  int titW  = ag_txtwidth(title, 1);
  int titH  = ag_fontheight(1) + (pad * 2);
  PNGCANVASP winp = atheme("img.dialog");
  PNGCANVASP titp = atheme("img.dialog.titlebar");
  APNG9      winv;
  APNG9      titv;
  int vtitY = -1;
  int vpadB = -1;
  int vimgX = pad * 2;
  
  if (titp != NULL) {
    if (apng9_calc(titp, &titv, 1)) {
      int tmptitH = titH - (pad * 2);
      titH        = tmptitH + (titv.t + titv.b);
      vtitY       = titv.t;
    }
  }
  
  if (winp != NULL) {
    if (apng9_calc(winp, &winv, 1)) {
      txtW = winW - (winv.l + winv.r);
      txtX = pad  + (winv.l);
      vimgX = pad  + (winv.l);
      vpadB = winv.b;
    }
  }
  
  //-- Load Icon
  PNGCANVAS ap;
  byte imgE = 0;
  int imgW = 0;
  int imgH = 0;
  
  if (apng_load(&ap, img)) {
    imgE      = 1;
    imgW      = min(ap.w, agdp() * 30);
    imgH      = min(ap.h, agdp() * 30);
    int imgA  = pad + imgW;
    txtX     += imgA;
    txtW     -= imgA;
  }
  
  int txtH    = ag_txtheight(txtW, text, 0);
  int infH    = ((imgE) && (txtH < imgH)) ? imgH : txtH;
  //-- Calculate Window Size & Position
  int winH    = titH + infH + btnH + (pad * 3);
  
  if (vpadB != -1) {
    winH    = titH + infH + btnH + (pad * 2) + vpadB;
  }
  
  int winX    = pad;
  int winY    = (agh() / 2) - (winH / 2);
  //-- Calculate Title Size & Position
  int titX    = (agw() / 2) - (titW / 2);
  int titY    = winY + pad;
  
  if (vtitY != -1) {
    titY = winY + vtitY;
  }
  
  //-- Calculate Text Size & Position
  int infY    = winY + titH + pad;
  int txtY    = infY + ((infH - txtH) / 2);
  int imgY    = infY;
  //-- Calculate Button Size & Position
  int btnW    = (txtW / 2) - (pad / 2);
  int btnY    = infY + infH + pad;
  int btnX    = txtX;
  int btnX2   = txtX + (txtW / 2) + (pad / 2);
  //-- Initializing Canvas
  CANVAS alertbg;
  ag_canvas(&alertbg, agw(), agh());
  ag_draw(&alertbg, agc(), 0, 0);
  
  //-- Draw Window
  if (!atheme_draw("img.dialog", &alertbg, winX - 1, winY - 1, winW + 2, winH + 2)) {
    ag_roundgrad(&alertbg, winX - 1, winY - 1, winW + 2, winH + 2, acfg_var.border, acfg_var.border_g, (acfg_var.roundsz * agdp()) + 1);
    ag_roundgrad(&alertbg, winX, winY, winW, winH, acfg_var.dialogbg, acfg_var.dialogbg_g, acfg_var.roundsz * agdp());
  }
  
  //-- Draw Title
  if (!atheme_draw("img.dialog.titlebar", &alertbg, winX, winY, winW, titH)) {
    ag_roundgrad_ex(&alertbg, winX, winY, winW, titH, acfg_var.dlgtitlebg, acfg_var.dlgtitlebg_g, acfg_var.roundsz * agdp(), 1, 1, 0, 0);
  }
  
  ag_textf(&alertbg, titW, titX + 1, titY + 1, title, acfg_var.dlgtitlebg_g, 1);
  ag_text(&alertbg, titW, titX, titY, title, acfg_var.dlgtitlefg, 1);
  
  //-- Draw Image
  if (imgE) {
    apng_draw_ex(&alertbg, &ap, vimgX, imgY, 0, 0, imgW, imgH);
    apng_close(&ap);
  }
  
  //-- Draw Text
  ag_textf(&alertbg, txtW, txtX + 1, txtY + 1, text, acfg_var.dialogbg, 0);
  ag_text(&alertbg, txtW, txtX, txtY, text, acfg_var.dialogfg, 0);
  AWINDOWP hWin   = aw(&alertbg);
  acbutton(hWin, btnX, btnY, btnW, btnH, (yes_text == NULL ? acfg_var.text_yes : yes_text), 0, 6);
  acbutton(hWin, btnX2, btnY, btnW, btnH, (no_text == NULL ? acfg_var.text_no : no_text), 0, 5);
  //aw_show_ex(hWin, 5, 0, NULL);
  aw_show_ex2(hWin, 5, winX - 1, winY - 1, winW + 2, winH + 2, NULL);
  byte ondispatch = 1;
  byte res = 0;
  
  while (ondispatch) {
    dword msg = aw_dispatch(hWin);
    
    switch (aw_gm(msg)) {
      case 6:
        res = 1;
        ondispatch = 0;
        break;
        
      case 5:
        ondispatch = 0;
        break;
    }
  }
  
  aw_destroy(hWin);
  ag_ccanvas(&alertbg);
  on_dialog_window = 0;
  aw_unmaskparent(parent, tmpc, maskc, winX - 1, winY - 1, winW + 2, winH + 2);
  return res;
}
void aw_help_dialog(AWINDOWP parent) {
}
byte aw_calibdraw(CANVAS * c,
                  int id, int * xpos, int * ypos, int * xtch, int * ytch) {
  return 0;
  /*
  ag_draw(agc(), c, 0, 0);
  // usleep(500000);
  int sz = agdp() * 10;
  int ty2 = 0;
  
  if (id == -2) {
    char * txt  =
      "<@center><b>Fine Tuning</b>\n"
      "Use volume button to fine tuning your calibration.\n"
      "Increase if it flicked while hold your finger.\n"
      "Reduce if it sticked while release your finger.\n"
      "<b>NOTE:</b> Smaller value mean more responsive</@>\n\n";
    char * txt2 = "Press Power Key or Other Keys To Continue";
    int txh = ag_txtheight(agw(), txt, 0);
    int tw = ag_txtwidth(txt, 0);
    int tw2 = ag_txtwidth(txt2, 0);
    int tx = (agw() / 2) - (tw / 2);
    int tx2 = (agw() / 2) - (tw2 / 2);
    int ty = (agh() / 2) + (sz * 2);
    ty2 = (ty + ag_fontheight(0) + agdp());
    ty -= txh - ag_fontheight(0);
    ag_text(agc(), agw(), 1, ty + 1, txt, 0x0000, 0);
    ag_text(agc(), agw(), 0, ty, txt, 0xffff, 0);
    ag_text(agc(), tw2, tx2 + 1, ty2 + 1, txt2, 0x0000, 0);
    ag_text(agc(), tw2, tx2, ty2, txt2, 0xffff, 0);
  }
  else if (id != -1) {
    char txt[128];
    snprintf(txt, 128, "Step %i: Tap The Circle To Calibrate", id + 1);
    char * txt2 = "Press Power Key or Other Keys To Cancel";
    int tw = ag_txtwidth(txt, 0);
    int tw2 = ag_txtwidth(txt2, 0);
    int tx = (agw() / 2) - (tw / 2);
    int tx2 = (agw() / 2) - (tw2 / 2);
    int ty = (agh() / 2) + (sz * 2);
    ty2 = (ty + ag_fontheight(0) + agdp());
    ag_text(agc(), tw, tx + 1, ty + 1, txt, 0x0000, 0);
    ag_text(agc(), tw, tx, ty, txt, 0xffff, 0);
    ag_text(agc(), tw2, tx2 + 1, ty2 + 1, txt2, 0x0000, 0);
    ag_text(agc(), tw2, tx2, ty2, txt2, 0xffff, 0);
  }
  else {
    char * txt  = "Tap The Screen to Test Calibrated Data";
    char * txt2 = "Press Power or Other Keys To Continue";
    int tw = ag_txtwidth(txt, 0);
    int tw2 = ag_txtwidth(txt2, 0);
    int tx = (agw() / 2) - (tw / 2);
    int tx2 = (agw() / 2) - (tw2 / 2);
    int ty = (agh() / 2) + (sz * 2);
    ty2 = (ty + ag_fontheight(0) + agdp());
    ag_text(agc(), tw, tx + 1, ty + 1, txt, 0x0000, 0);
    ag_text(agc(), tw, tx, ty, txt, 0xffff, 0);
    ag_text(agc(), tw2, tx2 + 1, ty2 + 1, txt2, 0x0000, 0);
    ag_text(agc(), tw2, tx2, ty2, txt2, 0xffff, 0);
  }
  
  CANVAS bg;
  ag_canvas(&bg, agw(), agh());
  ag_draw(&bg, agc(), 0, 0);
  ag_sync();
  int rx = 0;
  int ry = 0;
  
  if ((id != -1) && (id != -2)) {
    int x  = xpos[id];
    int y  = ypos[id];
    rx     = x - (sz / 2);
    ry     = y - (sz / 2);
  
    if (id > 0) {
      int prx = xpos[id - 1];
      int pry = ypos[id - 1];
      int px  = prx - (sz / 2);
      int py  = pry - (sz / 2);
      int i;
      int addx = floor((rx - prx) / 10.0);
      int addy = floor((ry - pry) / 10.0);
  
      for (i = 0; i < 9; i++) {
        usleep(5000);
        ag_draw(agc(), &bg, 0, 0);
        prx += addx;
        pry += addy;
        ag_roundgrad(agc(), prx, pry, sz, sz,
                     0xffff,
                     ag_rgb(200, 200, 200),
                     sz / 2);
        ag_sync();
      }
  
      usleep(50000);
      ag_draw(agc(), &bg, 0, 0);
    }
  
    ag_roundgrad(
      agc(), rx, ry, sz, sz,
      0xffff,
      ag_rgb(200, 200, 200),
      sz / 2);
    ag_sync();
  }
  
  if (id == -2) {
    char txt3[256];
    snprintf(txt3, 256, "<b>Fine Tuning Value : %i</b>", *xpos);
    int ty3 = agh() - (ag_fontheight(0) + (agdp() * 6));
    int tw3 = ag_txtwidth(txt3, 0);
    int tx3 = (agw() / 2) - (tw3 / 2);
    ag_text(agc(), tw3, tx3, ty3, txt3, 0xffff, 0);
    ag_sync();
  }
  
  byte res = 1;
  byte ond = 1;
  byte onp = 0;
  ui_clear_key_queue();
  
  while (ond) {
    ATEV atev;
    // ui_clear_key_queue();
    int action = atouch_wait_ex(&atev, 1);
  
    switch (action) {
      case ATEV_MOUSEDN: {
          onp = 1;
  
          if ((id == -1) || (id == -2)) {
            ag_draw(agc(), &bg, 0, 0);
            int vz = agdp() * 30;
            int vx = atev.x - (vz / 2);
            int vy = atev.y - (vz / 2);
            char txt3[256];
            snprintf(txt3, 256, "<b>Coordinate: %i x %i</b>", atev.x, atev.y);
            int ty3 = ty2 + ag_fontheight(0) + (agdp() * 6);
            int tw3 = ag_txtwidth(txt3, 0);
            int tx3 = (agw() / 2) - (tw3 / 2);
            ag_text(agc(), tw3, tx3, ty3, txt3, 0xffff, 0);
            ag_rect(agc(), 0, atev.y, agw(), 1, 0xffff);
            ag_rect(agc(), atev.x, 0, 1, agh(), 0xffff);
            ag_roundgrad(agc(), vx, vy, vz, vz, 0xffff, ag_rgb(180, 180, 180), (vz / 2));
  
            if (id == -2) {
              snprintf(txt3, 256, "<b>Fine Tuning Value : %i</b>", *xpos);
              ty3 = agh() - (ag_fontheight(0) + (agdp() * 6));
              tw3 = ag_txtwidth(txt3, 0);
              tx3 = (agw() / 2) - (tw3 / 2);
              ag_text(agc(), tw3, tx3, ty3, txt3, 0xffff, 0);
            }
  
            ag_sync();
          }
          else {
            ag_draw(agc(), &bg, 0, 0);
            int agpl = agdp() * 10;
            ag_roundgrad(
              agc(), rx - agpl, ry - agpl, sz + (agpl * 2), sz + (agpl * 2),
              0xffff,
              ag_rgb(200, 200, 200),
              (sz / 2) + agpl);
            agpl = agdp() * 7;
            ag_roundgrad(
              agc(), rx - agpl, ry - agpl, sz + (agpl * 2), sz + (agpl * 2),
              ag_rgb(40, 40, 40),
              0x0000,
              (sz / 2) + agpl);
            agpl = 0;
            ag_roundgrad(
              agc(), rx - agpl, ry - agpl, sz + (agpl * 2), sz + (agpl * 2),
              0xffff,
              ag_rgb(200, 200, 200),
              (sz / 2) + agpl);
            ag_sync();
          }
        }
        break;
  
      case ATEV_MOUSEMV: {
          if (onp) {
            if ((id != -1) && (id != -2)) {
              xtch[id] = atev.x;
              ytch[id] = atev.y;
            }
            else {
              ag_draw(agc(), &bg, 0, 0);
              int vz = agdp() * 30;
              int vx = atev.x - (vz / 2);
              int vy = atev.y - (vz / 2);
              char txt3[256];
              snprintf(txt3, 256, "<b>Coordinate: %i x %i</b>", atev.x, atev.y);
              int ty3 = ty2 + ag_fontheight(0) + (agdp() * 6);
              int tw3 = ag_txtwidth(txt3, 0);
              int tx3 = (agw() / 2) - (tw3 / 2);
              ag_text(agc(), tw3, tx3, ty3, txt3, 0xffff, 0);
              ag_rect(agc(), 0, atev.y, agw(), 1, 0xffff);
              ag_rect(agc(), atev.x, 0, 1, agh(), 0xffff);
              ag_roundgrad(agc(), vx, vy, vz, vz, 0xffff, ag_rgb(180, 180, 180), (vz / 2));
  
              if (id == -2) {
                snprintf(txt3, 256, "<b>Fine Tuning Value : %i</b>", *xpos);
                ty3 = agh() - (ag_fontheight(0) + (agdp() * 6));
                tw3 = ag_txtwidth(txt3, 0);
                tx3 = (agw() / 2) - (tw3 / 2);
                ag_text(agc(), tw3, tx3, ty3, txt3, 0xffff, 0);
              }
  
              ag_sync();
            }
          }
        }
        break;
  
      case ATEV_MOUSEUP: {
          if ((id != -1) && (id != -2)) {
            if (onp) {
              if ((xtch[id] > 0) && (ytch[id] > 0)) {
                ond = 0;
              }
  
              onp = 0;
            }
          }
          else {
            ag_draw(agc(), &bg, 0, 0);
            int vz = agdp() * 40;
            int vx = atev.x - (vz / 2);
            int vy = atev.y - (vz / 2);
            char txt3[256];
            snprintf(txt3, 256, "<b>Release Coordinate: %i x %i</b>", atev.x, atev.y);
            int ty3 = ty2 + ag_fontheight(0) + (agdp() * 6);
            int tw3 = ag_txtwidth(txt3, 0);
            int tx3 = (agw() / 2) - (tw3 / 2);
            ag_text(agc(), tw3, tx3, ty3, txt3, 0xffff, 0);
  
            if (id == -2) {
              snprintf(txt3, 256, "<b>Fine Tuning Value : %i</b>", *xpos);
              ty3 = agh() - (ag_fontheight(0) + (agdp() * 6));
              tw3 = ag_txtwidth(txt3, 0);
              tx3 = (agw() / 2) - (tw3 / 2);
              ag_text(agc(), tw3, tx3, ty3, txt3, 0xffff, 0);
            }
  
            // ag_roundgrad(agc(),vx,vy,vz,vz,0xffff,ag_rgb(180,180,180),(vz/2));
            ag_sync();
          }
        }
        break;
  
      case ATEV_DOWN: {
          if (id == -2) {
            *xpos -= 1;
  
            if (*xpos < 1) {
              *xpos = 1;
            }
  
            atouch_sethack(*xpos);
            ag_draw(agc(), &bg, 0, 0);
            char txt3[256];
            snprintf(txt3, 256, "<b>Fine Tuning Value : %i</b>", *xpos);
            int ty3 = agh() - (ag_fontheight(0) + (agdp() * 6));
            int tw3 = ag_txtwidth(txt3, 0);
            int tx3 = (agw() / 2) - (tw3 / 2);
            ag_text(agc(), tw3, tx3, ty3, txt3, 0xffff, 0);
            ag_sync();
          }
        }
        break;
  
      case ATEV_UP: {
          if (id == -2) {
            *xpos += 1;
  
            if (*xpos > 100) {
              *xpos = 100;
            }
  
            atouch_sethack(*xpos);
            ag_draw(agc(), &bg, 0, 0);
            char txt3[256];
            snprintf(txt3, 256, "<b>Fine Tuning Value : %i</b>", *xpos);
            int ty3 = agh() - (ag_fontheight(0) + (agdp() * 6));
            int tw3 = ag_txtwidth(txt3, 0);
            int tx3 = (agw() / 2) - (tw3 / 2);
            ag_text(agc(), tw3, tx3, ty3, txt3, 0xffff, 0);
            ag_sync();
          }
        }
        break;
  
      case ATEV_SELECT:
      case ATEV_BACK:
      case ATEV_MENU: {
          if (atev.d == 0) {
            ond = 0;
            res = 0;
          }
        }
        break;
    }
  }
  
  ag_ccanvas(&bg);
  return res;
  */
}

byte aw_calibmatrix(AW_CALIBPOINTP displayPtr, AW_CALIBPOINTP screenPtr, AW_CALIBMATRIXP matrixPtr) {
  return 0;
  /*
  byte retValue = 1;
  matrixPtr->Divider = ((screenPtr[0].x - screenPtr[2].x) * (screenPtr[1].y - screenPtr[2].y)) -
                       ((screenPtr[1].x - screenPtr[2].x) * (screenPtr[0].y - screenPtr[2].y)) ;
  
  if ( matrixPtr->Divider == 0 ) {
    retValue = 0;
  }
  else {
    matrixPtr->An = ((displayPtr[0].x - displayPtr[2].x) * (screenPtr[1].y - screenPtr[2].y)) -
                    ((displayPtr[1].x - displayPtr[2].x) * (screenPtr[0].y - screenPtr[2].y)) ;
    matrixPtr->Bn = ((screenPtr[0].x - screenPtr[2].x) * (displayPtr[1].x - displayPtr[2].x)) -
                    ((displayPtr[0].x - displayPtr[2].x) * (screenPtr[1].x - screenPtr[2].x)) ;
    matrixPtr->Cn = (screenPtr[2].x * displayPtr[1].x - screenPtr[1].x * displayPtr[2].x) * screenPtr[0].y +
                    (screenPtr[0].x * displayPtr[2].x - screenPtr[2].x * displayPtr[0].x) * screenPtr[1].y +
                    (screenPtr[1].x * displayPtr[0].x - screenPtr[0].x * displayPtr[1].x) * screenPtr[2].y ;
    matrixPtr->Dn = ((displayPtr[0].y - displayPtr[2].y) * (screenPtr[1].y - screenPtr[2].y)) -
                    ((displayPtr[1].y - displayPtr[2].y) * (screenPtr[0].y - screenPtr[2].y)) ;
    matrixPtr->En = ((screenPtr[0].x - screenPtr[2].x) * (displayPtr[1].y - displayPtr[2].y)) -
                    ((displayPtr[0].y - displayPtr[2].y) * (screenPtr[1].x - screenPtr[2].x)) ;
    matrixPtr->Fn = (screenPtr[2].x * displayPtr[1].y - screenPtr[1].x * displayPtr[2].y) * screenPtr[0].y +
                    (screenPtr[0].x * displayPtr[2].y - screenPtr[2].x * displayPtr[0].y) * screenPtr[1].y +
                    (screenPtr[1].x * displayPtr[0].y - screenPtr[0].x * displayPtr[1].y) * screenPtr[2].y ;
  }
  
  return retValue; */
} /* end of setCalibrationMatrix() */

byte aw_calibtools(AWINDOWP parent) {
  aw_alert(parent,
           "No Need Calibration",
           "This version doesn't need any calibration data...",
           NULL,
           NULL);
  return 1;
}
byte aw_calibtools_(AWINDOWP parent) {
  return 0;
  /*
  int USE_HACK = aw_confirm(
                   parent,
                   "Use alternative touch",
                   "Do you want to use alternative touch?\n  Only use if the default method does not work.\n\nPress the volume keys to select Yes or No.",
                   "@alert",
                   acfg_var.text_no,
                   acfg_var.text_yes
                 );
  int current_hack = atouch_gethack();
  int new_hack_val = 0;
  
  if (!USE_HACK) {
    atouch_sethack(20);
    new_hack_val = 20;
  }
  else {
    atouch_sethack(0);
  }
  
  //-- Set Mask
  CANVAS * tmpc = aw_muteparent(parent);
  on_dialog_window = 1;
  ag_rectopa(agc(), 0, 0, agw(), agh(), 0x0000, 220);
  ag_sync();
  byte isvalid = 0;
  //-- Initializing Canvas
  CANVAS ccv;
  ag_canvas(&ccv, agw(), agh());
  ag_blur(&ccv, agc(), agdp() * 2);
  atouch_plaincalibrate();
  int dp10    = agdp() * 20;
  int xpos[3] = {
    dp10, agw() - dp10, agw() / 2
  };
  int ypos[3] = {
    dp10, agh() / 2, agh() - dp10
  };
  int xtch[3] = { 0, 0, 0 };
  int ytch[3] = { 0, 0, 0 };
  AW_CALIBPOINT dPoint[3];
  AW_CALIBPOINT tPoint[3];
  int i;
  
  for (i = 0; i < 3; i++) {
    if (!aw_calibdraw(&ccv, i, xpos, ypos, xtch, ytch)) {
      goto doneit;
    }
  
    dPoint[i].x = ((float) xpos[i]) / 2;
    dPoint[i].y = ((float) ypos[i]) / 2;
    tPoint[i].x = ((float) xtch[i]) / 2;
    tPoint[i].y = ((float) ytch[i]) / 2;
  }
  
  AW_CALIBMATRIX matrix;
  
  if (aw_calibmatrix(dPoint, tPoint, &matrix)) {
    atouch_matrix_calibrate(&matrix);
  
    if (new_hack_val) {
      aw_calibdraw(&ccv, -2, &new_hack_val, ypos, xtch, ytch);
    }
    else {
      aw_calibdraw(&ccv, -1, xpos, ypos, xtch, ytch);
    }
  
    isvalid       = 1;
  }
  else {
    aw_alert(parent,
             "Calibrated Data",
             "Calibrated data not valid, please try again...",
             "@alert",
             NULL);
  }
  
  doneit:
  ag_ccanvas(&ccv);
  on_dialog_window = 0;
  aw_unmuteparent(parent, tmpc);
  byte dont_restore_caldata = 0;
  
  if (isvalid) {
    char data_calib[256];
    char datax[256];
    snprintf(datax, 256,
             "calibrate_matrix(\n"
             " \"%01.2f\",\"%01.2f\",\"%01.2f\",\"%01.2f\",\n"
             " \"%01.2f\",\"%01.2f\",\"%01.2f\",\"%i\"\n"
             ");",
             matrix.An, matrix.Bn, matrix.Cn,
             matrix.Dn, matrix.En, matrix.Fn,
             matrix.Divider, new_hack_val);
    snprintf(data_calib, 256,
             "Use/replace this command in <#009>aroma-config</#>:\n\n"
             "<#060>%s</#>\n\nLook at Recovery Log to copy this values.", datax);
    printf("\n\n\nCALIBRATION DATA\n================\nUse/replace this command in aroma-config\n%s\n\n\n",
           datax);
    aw_alert(parent,
             "Calibrated Data",
             data_calib,
             "@info",
             NULL);
    char msg_calib[256];
    snprintf(msg_calib, 256, "Do you want to use the current calibrated data in the current process?");
    dont_restore_caldata = aw_confirm(
                             parent,
                             "Calibration Data",
                             msg_calib,
                             "@info",
                             NULL,
                             NULL
                           );
  }
  
  if (!dont_restore_caldata) {
    atouch_sethack(current_hack);
    atouch_restorecalibrate();
  }
  
  return isvalid;*/
}
void aw_about_dialog(AWINDOWP parent) {
  char unchkmsg[512];
  snprintf(unchkmsg, 512,
           "<b>%s %s</b>\n"
           "%s\n\n"
           "  <#selectbg_g>Build <u>%s</u></#> (<b>%s</b>)\n"
           "  %s\n"
           "  %s\n"
           "  <u>%s</u>\n\n"
           "ROM Name:\n  <b><#selectbg_g>%s</#></b>\n"
           "ROM Version:\n  <b><#selectbg_g>%s</#></b>\n"
           "ROM Author:\n  <b><#selectbg_g>%s</#></b>\n"
           "Device:\n  <b><#selectbg_g>%s</#></b>\n"
           "Update:\n  <b><#selectbg_g>%s</#></b>"
           ,
           AROMA_NAME,
           AROMA_VERSION,
           AROMA_COPY,
           AROMA_BUILD,
           AROMA_BUILD_CN,
           AROMA_BUILD_L,
           AROMA_BUILD_A,
           AROMA_BUILD_URL,
           acfg()->rom_name,
           acfg()->rom_version,
           acfg()->rom_author,
           acfg()->rom_device,
           acfg()->rom_date
          );
  aw_alert(parent,
           AROMA_NAME " " AROMA_VERSION,
           unchkmsg,
           "@install",
           NULL);
}
byte aw_showmenu(AWINDOWP parent) {
  CANVAS * tmpc = aw_muteparent(parent);
  //-- Set Mask
  on_dialog_window = 2;
  ag_rectopa(agc(),   0, 0, agw(), agh(), 0x0000, 180);
  //ag_sync();
  //ag_sync_fade(acfg_var.fadeframes);
  int btnH  = agdp() * 20;
  int pad   = agdp() * 2;
  int vpad  = agdp() * 1;
  int winH  = ((btnH + vpad) * 3) + pad;
  int winW  = agw();
  int winX  = 0;
  int winY  = agh() - winH;
  int btnY  = winY + pad;
  int btnX  = winX + pad;
  int btnW  = winW - (pad * 2);
  //-- Initializing Canvas
  CANVAS alertbg;
  ag_canvas(&alertbg, agw(), agh());
  ag_draw(&alertbg, agc(), 0, 0);
  //-- Draw Window Background
  ag_roundgrad_ex(&alertbg, winX - 1, winY - 1, winW + 2, winH + 2, acfg_var.border, acfg_var.border_g, 0, 1, 1, 0, 0);
  ag_roundgrad_ex(&alertbg, winX, winY, winW, winH, acfg_var.navbg, acfg_var.navbg_g, 0, 1, 1, 0, 0);
  //-- Create Window
  AWINDOWP hWin   = aw(&alertbg);
  acbutton(hWin, btnX, btnY, btnW, btnH, acfg_var.text_about, 0, 11);
  //acbutton(hWin,btnX,btnY+((btnH+vpad)*1),btnW,btnH,"Help",0,12);
  //acbutton(hWin,btnX,btnY+((btnH+vpad)*1),btnW,btnH,acfg_var.text_calibrating,0,13);
  acbutton(hWin, btnX, btnY + ((btnH + vpad) * 1), btnW, btnH, acfg_var.text_quit, 0, 14);
  acbutton(hWin, btnX, btnY + ((btnH + vpad) * 2), btnW, btnH, acfg_var.text_back, 0, 5);
  /* aw_show(hWin); */
  aw_show_ex(hWin, 1, winY - 1, NULL);
  byte ondispatch = 1;
  byte res        = 0;
  
  while (ondispatch) {
    dword msg = aw_dispatch(hWin);
    
    switch (aw_gm(msg)) {
      case 5:
        ondispatch = 0;
        break;
        
      case 11:
        res = 1;
        ondispatch = 0;
        break;
        
      case 12:
        res = 2;
        ondispatch = 0;
        break;
        
      case 13:
        res = 3;
        ondispatch = 0;
        break;
        
      case 14:
        res = 4;
        ondispatch = 0;
        break;
    }
  }
  
  aw_destroy(hWin);
  ag_ccanvas(&alertbg);
  on_dialog_window = 0;
  aw_unmuteparent(parent, tmpc);
  
  if (res == 1) {
    aw_about_dialog(parent);
  }
  else if (res == 2) {
    aw_help_dialog(parent);
  }
  else if (res == 3) {
    aw_calibtools(parent);
  }
  else if (res == 4) {
    byte res = aw_confirm(parent, AROMA_NAME " " AROMA_VERSION, acfg_var.text_quit_msg, "@alert", NULL, NULL);
    
    if (res) {
      return 2;
    }
  }
  
  return 0;
}