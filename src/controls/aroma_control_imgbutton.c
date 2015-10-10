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
} IMGBTND, * IMGBTNDP;
dword imgbtn_oninput(void * x, int action, ATEV * atev) {
  ACONTROLP ctl  = (ACONTROLP) x;
  IMGBTNDP  d  = (IMGBTNDP) ctl->d;
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
void imgbtn_ondraw(void * x) {
  ACONTROLP   ctl = (ACONTROLP) x;
  IMGBTNDP  d  = (IMGBTNDP) ctl->d;
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
void imgbtn_ondestroy(void * x) {
  ACONTROLP   ctl = (ACONTROLP) x;
  IMGBTNDP  d  = (IMGBTNDP) ctl->d;
  ag_ccanvas(&d->control);
  ag_ccanvas(&d->control_pushed);
  ag_ccanvas(&d->control_focused);
  free(ctl->d);
}
byte imgbtn_onfocus(void * x) {
  ACONTROLP   ctl = (ACONTROLP) x;
  IMGBTNDP  d  = (IMGBTNDP) ctl->d;
  d->focused = 1;
  ctl->ondraw(ctl);
  return 1;
}
void imgbtn_onblur(void * x) {
  ACONTROLP   ctl = (ACONTROLP) x;
  IMGBTNDP  d  = (IMGBTNDP) ctl->d;
  d->focused = 0;
  ctl->ondraw(ctl);
}
ACONTROLP imgbtn_reinit(
  AWINDOWP win,
  ACONTROLP ctl,
  int x,
  int y,
  int w,
  int h,
  PNGCANVAS * img,
  char * text,
  byte isflat,
  byte touchmsg
) {
  if ((ctl != NULL) && (x == 0) && (y == 0) && (h == 0) && (w == 0)) {
    x = ctl->x;
    y = ctl->y;
    w = ctl->w;
    h = ctl->h;
  }
  
  int txtw = 0;
  int txth = 0;
  int txtx = 0;
  int txty = 0;
  char vtext[64] = {0};
  
  if (w < agdp() * 20) {
    w = agdp() * 20;
  }
  
  if (h < agdp() * 20) {
    h = agdp() * 20;
  }
  
  if (text != NULL) {
    snprintf(vtext, 64, "<b>%s</b>", text);
    
    //-- Initializing Text Metrics
    if ((isflat == 3) || (isflat == 4) || (isflat == 5)) {
      txtw = ag_txtwidth(text, 0);
      txth = ag_fontheight(0);
      
      if (w < ((agdp() * 22) + txtw)) {
        w = ((agdp() * 22) + txtw);
      }
      
      txtx = round(w / 2) - round(((agdp() * 20) + txtw) / 2);
      txty = round(h / 2) - round(txth / 2);
    }
    else {
      txtw = ag_txtwidth(text, 0);
      txth = ag_fontheight(0);
      
      if (h < ((agdp() * 20) + txth)) {
        h = ((agdp() * 20) + txth);
      }
      
      txtx = round(w / 2) - round(txtw / 2);
      txty = (agdp() * 16);
    }
  }
  
  int imgS = agdp() * 16;
  int imgX = round(w / 2) - round(imgS / 2);
  int imgY = 0; // agdp()*2;
  int contentH = (agdp() * 16) +  txth;
  int contentY = (h / 2) - (contentH / 2);
  
  if ((isflat == 3) || (isflat == 4) || (isflat == 5)) {
    imgY = round(h / 2) - round(imgS / 2);
    imgX = txtx + (agdp() * 2);
    txtx += agdp() * 20;
  }
  else {
    imgY += contentY;
    txty += contentY;
  }
  
  int allW      = (agdp() * 18) + txtw;
  int startX    = round(w / 2) - round(allW / 2);
  
  if (isflat == 5) {
    imgX = startX + txtw + (agdp() * 2);
    txtx = startX;
  }
  else if (isflat == 4) {
    imgX = startX;
    txtx = startX + (agdp() * 18);
  }
  
  if (isflat == 3) {
    isflat = 0;
  }
  
  //-- Initializing Button Data
  IMGBTNDP d = NULL;
  
  if (ctl != NULL) {
    d = ctl->d;
    win = ctl->win;
  }
  else {
    d = (IMGBTNDP) malloc(sizeof(IMGBTND));
    memset(d, 0, sizeof(IMGBTND));
    //-- Save Touch Message & Set Stats
    d->focused   = 0;
    d->pushed    = 0;
    //-- Initializing Canvas
    ag_canvas(&d->control, w, h);
    ag_canvas(&d->control_pushed, w, h);
    ag_canvas(&d->control_focused, w, h);
  }
  
  d->touchmsg  = touchmsg;
  //-- Draw Rest Control
  dword hl1 = ag_calchighlight(acfg()->controlbg, acfg()->controlbg_g);
  ag_draw_ex(&d->control, win->bg, 0, 0, x, y, w, h);
  
  if (!isflat) {
    if (!atheme_draw("img.button", &d->control, 0, 0, w, h)) {
      ag_roundgrad(&d->control, 0, 0, w, h, acfg()->border, acfg()->border_g, (agdp()*acfg()->btnroundsz));
      ag_roundgrad(&d->control, 1, 1, w - 2, h - 2,
                   ag_calculatealpha(acfg()->controlbg, acfg()->winbg, 180),
                   ag_calculatealpha(acfg()->controlbg_g, acfg()->winbg, 160),
                   (agdp()*acfg()->btnroundsz) - 1
                  );
      ag_roundgrad(&d->control, 2, 2, w - 4, h - 4, acfg()->controlbg, acfg()->controlbg_g, (agdp()*acfg()->btnroundsz) - 2);
      ag_roundgrad_ex(&d->control, 2, 2, w - 4, (h - 4) / 2, LOWORD(hl1), HIWORD(hl1), (agdp()*acfg()->btnroundsz) - 2, 1, 1, 0, 0);
    }
  }
  
  ag_textf(&d->control, txtw, txtx + 1, txty + 1, vtext, acfg()->controlbg, 0);
  ag_text(&d->control, txtw, txtx, txty, vtext, acfg()->controlfg, 0);
  color pshad = ag_calpushad(acfg()->selectbg_g);
  ag_draw_ex(&d->control_pushed, win->bg, 0, 0, x, y, w, h);
  int wadd = (isflat == 2) ? 2 : 0;
  int wdel = wadd * 2;
  
  //-- Draw Pushed Control
  if (!isflat) {
    hl1 = ag_calcpushlight(acfg()->selectbg, pshad);
    
    if (!atheme_draw("img.button.push", &d->control_pushed, 0, 0, w, h)) {
      ag_roundgrad(&d->control_pushed, 0, 0, w, h, acfg()->border, acfg()->border_g, (agdp()*acfg()->btnroundsz));
      ag_roundgrad(&d->control_pushed, 1, 1, w - 2, h - 2, acfg()->selectbg, pshad, (agdp()*acfg()->btnroundsz) - 1);
      ag_roundgrad_ex(&d->control_pushed, 1, 1, w - 2, (h - 2) / 2, LOWORD(hl1), HIWORD(hl1), (agdp()*acfg()->btnroundsz) - 1, 1, 1, 0, 0);
    }
    
    ag_textf(&d->control_pushed, txtw, txtx + 1, txty + 1, vtext, acfg()->selectbg_g, 0);
    ag_text(&d->control_pushed, txtw, txtx, txty, vtext, acfg()->selectfg, 0);
  }
  else {
    hl1 = ag_calchighlight(acfg()->controlbg, acfg()->controlbg_g);
    
    if (!atheme_draw("img.button", &d->control_pushed, 0, 0, w, h)) {
      ag_roundgrad(&d->control_pushed, wadd, wadd, w - wdel, h - wdel, acfg()->border, acfg()->border_g, (agdp()*acfg()->btnroundsz));
      ag_roundgrad(&d->control_pushed, wadd + 1, wadd + 1, w - (2 + wdel), h - (2 + wdel), acfg()->controlbg, acfg()->controlbg_g, (agdp()*acfg()->btnroundsz) - 1);
      ag_roundgrad_ex(&d->control_pushed, wadd + 1, wadd + 1, w - (2 + wdel), (h - (1 + wdel)) / 2, LOWORD(hl1), HIWORD(hl1), (agdp()*acfg()->btnroundsz) - 1, 1, 1, 0, 0);
    }
    
    ag_textf(&d->control_pushed, txtw, txtx + 1, txty + 1, vtext, acfg()->controlbg, 0);
    ag_text(&d->control_pushed, txtw, txtx, txty, vtext, acfg()->controlfg, 0);
  }
  
  //-- Draw Focused Control
  hl1 = ag_calchighlight(acfg()->selectbg, acfg()->selectbg_g);
  ag_draw_ex(&d->control_focused, win->bg, 0, 0, x, y, w, h);
  
  if (!isflat) {
    if (!atheme_draw("img.button.focus", &d->control_focused, 0, 0, w, h)) {
      ag_roundgrad(&d->control_focused, wadd, wadd, w - wdel, h - wdel, acfg()->border, acfg()->border_g, (agdp()*acfg()->btnroundsz));
      ag_roundgrad(&d->control_focused, wadd + 1, wadd + 1, w - (wdel + 2), h - (wdel + 2), acfg()->selectbg, acfg()->selectbg_g, (agdp()*acfg()->btnroundsz) - 1);
      ag_roundgrad_ex(&d->control_focused, wadd + 1, wadd + 1, w - (wdel + 2), (h - (wdel + 2)) / 2, LOWORD(hl1), HIWORD(hl1), (agdp()*acfg()->btnroundsz) - 1, 1, 1, 0, 0);
    }
    
    ag_textf(&d->control_focused, txtw, txtx + 1, txty + 1, vtext, acfg()->selectbg_g, 0);
    ag_text(&d->control_focused, txtw, txtx, txty, vtext, acfg()->selectfg, 0);
  }
  else {
    ag_textf(&d->control_focused, txtw, txtx + 1, txty + 1, vtext, acfg()->controlbg, 0);
    ag_text(&d->control_focused, txtw, txtx, txty, vtext, acfg()->controlfg, 0);
  }
  
  if (img != NULL) {
    apng_stretch(&d->control, img, imgX, imgY, imgS, imgS, 0, 0, img->w, img->h);
    apng_stretch(&d->control_pushed, img, imgX, imgY, imgS, imgS, 0, 0, img->w, img->h);
    apng_stretch(&d->control_focused, img, imgX, imgY, imgS, imgS, 0, 0, img->w, img->h);
  }
  
  //-- Initializing Control
  if (ctl == NULL) {
    ctl  = malloc(sizeof(ACONTROL));
    ctl->ondestroy = &imgbtn_ondestroy;
    ctl->oninput  = &imgbtn_oninput;
    ctl->ondraw   = &imgbtn_ondraw;
    ctl->onblur   = &imgbtn_onblur;
    ctl->onfocus  = &imgbtn_onfocus;
    ctl->win      = win;
    ctl->forceNS  = 0;
    ctl->d        = (void *) d;
    ctl->x        = x;
    ctl->y        = y;
    ctl->w        = w;
    ctl->h        = h;
    aw_add(win, ctl);
  }
  else {
    ctl->x        = x;
    ctl->y        = y;
    ctl->w        = w;
    ctl->h        = h;
    imgbtn_ondraw(ctl);
  }
  
  return ctl;
}

ACONTROLP imgbtn(
  AWINDOWP win,
  int x,
  int y,
  int w,
  int h,
  PNGCANVAS * img,
  char * text,
  byte isflat,
  byte touchmsg
) {
  return imgbtn_reinit(
           win,
           NULL,
           x,
           y,
           w,
           h,
           img,
           text,
           isflat,
           touchmsg
         );
}