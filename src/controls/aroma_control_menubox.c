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
 * AROMA UI: Menubox List Window Control
 *
 */
#include "../aroma.h"

/***************************[ MENU BOX ]**************************/
typedef struct {
  char title[64];
  char desc[128];
  PNGCANVAS * img;
  int  id;
  int  h;
  int  y;
  
  /* Title & Desc Size/Pos */
  int  th;
  int  dh;
  int  ty;
  int  dy;
} ACMENUI, * ACMENUIP;
typedef struct {
  byte      acheck_signature;
  CANVAS    client;
  CANVAS    control;
  CANVAS    control_focused;
  AKINETIC  akin;
  int       scrollY;
  int       maxScrollY;
  int       prevTouchY;
  int       invalidDrawItem;
  
  /* Client Size */
  int clientWidth;
  int clientTextW;
  int clientTextX;
  int nextY;
  
  /* Items */
  ACMENUIP * items;
  int       itemn;
  int       touchedItem;
  int       focusedItem;
  int       draweditemn;
  int       selectedIndex;
  byte      touchmsg;
  /* Focus */
  byte      focused;
} ACMENUD, * ACMENUDP;
void acmenu_ondestroy(void * x) {
  ACONTROLP ctl = (ACONTROLP) x;
  ACMENUDP d  = (ACMENUDP) ctl->d;
  ag_ccanvas(&d->control);
  ag_ccanvas(&d->control_focused);
  
  if (d->itemn > 0) {
    int i;
    
    for (i = 0; i < d->itemn; i++) {
      if (d->items[i]->img != NULL) {
        apng_close(d->items[i]->img);
        free(d->items[i]->img);
        d->items[i]->img = NULL;
      }
      
      free(d->items[i]);
    }
    
    free(d->items);
    ag_ccanvas(&d->client);
  }
  
  free(ctl->d);
}
void acmenu_redrawitem(ACONTROLP ctl, int index) {
  ACMENUDP d = (ACMENUDP) ctl->d;
  
  if (d->acheck_signature != 144) {
    return;  //-- Not Valid Signature
  }
  
  if ((index >= d->itemn) || (index < 0)) {
    return;  //-- Not Valid Index
  }
  
  ACMENUIP p = d->items[index];
  CANVAS  * c = &d->client;
  //-- Cleanup Background
  ag_rect(c, 0, p->y, d->clientWidth, p->h, acfg()->textbg);
  color txtcolor = acfg()->textfg;
  color graycolor = acfg()->textfg_gray;
  byte isselectcolor = 0;
  
  if (index == d->touchedItem) {
    if (!atheme_draw("img.selection.push", c, 0, p->y + agdp(), d->clientWidth, p->h - (agdp() * 2))) {
      color pshad = ag_calpushad(acfg()->selectbg_g);
      dword hl1 = ag_calcpushlight(acfg()->selectbg, pshad);
      ag_roundgrad(c, 0, p->y + agdp(), d->clientWidth, p->h - (agdp() * 2), acfg()->selectbg, pshad, (agdp()*acfg()->roundsz));
      ag_roundgrad(c, 0, p->y + agdp(), d->clientWidth, (p->h - (agdp() * 2)) / 2, LOWORD(hl1), HIWORD(hl1), (agdp()*acfg()->roundsz));
    }
    
    graycolor = txtcolor = acfg()->selectfg;
    isselectcolor = 1;
  }
  else if ((index == d->focusedItem) && (d->focused)) {
    if (!atheme_draw("img.selection", c, 0, p->y + agdp(), d->clientWidth, p->h - (agdp() * 2))) {
      dword hl1 = ag_calchighlight(acfg()->selectbg, acfg()->selectbg_g);
      ag_roundgrad(c, 0, p->y + agdp(), d->clientWidth, p->h - (agdp() * 2), acfg()->selectbg, acfg()->selectbg_g, (agdp()*acfg()->roundsz));
      ag_roundgrad(c, 0, p->y + agdp(), d->clientWidth, (p->h - (agdp() * 2)) / 2, LOWORD(hl1), HIWORD(hl1), (agdp()*acfg()->roundsz));
    }
    
    graycolor = txtcolor = acfg()->selectfg;
    isselectcolor = 1;
  }
  
  if (index < d->itemn - 1) {
    //-- Not Last... Add Separator
    color sepcl = ag_calculatealpha(acfg()->textbg, acfg()->textfg_gray, 80);
    ag_rect(c, 0, p->y + p->h - 1, d->clientWidth, 1, sepcl);
  }
  
  //-- Now Draw The Checkbox
  int imgS = agdp() * 30;
  
  if (p->img != NULL) {
    int imgW = p->img->w;
    int imgH = p->img->h;
    
    if (imgW > imgS) {
      imgW = imgS;
    }
    
    if (imgH > imgS) {
      imgH = imgS;
    }
    
    int imgX = round((imgS - imgW) / 2);
    int imgY = round((imgS - imgH) / 2) + (agdp() * 2);
    apng_draw_ex(c, p->img, imgX + agdp(), p->y + imgY, 0, 0, imgW, imgH);
  }
  
  int txtH    = p->th + p->dh;
  int txtAddY = 0;
  
  if (txtH < imgS) {
    txtAddY = round((imgS - txtH) / 2);
  }
  
  //-- Now Draw The Text
  if (isselectcolor) {
    ag_textf(c, d->clientTextW, d->clientTextX, p->y + p->ty + txtAddY, p->title, acfg()->selectbg_g, 0);
    ag_textf(c, d->clientTextW, d->clientTextX, p->y + p->dy + txtAddY, p->desc, acfg()->selectbg_g, 0);
  }
  
  ag_text(c, d->clientTextW, d->clientTextX - 1, (p->y + p->ty + txtAddY) - 1, p->title, txtcolor, 0);
  ag_text(c, d->clientTextW, d->clientTextX - 1, (p->y + p->dy + txtAddY) - 1, p->desc, graycolor, 0);
}
void acmenu_redraw(ACONTROLP ctl) {
  ACMENUDP d = (ACMENUDP) ctl->d;
  
  if (d->acheck_signature != 144) {
    return;  //-- Not Valid Signature
  }
  
  if ((d->itemn > 0) && (d->draweditemn < d->itemn)) {
    ag_ccanvas(&d->client);
    ag_canvas(&d->client, d->clientWidth, d->nextY);
    ag_rect(&d->client, 0, 0, d->clientWidth, agdp()*max(acfg()->roundsz, 4), acfg()->textbg);
    //-- Set Values
    d->scrollY     = 0;
    d->maxScrollY  = d->nextY - (ctl->h - (agdp() * max(acfg()->roundsz, 4)));
    
    if (d->maxScrollY < 0) {
      d->maxScrollY = 0;
    }
    
    //-- Draw Items
    int i;
    
    for (i = 0; i < d->itemn; i++) {
      acmenu_redrawitem(ctl, i);
    }
    
    d->draweditemn = d->itemn;
  }
}
int acmenu_getselectedindex(ACONTROLP ctl) {
  ACMENUDP d = (ACMENUDP) ctl->d;
  
  if (d->acheck_signature != 144) {
    return -1;  //-- Not Valid Signature
  }
  
  return d->selectedIndex;
}
//-- Add Item Into Control
byte acmenu_add(ACONTROLP ctl, char * title, char * desc, char * img) {
  ACMENUDP d = (ACMENUDP) ctl->d;
  
  if (d->acheck_signature != 144) {
    return 0;  //-- Not Valid Signature
  }
  
  //-- Allocating Memory For Item Data
  ACMENUIP newip = (ACMENUIP) malloc(sizeof(ACMENUI));
  snprintf(newip->title, 64, "%s", title);
  snprintf(newip->desc, 128, "%s", desc);
  //-- Load Image
  newip->img      = (PNGCANVAS *) malloc(sizeof(PNGCANVAS));
  memset(newip->img, 0, sizeof(PNGCANVAS));
  
  if (!apng_load(newip->img, img)) {
    free(newip->img);
    newip->img = NULL;
  }
  
  newip->th       = ag_txtheight(d->clientTextW, newip->title, 0);
  newip->dh       = ag_txtheight(d->clientTextW, newip->desc, 0);
  newip->ty       = agdp() * 2;
  newip->dy       = (agdp() * 2) + newip->th;
  newip->h        = (agdp() * 4) + newip->dh + newip->th;
  
  if (newip->h < (agdp() * 34)) {
    newip->h = (agdp() * 34);
  }
  
  newip->id       = d->itemn;
  newip->y        = d->nextY;
  d->nextY       += newip->h;
  
  if (d->itemn > 0) {
    int i;
    ACMENUIP * tmpitms   = d->items;
    d->items              = malloc( sizeof(ACMENUIP) * (d->itemn + 1) );
    
    for (i = 0; i < d->itemn; i++) {
      d->items[i] = tmpitms[i];
    }
    
    d->items[d->itemn] = newip;
    free(tmpitms);
  }
  else {
    d->items    = malloc(sizeof(ACMENUIP));
    d->items[0] = newip;
  }
  
  d->itemn++;
  return 1;
}
void acmenu_ondraw(void * x) {
  ACONTROLP   ctl = (ACONTROLP) x;
  ACMENUDP   d  = (ACMENUDP) ctl->d;
  CANVAS   *  pc = &ctl->win->c;
  acmenu_redraw(ctl);
  
  if (d->invalidDrawItem != -1) {
    d->touchedItem = d->invalidDrawItem;
    acmenu_redrawitem(ctl, d->invalidDrawItem);
    d->invalidDrawItem = -1;
  }
  
  //-- Init Device Pixel Size
  int minpadding = 4;// max(acfg()->roundsz,4);
  int agdp3 = (agdp() * minpadding);
  int agdp6 = (agdp() * (minpadding * 2));
  int agdpX = agdp6;
  
  if (d->focused) {
    ag_draw(pc, &d->control_focused, ctl->x, ctl->y);
    ag_draw_ex(pc, &d->client, ctl->x + agdp3, ctl->y + agdp(), 0, d->scrollY + agdp(), ctl->w - agdp6, ctl->h - (agdp() * 2));
  }
  else {
    ag_draw(pc, &d->control, ctl->x, ctl->y);
    ag_draw_ex(pc, &d->client, ctl->x + agdp3, ctl->y + 1, 0, d->scrollY + 1, ctl->w - agdp6, ctl->h - 2);
  }
  
  if (d->maxScrollY > 0) {
    //-- Glow
    int i;
    byte isST = (d->scrollY > 0) ? 1 : 0;
    byte isSB = (d->scrollY < d->maxScrollY) ? 1 : 0;
    int add_t_y = 1;
    
    if (d->focused) {
      add_t_y = agdp();
    }
    
    for (i = 0; i < agdpX; i++) {
      byte alph = 255 - round((((float) (i + 1)) / ((float) agdpX)) * 230);
      
      if (isST) {
        ag_rectopa(pc, ctl->x + agdp3, ctl->y + i + add_t_y, ctl->w - agdpX, 1, acfg()->textbg, alph);
      }
      
      if (isSB) {
        ag_rectopa(pc, ctl->x + agdp3, ((ctl->y + ctl->h) - (add_t_y)) - (i + 1), ctl->w - agdpX, 1, acfg()->textbg, alph);
      }
    }
    
    if (d->maxScrollY > 0) {
      //-- Scrollbar
      int newh = ctl->h - agdp() * 3;
      float scrdif    = ((float) newh) / ((float) d->client.h);
      int  scrollbarH = floor(scrdif * newh);
      int  scrollbarY = floor(scrdif * d->scrollY) + agdp();
      
      if (d->scrollY < 0) {
        scrollbarY = agdp();
        int alp = (1.0 - (((float) abs(d->scrollY)) / (((float) ctl->h) / 4))) * 255;
        
        if (alp < 0) {
          alp = 0;
        }
        
        ag_rectopa(pc, (ctl->w - agdp() * 3) + ctl->x, scrollbarY + ctl->y, agdp(), scrollbarH, acfg()->scrollbar, alp);
      }
      else if (d->scrollY > d->maxScrollY) {
        scrollbarY = floor(scrdif * d->maxScrollY) + agdp();
        int alp = (1.0 - (((float) abs(d->scrollY - d->maxScrollY)) / (((float) ctl->h) / 4))) * 255;
        
        if (alp < 0) {
          alp = 0;
        }
        
        ag_rectopa(pc, (ctl->w - agdp() * 3) + ctl->x, scrollbarY + ctl->y, agdp(), scrollbarH, acfg()->scrollbar, alp);
      }
      else {
        ag_rect(pc, (ctl->w - agdp() * 3) + ctl->x, scrollbarY + ctl->y, agdp(), scrollbarH, acfg()->scrollbar);
      }
    }
  }
}
dword acmenu_oninput(void * x, int action, ATEV * atev) {
  ACONTROLP ctl = (ACONTROLP) x;
  ACMENUDP d  = (ACMENUDP) ctl->d;
  dword msg = 0;
  
  switch (action) {
    case ATEV_MOUSEDN: {
        d->prevTouchY  = atev->y;
        akinetic_downhandler(&d->akin, atev->y);
        int touchpos = atev->y - ctl->y + d->scrollY;
        int i;
        
        for (i = 0; i < d->itemn; i++) {
          if ((touchpos >= d->items[i]->y) && (touchpos < d->items[i]->y + d->items[i]->h)) {
            ac_regpushwait(
              ctl, &d->prevTouchY, &d->invalidDrawItem, i
            );
            break;
          }
        }
      }
      break;
      
    case ATEV_MOUSEUP: {
        byte retmsgx = 0;
        
        if ((d->prevTouchY != -50) && (abs(d->prevTouchY - atev->y) < agdp() * 5)) {
          d->prevTouchY = -50;
          int touchpos = atev->y - ctl->y + d->scrollY;
          int i;
          
          for (i = 0; i < d->itemn; i++) {
            if ((touchpos >= d->items[i]->y) && (touchpos < d->items[i]->y + d->items[i]->h)) {
              if ((d->touchedItem != -1) && (d->touchedItem != i)) {
                int tmptouch = d->touchedItem;
                d->touchedItem = -1;
                acmenu_redrawitem(ctl, tmptouch);
              }
              
              if ((d->selectedIndex != -1) && (d->selectedIndex != i)) {
                int tmpsidx = d->selectedIndex;
                d->selectedIndex = -1;
                acmenu_redrawitem(ctl, tmpsidx);
              }
              
              int prevfocus = d->focusedItem;
              d->focusedItem = i;
              d->touchedItem   = i;
              d->selectedIndex = i;
              
              if ((prevfocus != -1) && (prevfocus != i)) {
                acmenu_redrawitem(ctl, prevfocus);
              }
              
              acmenu_redrawitem(ctl, i);
              ctl->ondraw(ctl);
              aw_draw(ctl->win);
              vibrate(30);
              retmsgx = d->touchmsg;
              msg = aw_msg(retmsgx, 1, 0, 0);
              break;
            }
          }
          
          if ((d->scrollY < 0) || (d->scrollY > d->maxScrollY)) {
            ac_regbounce(ctl, &d->scrollY, d->maxScrollY);
          }
        }
        else if (d->maxScrollY > 0) {
          if (akinetic_uphandler(&d->akin, atev->y)) {
            ac_regfling(ctl, &d->akin, &d->scrollY, d->maxScrollY);
          }
          else if ((d->scrollY < 0) || (d->scrollY > d->maxScrollY)) {
            ac_regbounce(ctl, &d->scrollY, d->maxScrollY);
          }
        }
        
        if (d->touchedItem != -1) {
          usleep(30);
          int tmptouch = d->touchedItem;
          d->touchedItem = -1;
          acmenu_redrawitem(ctl, tmptouch);
          ctl->ondraw(ctl);
          msg = aw_msg(retmsgx, 1, 0, 0);
        }
      }
      break;
      
    case ATEV_MOUSEMV: {
        byte allowscroll = 1;
        
        if (atev->y != 0) {
          if (d->prevTouchY != -50) {
            if (abs(d->prevTouchY - atev->y) >= agdp() * 5) {
              d->prevTouchY = -50;
              
              if (d->touchedItem != -1) {
                int tmptouch = d->touchedItem;
                d->touchedItem = -1;
                acmenu_redrawitem(ctl, tmptouch);
                ctl->ondraw(ctl);
                aw_draw(ctl->win);
              }
            }
            else {
              allowscroll = 0;
            }
          }
          
          if ((allowscroll) && (d->maxScrollY > 0)) {
            int mv = akinetic_movehandler(&d->akin, atev->y);
            
            if (mv != 0) {
              if ((d->scrollY < 0) && (mv < 0)) {
                float dumpsz = 0.6 - (0.6 * (((float) abs(d->scrollY)) / (ctl->h / 8)));
                d->scrollY += floor(mv * dumpsz);
              }
              else if ((d->scrollY > d->maxScrollY) && (mv > 0)) {
                float dumpsz = 0.6 - (0.6 * (((float) abs(d->scrollY - d->maxScrollY)) / (ctl->h / 8)));
                d->scrollY += floor(mv * dumpsz);
              }
              else {
                d->scrollY += mv;
              }
              
              if (d->scrollY < 0 - (ctl->h / 4)) {
                d->scrollY = 0 - (ctl->h / 8);
              }
              
              if (d->scrollY > d->maxScrollY + (ctl->h / 4)) {
                d->scrollY = d->maxScrollY + (ctl->h / 8);
              }
              
              msg = aw_msg(0, 1, 0, 0);
              ctl->ondraw(ctl);
            }
          }
        }
      }
      break;
      
    case ATEV_SELECT: {
        if ((d->focusedItem > -1) && (d->draweditemn > 0)) {
          if (atev->d) {
            if ((d->touchedItem != -1) && (d->touchedItem != d->focusedItem)) {
              int tmptouch = d->touchedItem;
              d->touchedItem = -1;
              acmenu_redrawitem(ctl, tmptouch);
            }
            
            vibrate(30);
            d->touchedItem = d->focusedItem;
            acmenu_redrawitem(ctl, d->focusedItem);
            ctl->ondraw(ctl);
            msg = aw_msg(0, 1, 0, 0);
          }
          else {
            if ((d->touchedItem != -1) && (d->touchedItem != d->focusedItem)) {
              int tmptouch = d->touchedItem;
              d->touchedItem = -1;
              acmenu_redrawitem(ctl, tmptouch);
            }
            
            if ((d->selectedIndex != -1) && (d->selectedIndex != d->focusedItem)) {
              int tmpsidx = d->selectedIndex;
              d->selectedIndex = -1;
              acmenu_redrawitem(ctl, tmpsidx);
            }
            
            d->selectedIndex = d->focusedItem;
            d->touchedItem = -1;
            acmenu_redrawitem(ctl, d->focusedItem);
            ctl->ondraw(ctl);
            // msg=aw_msg(0,1,0,0);
            msg = aw_msg(d->touchmsg, 1, 0, 0);
          }
        }
      }
      break;
      
    case ATEV_DOWN: {
        if ((d->focusedItem < d->itemn - 1) && (d->draweditemn > 0)) {
          int prevfocus = d->focusedItem;
          d->focusedItem++;
          acmenu_redrawitem(ctl, prevfocus);
          acmenu_redrawitem(ctl, d->focusedItem);
          ctl->ondraw(ctl);
          msg = aw_msg(0, 1, 1, 0);
          int reqY = d->items[d->focusedItem]->y - round((ctl->h / 2) - (d->items[d->focusedItem]->h / 2));
          ac_regscrollto(
            ctl,
            &d->scrollY,
            d->maxScrollY,
            reqY,
            &d->focusedItem,
            d->focusedItem
          );
        }
      }
      break;
      
    case ATEV_UP: {
        if ((d->focusedItem > 0) && (d->draweditemn > 0)) {
          int prevfocus = d->focusedItem;
          d->focusedItem--;
          acmenu_redrawitem(ctl, prevfocus);
          acmenu_redrawitem(ctl, d->focusedItem);
          ctl->ondraw(ctl);
          msg = aw_msg(0, 1, 1, 0);
          int reqY = d->items[d->focusedItem]->y - round((ctl->h / 2) - (d->items[d->focusedItem]->h / 2));
          ac_regscrollto(
            ctl,
            &d->scrollY,
            d->maxScrollY,
            reqY,
            &d->focusedItem,
            d->focusedItem
          );
        }
      }
      break;
  }
  
  return msg;
}
byte acmenu_onfocus(void * x) {
  ACONTROLP   ctl = (ACONTROLP) x;
  ACMENUDP   d  = (ACMENUDP) ctl->d;
  d->focused = 1;
  
  if ((d->focusedItem == -1) && (d->itemn > 0)) {
    d->focusedItem = 0;
  }
  
  if ((d->focusedItem != -1) && (d->draweditemn > 0)) {
    acmenu_redrawitem(ctl, d->focusedItem);
  }
  
  ctl->ondraw(ctl);
  return 1;
}
void acmenu_onblur(void * x) {
  ACONTROLP   ctl = (ACONTROLP) x;
  ACMENUDP   d  = (ACMENUDP) ctl->d;
  d->focused = 0;
  
  if ((d->focusedItem != -1) && (d->draweditemn > 0)) {
    acmenu_redrawitem(ctl, d->focusedItem);
  }
  
  ctl->ondraw(ctl);
}
ACONTROLP acmenu(
  AWINDOWP win,
  int x,
  int y,
  int w,
  int h,
  byte touchmsg
) {
  //-- Validate Minimum Size
  if (h < agdp() * 16) {
    h = agdp() * 16;
  }
  
  if (w < agdp() * 20) {
    w = agdp() * 20;
  }
  
  //-- Initializing Text Data
  ACMENUDP d        = (ACMENUDP) malloc(sizeof(ACMENUD));
  memset(d, 0, sizeof(ACMENUD));
  //-- Set Signature
  d->acheck_signature = 144;
  //-- Initializing Canvas
  ag_canvas(&d->control, w, h);
  ag_canvas(&d->control_focused, w, h);
  int minpadding = 4; //max(acfg()->roundsz,4);
  //-- Initializing Client Size
  d->clientWidth  = w - (agdp() * minpadding * 2);
  d->clientTextW  = d->clientWidth - ((agdp() * 34) + (agdp() * acfg()->btnroundsz * 2));
  d->clientTextX  = (agdp() * 31) + (agdp() * acfg()->btnroundsz * 2);
  d->client.data = NULL;
  //-- Draw Control
  ag_draw_ex(&d->control, &win->c, 0, 0, x, y, w, h);
  ag_rect(&d->control, 0, 0, w, h, acfg()->border);
  ag_rect(&d->control, 0, 1, w, h - 2, acfg()->textbg);
  //-- Draw Focused Control
  ag_draw_ex(&d->control_focused, &win->c, 0, 0, x, y, w, h);
  ag_rect(&d->control_focused, 0, 0, w, h, acfg()->selectbg);
  ag_rect(&d->control_focused, 0, 1, w, h - 2, acfg()->textbg);
  //-- Set Scroll Value
  d->scrollY     = 0;
  d->maxScrollY  = 0;
  d->prevTouchY  = -50;
  d->invalidDrawItem = -1;
  //-- Set Data Values
  d->items       = NULL;
  d->itemn       = 0;
  d->touchedItem = -1;
  d->focusedItem = -1;
  d->nextY       = agdp() * minpadding;
  d->draweditemn = 0;
  d->selectedIndex = -1;
  d->touchmsg    = touchmsg;
  ACONTROLP ctl  = malloc(sizeof(ACONTROL));
  ctl->ondestroy = &acmenu_ondestroy;
  ctl->oninput  = &acmenu_oninput;
  ctl->ondraw   = &acmenu_ondraw;
  ctl->onblur   = &acmenu_onblur;
  ctl->onfocus  = &acmenu_onfocus;
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