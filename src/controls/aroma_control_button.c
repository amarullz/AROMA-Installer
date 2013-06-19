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
 * AROMA UI: Button Window Control
 *
 */
#include "../aroma.h"

/***************************[ BUTTON ]**************************/
typedef struct {
  CANVAS    control;
  CANVAS    control_pushed;
  CANVAS    control_focused;
  byte      touchmsg;
  byte      focused;
  byte      pushed;
} ACBUTTOND, * ACBUTTONDP;
dword acbutton_oninput(void * x, int action, ATEV * atev) {
  ACONTROLP ctl  = (ACONTROLP) x;
  ACBUTTONDP  d  = (ACBUTTONDP) ctl->d;
  dword msg = 0;
  
  switch (action) {
    case ATEV_MOUSEDN: {
        vibrate(30);
        d->pushed = 1;
        msg = aw_msg(0, 1, 0, 0);
        ctl->ondraw(ctl);
      }
      break;
      
    case ATEV_MOUSEUP: {
        d->pushed = 0;
        
        if (aw_touchoncontrol(ctl, atev->x, atev->y)) {
          msg = aw_msg(d->touchmsg, 1, 0, 0);
        }
        else {
          msg = aw_msg(0, 1, 0, 0);
        }
        
        ctl->ondraw(ctl);
      }
      break;
      
    case ATEV_SELECT: {
        if (atev->d) {
          vibrate(30);
          d->pushed = 1;
          msg = aw_msg(0, 1, 0, 0);
          ctl->ondraw(ctl);
        }
        else {
          d->pushed = 0;
          msg = aw_msg(d->touchmsg, 1, 0, 0);
          ctl->ondraw(ctl);
        }
      }
      break;
  }
  
  return msg;
}
void acbutton_ondraw(void * x) {
  ACONTROLP   ctl = (ACONTROLP) x;
  ACBUTTONDP  d  = (ACBUTTONDP) ctl->d;
  CANVAS  * pc = &ctl->win->c;
  
  if (d->pushed) {
    ag_draw(pc, &d->control_pushed, ctl->x, ctl->y);
  }
  else if (d->focused) {
    ag_draw(pc, &d->control_focused, ctl->x, ctl->y);
  }
  else {
    ag_draw(pc, &d->control, ctl->x, ctl->y);
  }
}
void acbutton_ondestroy(void * x) {
  ACONTROLP   ctl = (ACONTROLP) x;
  ACBUTTONDP  d  = (ACBUTTONDP) ctl->d;
  ag_ccanvas(&d->control);
  ag_ccanvas(&d->control_pushed);
  ag_ccanvas(&d->control_focused);
  free(ctl->d);
}
byte acbutton_onfocus(void * x) {
  ACONTROLP   ctl = (ACONTROLP) x;
  ACBUTTONDP  d  = (ACBUTTONDP) ctl->d;
  d->focused = 1;
  ctl->ondraw(ctl);
  return 1;
}
void acbutton_onblur(void * x) {
  ACONTROLP   ctl = (ACONTROLP) x;
  ACBUTTONDP  d  = (ACBUTTONDP) ctl->d;
  d->focused = 0;
  ctl->ondraw(ctl);
}
ACONTROLP acbutton(
  AWINDOWP win,
  int x,
  int y,
  int w,
  int h,
  char * text,
  byte isbig,
  byte touchmsg
) {
  //-- Validate Minimum Size
  if (h < agdp() * 16) {
    h = agdp() * 16;
  }
  
  if (w < agdp() * 16) {
    w = agdp() * 16;
  }
  
  //-- Initializing Text Metrics
  int txtw     = ag_txtwidth(text, isbig);
  int txth     = ag_fontheight(isbig);
  int txtx     = round(w / 2) - round(txtw / 2);
  int txty     = round(h / 2) - round(txth / 2);
  //-- Initializing Button Data
  ACBUTTONDP d = (ACBUTTONDP) malloc(sizeof(ACBUTTOND));
  memset(d, 0, sizeof(ACBUTTOND));
  //-- Save Touch Message & Set Stats
  d->touchmsg  = touchmsg;
  d->focused   = 0;
  d->pushed    = 0;
  //-- Initializing Canvas
  ag_canvas(&d->control, w, h);
  ag_canvas(&d->control_pushed, w, h);
  ag_canvas(&d->control_focused, w, h);
  //-- Draw Rest Control
  dword hl1 = ag_calchighlight(acfg()->controlbg, acfg()->controlbg_g);
  ag_draw_ex(&d->control, &win->c, 0, 0, x, y, w, h);
  
  if (!atheme_draw("img.button", &d->control, 0, 0, w, h)) {
    ag_roundgrad(&d->control, 0, 0, w, h, acfg()->border, acfg()->border_g, (agdp()*acfg()->btnroundsz));
    ag_roundgrad(&d->control, 1, 1, w - 2, h - 2,
                 ag_calculatealpha(acfg()->controlbg, acfg()->winbg, 180),
                 ag_calculatealpha(acfg()->controlbg_g, acfg()->winbg, 160),
                 (agdp()*acfg()->btnroundsz) - 1);
    ag_roundgrad(&d->control, 2, 2, w - 4, h - 4, acfg()->controlbg, acfg()->controlbg_g, (agdp()*acfg()->btnroundsz) - 2);
    ag_roundgrad_ex(&d->control, 2, 2, w - 4, (h - 4) / 2, LOWORD(hl1), HIWORD(hl1), (agdp()*acfg()->btnroundsz) - 2, 1, 1, 0, 0);
  }
  
  ag_textf(&d->control, txtw, txtx + 1, txty + 1, text, acfg()->controlbg, isbig);
  ag_text(&d->control, txtw, txtx, txty, text, acfg()->controlfg, isbig);
  //-- Draw Pushed Control
  //-- Highlight
  color pshad = ag_calpushad(acfg()->selectbg_g);
  hl1 = ag_calcpushlight(acfg()->selectbg, pshad);
  ag_draw_ex(&d->control_pushed, &win->c, 0, 0, x, y, w, h);
  
  if (!atheme_draw("img.button.push", &d->control_pushed, 0, 0, w, h)) {
    ag_roundgrad(&d->control_pushed, 0, 0, w, h, acfg()->border, acfg()->border_g, (agdp()*acfg()->btnroundsz));
    ag_roundgrad(&d->control_pushed, 1, 1, w - 2, h - 2, acfg()->controlbg, acfg()->controlbg_g, (agdp()*acfg()->btnroundsz) - 1);
    ag_roundgrad(&d->control_pushed, 2, 2, w - 4, h - 4, acfg()->selectbg, pshad, (agdp()*acfg()->btnroundsz) - 2);
    ag_roundgrad_ex(&d->control_pushed, 2, 2, w - 4, (h - 4) / 2, LOWORD(hl1), HIWORD(hl1), (agdp()*acfg()->btnroundsz) - 2, 1, 1, 0, 0);
  }
  
  ag_textf(&d->control_pushed, txtw, txtx + 1, txty + 1, text, acfg()->selectbg_g, isbig);
  ag_text(&d->control_pushed, txtw, txtx, txty, text, acfg()->selectfg, isbig);
  //-- Draw Focused Control
  hl1 = ag_calchighlight(acfg()->selectbg, acfg()->selectbg_g);
  ag_draw_ex(&d->control_focused, &win->c, 0, 0, x, y, w, h);
  
  if (!atheme_draw("img.button.focus", &d->control_focused, 0, 0, w, h)) {
    ag_roundgrad(&d->control_focused, 0, 0, w, h, acfg()->border, acfg()->border_g, (agdp()*acfg()->btnroundsz));
    ag_roundgrad(&d->control_focused, 1, 1, w - 2, h - 2, acfg()->controlbg, acfg()->controlbg_g, (agdp()*acfg()->btnroundsz) - 1);
    ag_roundgrad(&d->control_focused, 2, 2, w - 4, h - 4, acfg()->selectbg, acfg()->selectbg_g, (agdp()*acfg()->btnroundsz) - 2);
    ag_roundgrad_ex(&d->control_focused, 2, 2, w - 4, (h - 4) / 2, LOWORD(hl1), HIWORD(hl1), (agdp()*acfg()->btnroundsz) - 2, 1, 1, 0, 0);
  }
  
  ag_textf(&d->control_focused, txtw, txtx + 1, txty + 1, text, acfg()->selectbg_g, isbig);
  ag_text(&d->control_focused, txtw, txtx, txty, text, acfg()->selectfg, isbig);
  //-- Initializing Control
  ACONTROLP ctl  = malloc(sizeof(ACONTROL));
  ctl->ondestroy = &acbutton_ondestroy;
  ctl->oninput  = &acbutton_oninput;
  ctl->ondraw   = &acbutton_ondraw;
  ctl->onblur   = &acbutton_onblur;
  ctl->onfocus  = &acbutton_onfocus;
  ctl->win      = win;
  ctl->x        = x;
  ctl->y        = y;
  ctl->w        = w;
  ctl->h        = h;
  ctl->forceNS  = 0;
  ctl->d        = (void *) d;
  aw_add(win, ctl);
  return ctl;
}
