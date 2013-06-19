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
 * AROMA UI: Checkbox Window Control
 *
 */
#include "../aroma.h"

/***************************[ BUTTON ]**************************/
typedef struct {
  CANVAS    control;
  byte      focused;
  byte      pushed;
  byte      checked;
  int       chkS;
} ACCBD, * ACCBDP;
dword accb_oninput(void * x, int action, ATEV * atev) {
  ACONTROLP ctl  = (ACONTROLP) x;
  ACCBDP  d  = (ACCBDP) ctl->d;
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
          d->checked = !d->checked;
        }
        
        msg = aw_msg(0, 1, 0, 0);
        ctl->ondraw(ctl);
      }
      break;
      
    case ATEV_SELECT: {
        if (atev->d) {
          vibrate(30);
          d->pushed = 1;
        }
        else {
          d->pushed = 0;
          d->checked = !d->checked;
        }
        
        msg = aw_msg(0, 1, 0, 0);
        ctl->ondraw(ctl);
      }
      break;
  }
  
  return msg;
}
void accb_ondraw(void * x) {
  ACONTROLP   ctl = (ACONTROLP) x;
  ACCBDP      d  = (ACCBDP) ctl->d;
  CANVAS   *  pc = &ctl->win->c;
  ag_draw(pc, &d->control, ctl->x, ctl->y);
  int halfdp   = ceil(((float) agdp()) / 2);
  int halfdp2  = halfdp * 2;
  int chkY = ((ctl->h - d->chkS) / 2);
  byte drawed = 0;
  //-- Draw Check UI
  int minpad = 3 * agdp();
  int addpad = 6 * agdp();
  
  if (!d->checked) {
    if (d->pushed) {
      drawed = atheme_draw("img.checkbox.push", pc, ctl->x + halfdp, ctl->y + chkY - minpad, d->chkS + addpad, d->chkS + addpad);
    }
    else if (d->focused) {
      drawed = atheme_draw("img.checkbox.focus", pc, ctl->x + halfdp, ctl->y + chkY - minpad, d->chkS + addpad, d->chkS + addpad);
    }
    else {
      drawed = atheme_draw("img.checkbox", pc, ctl->x + halfdp, ctl->y + chkY - minpad, d->chkS + addpad, d->chkS + addpad);
    }
  }
  else {
    if (d->pushed) {
      drawed = atheme_draw("img.checkbox.on.push", pc, ctl->x + halfdp, ctl->y + chkY - minpad, d->chkS + addpad, d->chkS + addpad);
    }
    else if (d->focused) {
      drawed = atheme_draw("img.checkbox.on.focus", pc, ctl->x + halfdp, ctl->y + chkY - minpad, d->chkS + addpad, d->chkS + addpad);
    }
    else {
      drawed = atheme_draw("img.checkbox.on", pc, ctl->x + halfdp, ctl->y + chkY - minpad, d->chkS + addpad, d->chkS + addpad);
    }
  }
  
  //-- Generic Draw
  if (!drawed) {
    if (d->pushed) {
      ag_roundgrad(pc, minpad + ctl->x + halfdp,  ctl->y + chkY,         d->chkS,          d->chkS,          acfg()->selectbg_g,  acfg()->selectbg, 0);
    }
    else if (d->focused) {
      ag_roundgrad(pc, minpad + ctl->x + halfdp,  ctl->y + chkY,         d->chkS,          d->chkS,          acfg()->selectbg,  acfg()->selectbg_g, 0);
    }
    else {
      ag_roundgrad(pc, minpad + ctl->x + halfdp,  ctl->y + chkY,         d->chkS,          d->chkS,          acfg()->controlbg_g,  acfg()->controlbg, 0);
    }
    
    ag_roundgrad(pc, minpad + ctl->x + halfdp2, ctl->y + chkY + halfdp,  d->chkS - halfdp2,  d->chkS - halfdp2,  acfg()->textbg,       acfg()->textbg,    0);
    
    if (d->checked) {
      ag_roundgrad(pc, minpad + ctl->x + halfdp + halfdp2, ctl->y + chkY + halfdp2,  d->chkS - (halfdp2 * 2),  d->chkS - (halfdp2 * 2),  acfg()->selectbg,   acfg()->selectbg_g, 0);
    }
  }
}
byte accb_ischecked(ACONTROLP ctl) {
  ACCBDP      d  = (ACCBDP) ctl->d;
  return d->checked;
}
void accb_ondestroy(void * x) {
  ACONTROLP   ctl = (ACONTROLP) x;
  ACCBDP  d  = (ACCBDP) ctl->d;
  ag_ccanvas(&d->control);
  free(ctl->d);
}
byte accb_onfocus(void * x) {
  ACONTROLP   ctl = (ACONTROLP) x;
  ACCBDP  d  = (ACCBDP) ctl->d;
  d->focused = 1;
  ctl->ondraw(ctl);
  return 1;
}
void accb_onblur(void * x) {
  ACONTROLP   ctl = (ACONTROLP) x;
  ACCBDP  d  = (ACCBDP) ctl->d;
  d->focused = 0;
  ctl->ondraw(ctl);
}
ACONTROLP accb(
  AWINDOWP win,
  int x,
  int y,
  int w,
  int h,
  char * textv,
  byte checked
) {
  //-- Validate Minimum Size
  if (h < agdp() * 16) {
    h = agdp() * 16;
  }
  
  if (w < agdp() * 16) {
    w = agdp() * 16;
  }
  
  //-- Limit Title Length
  char title[128];
  snprintf(title, 128, "%s", textv);
  //-- Initializing Button Data
  ACCBDP d = (ACCBDP) malloc(sizeof(ACCBD));
  memset(d, 0, sizeof(ACCBD));
  //-- Save Touch Message & Set Stats
  d->checked   = checked;
  d->focused   = 0;
  d->pushed    = 0;
  //-- Initializing Canvas
  ag_canvas(&d->control, w, h);
  //-- Draw Control Background
  ag_draw_ex(&d->control, &win->c, 0, 0, x, y, w, h);
  //-- Calculate Position & Size
  int minpad    = 5 * agdp();
  d->chkS       = (agdp() * 10);
  int txtW      = w - ((d->chkS + 6) + (agdp() * 4));
  int txtX      = (d->chkS + (agdp() * 4));
  int txtH      = ag_txtheight(txtW, title, 0);
  int txtY      = ((h - txtH) / 2);
  
  if (txtY < 1) {
    txtY = 1;
  }
  
  ag_textf(&d->control, txtW, minpad + txtX, txtY, title, acfg()->textbg, 0);
  ag_text(&d->control, txtW, minpad + txtX - 1, txtY - 1, title, acfg()->textfg, 0);
  //-- Initializing Control
  ACONTROLP ctl  = malloc(sizeof(ACONTROL));
  ctl->ondestroy = &accb_ondestroy;
  ctl->oninput  = &accb_oninput;
  ctl->ondraw   = &accb_ondraw;
  ctl->onblur   = &accb_onblur;
  ctl->onfocus  = &accb_onfocus;
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
