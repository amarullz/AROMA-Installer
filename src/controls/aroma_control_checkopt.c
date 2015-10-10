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
 * AROMA UI: Checkbox Optbox Hybrid List Window Control
 *
 */
#include "../aroma.h"

#define ACCHKOPT_MAX_GROUP   64

/***************************[ CHECKBOX ]**************************/
typedef struct {
  char iid[32];
  char title[64];
  char desc[128];
  byte checked;
  int  id;
  int  h;
  int  y;
  
  /* Title & Desc Size/Pos */
  int  th;
  int  dh;
  int  ty;
  int  dy;
  
  /* Type */
  byte isTitle;
  int  group;
  int  groupid;
  byte  type;
} ACCHKOPTI, * ACCHKOPTIP;
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
  ACCHKOPTIP * items;
  int       itemn;
  int       touchedItem;
  int       focusedItem;
  int       draweditemn;
  
  int       groupCounts;
  int       groupCurrId;
  int       selectedIndexs[ACCHKOPT_MAX_GROUP];
  
  /* Focus */
  byte      focused;
} ACCHKOPTD, * ACCHKOPTDP;
void acchkopt_ondestroy(void * x) {
  ACONTROLP ctl = (ACONTROLP) x;
  ACCHKOPTDP d  = (ACCHKOPTDP) ctl->d;
  ag_ccanvas(&d->control);
  ag_ccanvas(&d->control_focused);
  
  if (d->itemn > 0) {
    int i;
    
    for (i = 0; i < d->itemn; i++) {
      free(d->items[i]);
    }
    
    free(d->items);
    ag_ccanvas(&d->client);
  }
  
  free(ctl->d);
}
int acchkopt_itemcount(ACONTROLP ctl) {
  ACCHKOPTDP d = (ACCHKOPTDP) ctl->d;
  
  if (d->acheck_signature != 215) {
    return -1;
  }
  
  return d->itemn;
}
byte acchkopt_itemtype(ACONTROLP ctl, int index) {
  ACCHKOPTDP d = (ACCHKOPTDP) ctl->d;
  
  if (d->acheck_signature != 215) {
    return 0;
  }
  
  if (index < d->itemn) {
    return d->items[index]->type;
  }
  
  return 0;
}
byte acchkopt_ischecked(ACONTROLP ctl, int index) {
  ACCHKOPTDP d = (ACCHKOPTDP) ctl->d;
  
  if (d->acheck_signature != 215) {
    return 0;
  }
  
  if (index < d->itemn) {
    return d->items[index]->checked;
  }
  
  return 0;
}
int acchkopt_getselectedindex(ACONTROLP ctl, int group) {
  if ((group < 0) || (group >= ACCHKOPT_MAX_GROUP)) {
    return -1;
  }
  
  ACCHKOPTDP d = (ACCHKOPTDP) ctl->d;
  
  if (d->acheck_signature != 215) {
    return -1;  //-- Not Valid Signature
  }
  
  return d->selectedIndexs[group];
}
byte acchkopt_isgroup(ACONTROLP ctl, int index) {
  ACCHKOPTDP d = (ACCHKOPTDP) ctl->d;
  
  if (d->acheck_signature != 215) {
    return 0;
  }
  
  return d->items[index]->isTitle;
}
int acchkopt_getgroup(ACONTROLP ctl, int index) {
  ACCHKOPTDP d = (ACCHKOPTDP) ctl->d;
  
  if (d->acheck_signature != 215) {
    return 0;
  }
  
  return d->items[index]->group;
}
int acchkopt_getgroupid(ACONTROLP ctl, int index) {
  ACCHKOPTDP d = (ACCHKOPTDP) ctl->d;
  
  if (d->acheck_signature != 215) {
    return 0;
  }
  
  return d->items[index]->groupid;
}
char * acchkopt_getitemiid(ACONTROLP ctl, int index) {
  ACCHKOPTDP d = (ACCHKOPTDP) ctl->d;
  
  if (d->acheck_signature != 215) {
    return 0;
  }
  
  return d->items[index]->iid;
}
void acchkopt_redrawitem(ACONTROLP ctl, int index) {
  ACCHKOPTDP d = (ACCHKOPTDP) ctl->d;
  
  if (d->acheck_signature != 215) {
    return;  //-- Not Valid Signature
  }
  
  if ((index >= d->itemn) || (index < 0)) {
    return;  //-- Not Valid Index
  }
  
  ACCHKOPTIP p = d->items[index];
  CANVAS  * c = &d->client;
  //-- Cleanup Background
  ag_rect(c, 0, p->y, d->clientWidth, p->h, acfg()->textbg);
  
  if (p->isTitle) {
    ag_roundgrad(c, 0, p->y, d->clientWidth, p->h, acfg()->titlebg, acfg()->titlebg_g, 0);
    ag_textf(c, d->clientTextW + (agdp() * 14), (d->clientTextX - (agdp() * 14)) + 1, p->y + p->ty, p->title, acfg()->titlebg_g, 0);
    ag_text(c, d->clientTextW + (agdp() * 14), d->clientTextX - (agdp() * 14), p->y + p->ty - 1, p->title, acfg()->titlefg, 0);
  }
  else {
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
    
    //-- Now Draw The Text
    if (isselectcolor) {
      ag_textf(c, d->clientTextW, d->clientTextX, p->y + p->ty, p->title, acfg()->selectbg_g, 0);
      ag_textf(c, d->clientTextW, d->clientTextX, p->y + p->dy, p->desc, acfg()->selectbg_g, 0);
    }
    
    ag_text(c, d->clientTextW, d->clientTextX - 1, p->y + p->ty - 1, p->title, txtcolor, 0);
    ag_text(c, d->clientTextW, d->clientTextX - 1, p->y + p->dy - 1, p->desc, graycolor, 0);
    //-- Now Draw The Checkbox
    int halfdp   = ceil(((float) agdp()) / 2);
    int halfdp2  = halfdp * 2;
    int chkbox_s = (agdp() * 10);
    int chkbox_x = round((d->clientTextX / 2) - ((chkbox_s + 2) / 2));
    int chkbox_y = p->y + round((p->h / 2) - (chkbox_s / 2));
    byte drawed = 0;
    int minpad = 3 * agdp();
    int addpad = 6 * agdp();
    
    if (p->type) {
      if (p->id == d->selectedIndexs[p->group]) {
        if (index == d->touchedItem) {
          drawed = atheme_draw("img.radio.on.push", c, chkbox_x - minpad, chkbox_y - minpad, chkbox_s + addpad, chkbox_s + addpad);
        }
        else if ((index == d->focusedItem) && (d->focused)) {
          drawed = atheme_draw("img.radio.on.focus", c, chkbox_x - minpad, chkbox_y - minpad, chkbox_s + addpad, chkbox_s + addpad);
        }
        else {
          drawed = atheme_draw("img.radio.on", c, chkbox_x - minpad, chkbox_y - minpad, chkbox_s + addpad, chkbox_s + addpad);
        }
      }
      else {
        if (index == d->touchedItem) {
          drawed = atheme_draw("img.radio.push", c, chkbox_x - minpad, chkbox_y - minpad, chkbox_s + addpad, chkbox_s + addpad);
        }
        else if ((index == d->focusedItem) && (d->focused)) {
          drawed = atheme_draw("img.radio.focus", c, chkbox_x - minpad, chkbox_y - minpad, chkbox_s + addpad, chkbox_s + addpad);
        }
        else {
          drawed = atheme_draw("img.radio", c, chkbox_x - minpad, chkbox_y - minpad, chkbox_s + addpad, chkbox_s + addpad);
        }
      }
    }
    else {
      if (p->checked) {
        if (index == d->touchedItem) {
          drawed = atheme_draw("img.checkbox.on.push", c, chkbox_x - minpad, chkbox_y - minpad, chkbox_s + addpad, chkbox_s + addpad);
        }
        else if ((index == d->focusedItem) && (d->focused)) {
          drawed = atheme_draw("img.checkbox.on.focus", c, chkbox_x - minpad, chkbox_y - minpad, chkbox_s + addpad, chkbox_s + addpad);
        }
        else {
          drawed = atheme_draw("img.checkbox.on", c, chkbox_x - minpad, chkbox_y - minpad, chkbox_s + addpad, chkbox_s + addpad);
        }
      }
      else {
        if (index == d->touchedItem) {
          drawed = atheme_draw("img.checkbox.push", c, chkbox_x - minpad, chkbox_y - minpad, chkbox_s + addpad, chkbox_s + addpad);
        }
        else if ((index == d->focusedItem) && (d->focused)) {
          drawed = atheme_draw("img.checkbox.focus", c, chkbox_x - minpad, chkbox_y - minpad, chkbox_s + addpad, chkbox_s + addpad);
        }
        else {
          drawed = atheme_draw("img.checkbox", c, chkbox_x - minpad, chkbox_y - minpad, chkbox_s + addpad, chkbox_s + addpad);
        }
      }
    }
    
    if (!drawed) {
      if (p->type) {
        ag_roundgrad(c,
                     chkbox_x,
                     chkbox_y,
                     chkbox_s,
                     chkbox_s,
                     acfg()->controlbg_g,
                     acfg()->controlbg,
                     chkbox_s
                    );
        ag_roundgrad(c,
                     chkbox_x + halfdp,
                     chkbox_y + halfdp,
                     chkbox_s - halfdp2,
                     chkbox_s - halfdp2,
                     acfg()->textbg,
                     acfg()->textbg,
                     chkbox_s - halfdp);
                     
        if (p->id == d->selectedIndexs[p->group]) {
          ag_roundgrad(c,
                       chkbox_x + halfdp2,
                       chkbox_y + halfdp2,
                       chkbox_s - (halfdp2 * 2),
                       chkbox_s - (halfdp2 * 2),
                       acfg()->selectbg,
                       acfg()->selectbg_g,
                       chkbox_s - halfdp2);
        }
      }
      else {
        ag_roundgrad(c,
                     chkbox_x,
                     chkbox_y,
                     chkbox_s,
                     chkbox_s,
                     acfg()->controlbg_g,
                     acfg()->controlbg,
                     0);
        ag_roundgrad(c,
                     chkbox_x + halfdp,
                     chkbox_y + halfdp,
                     chkbox_s - halfdp2,
                     chkbox_s - halfdp2,
                     acfg()->textbg,
                     acfg()->textbg,
                     0);
                     
        if (p->checked) {
          ag_roundgrad(c,
                       chkbox_x + halfdp2,
                       chkbox_y + halfdp2,
                       chkbox_s - (halfdp2 * 2),
                       chkbox_s - (halfdp2 * 2),
                       acfg()->selectbg,
                       acfg()->selectbg_g,
                       0);
        }
      }
    }
  }
}
void acchkopt_redraw(ACONTROLP ctl) {
  ACCHKOPTDP d = (ACCHKOPTDP) ctl->d;
  
  if (d->acheck_signature != 215) {
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
      acchkopt_redrawitem(ctl, i);
    }
    
    d->draweditemn = d->itemn;
  }
}
//-- Add Item Into Control
byte acchkopt_add(ACONTROLP ctl, char * id, char * title, char * desc, byte checked, byte type) {
  ACCHKOPTDP d = (ACCHKOPTDP) ctl->d;
  
  if (d->acheck_signature != 215) {
    return 0;  //-- Not Valid Signature
  }
  
  //-- Allocating Memory For Item Data
  ACCHKOPTIP newip = (ACCHKOPTIP) malloc(sizeof(ACCHKOPTI));
  snprintf(newip->iid, 32, "%s", id);
  snprintf(newip->title, 64, "%s", title);
  snprintf(newip->desc, 128, "%s", desc);
  newip->th       = ag_txtheight(d->clientTextW, newip->title, 0);
  newip->dh       = ag_txtheight(d->clientTextW, newip->desc, 0);
  newip->ty       = agdp() * 5;
  newip->dy       = (agdp() * 5) + newip->th;
  newip->h        = (agdp() * 10) + newip->dh + newip->th;
  newip->type     = type;
  
  if (newip->h < (agdp() * 22)) {
    newip->h = (agdp() * 22);
  }
  
  newip->checked  = checked;
  newip->id       = d->itemn;
  newip->group    = d->groupCounts;
  newip->groupid  = ++d->groupCurrId;
  newip->isTitle  = 0;
  newip->y        = d->nextY;
  d->nextY       += newip->h;
  
  if (checked && type) {
    d->selectedIndexs[newip->group] = newip->id;
  }
  
  if (d->itemn > 0) {
    int i;
    ACCHKOPTIP * tmpitms   = d->items;
    d->items              = malloc( sizeof(ACCHKOPTIP) * (d->itemn + 1) );
    
    for (i = 0; i < d->itemn; i++) {
      d->items[i] = tmpitms[i];
    }
    
    d->items[d->itemn] = newip;
    free(tmpitms);
  }
  else {
    d->items    = malloc(sizeof(ACCHKOPTIP));
    d->items[0] = newip;
  }
  
  d->itemn++;
  return 1;
}
//-- Add Item Into Control
byte acchkopt_addgroup(ACONTROLP ctl, char * id, char * title, char * desc) {
  ACCHKOPTDP d = (ACCHKOPTDP) ctl->d;
  
  if (d->acheck_signature != 215) {
    return 0;  //-- Not Valid Signature
  }
  
  if (d->groupCounts + 1 >= ACCHKOPT_MAX_GROUP) {
    return 0;
  }
  
  //-- Allocating Memory For Item Data
  ACCHKOPTIP newip = (ACCHKOPTIP) malloc(sizeof(ACCHKOPTI));
  snprintf(newip->iid, 32, "%s", id);
  snprintf(newip->title, 64, "%s", title);
  snprintf(newip->desc, 128, "%s", desc);
  newip->th       = ag_txtheight(d->clientTextW + (agdp() * 14), newip->title, 0);
  newip->dh       = 0;
  newip->ty       = agdp() * 3;
  newip->dy       = (agdp() * 3) + newip->th;
  newip->h        = (agdp() * 6) + newip->dh + newip->th;
  newip->id       = d->itemn;
  newip->group    = ++d->groupCounts;
  d->groupCurrId  = -1;
  newip->groupid  = -1;
  newip->isTitle  = 1;
  newip->y        = d->nextY;
  d->nextY       += newip->h;
  
  if (d->itemn > 0) {
    int i;
    ACCHKOPTIP * tmpitms   = d->items;
    d->items              = malloc( sizeof(ACCHKOPTIP) * (d->itemn + 1) );
    
    for (i = 0; i < d->itemn; i++) {
      d->items[i] = tmpitms[i];
    }
    
    d->items[d->itemn] = newip;
    free(tmpitms);
  }
  else {
    d->items    = malloc(sizeof(ACCHKOPTIP));
    d->items[0] = newip;
  }
  
  d->itemn++;
  return 1;
}

void acchkopt_ondraw(void * x) {
  ACONTROLP   ctl = (ACONTROLP) x;
  ACCHKOPTDP   d  = (ACCHKOPTDP) ctl->d;
  CANVAS   *  pc = &ctl->win->c;
  acchkopt_redraw(ctl);
  
  if (d->invalidDrawItem != -1) {
    d->touchedItem = d->invalidDrawItem;
    acchkopt_redrawitem(ctl, d->invalidDrawItem);
    d->invalidDrawItem = -1;
  }
  
  //-- Init Device Pixel Size
  int minpadding = 4; // max(acfg()->roundsz,4);
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
dword acchkopt_oninput(void * x, int action, ATEV * atev) {
  ACONTROLP ctl = (ACONTROLP) x;
  ACCHKOPTDP d  = (ACCHKOPTDP) ctl->d;
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
        if ((d->prevTouchY != -50) && (abs(d->prevTouchY - atev->y) < agdp() * 5)) {
          d->prevTouchY = -50;
          int touchpos = atev->y - ctl->y + d->scrollY;
          int i;
          
          for (i = 0; i < d->itemn; i++) {
            if ((!d->items[i]->isTitle) && (touchpos >= d->items[i]->y) && (touchpos < d->items[i]->y + d->items[i]->h)) {
              if (d->items[i]->type) {
                if ((d->touchedItem != -1) && (d->touchedItem != i)) {
                  int tmptouch = d->touchedItem;
                  d->touchedItem = -1;
                  acchkopt_redrawitem(ctl, tmptouch);
                }
                
                int grp = d->items[i]->group;
                
                if ((d->selectedIndexs[grp] != -1) && (d->selectedIndexs[grp] != i)) {
                  int tmpsidx = d->selectedIndexs[grp];
                  d->selectedIndexs[grp] = -1;
                  acchkopt_redrawitem(ctl, tmpsidx);
                }
                
                int prevfocus               = d->focusedItem;
                d->focusedItem              = i;
                d->touchedItem              = i;
                d->selectedIndexs[grp]  = i;
                
                if ((prevfocus != -1) && (prevfocus != i)) {
                  acchkopt_redrawitem(ctl, prevfocus);
                }
                
                acchkopt_redrawitem(ctl, i);
                ctl->ondraw(ctl);
                aw_draw(ctl->win);
                vibrate(30);
                break;
              }
              else {
                d->items[i]->checked = (d->items[i]->checked) ? 0 : 1;
                
                if ((d->touchedItem != -1) && (d->touchedItem != i)) {
                  int tmptouch = d->touchedItem;
                  d->touchedItem = -1;
                  acchkopt_redrawitem(ctl, tmptouch);
                }
                
                int prevfocus = d->focusedItem;
                d->focusedItem = i;
                d->touchedItem = i;
                
                if ((prevfocus != -1) && (prevfocus != i)) {
                  acchkopt_redrawitem(ctl, prevfocus);
                }
                
                acchkopt_redrawitem(ctl, i);
                ctl->ondraw(ctl);
                aw_draw(ctl->win);
                vibrate(30);
                break;
              }
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
          acchkopt_redrawitem(ctl, tmptouch);
          ctl->ondraw(ctl);
          msg = aw_msg(0, 1, 0, 0);
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
                acchkopt_redrawitem(ctl, tmptouch);
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
              acchkopt_redrawitem(ctl, tmptouch);
            }
            
            vibrate(30);
            d->touchedItem = d->focusedItem;
            acchkopt_redrawitem(ctl, d->focusedItem);
            ctl->ondraw(ctl);
            msg = aw_msg(0, 1, 0, 0);
          }
          else {
            if ((d->touchedItem != -1) && (d->touchedItem != d->focusedItem)) {
              int tmptouch = d->touchedItem;
              d->touchedItem = -1;
              acchkopt_redrawitem(ctl, tmptouch);
            }
            
            if (d->items[d->focusedItem]->type) {
              int grp = d->items[d->focusedItem]->group;
              
              if ((d->selectedIndexs[grp] != -1) && (d->selectedIndexs[grp] != d->focusedItem)) {
                int tmpsidx = d->selectedIndexs[grp];
                d->selectedIndexs[grp] = -1;
                acchkopt_redrawitem(ctl, tmpsidx);
              }
              
              d->selectedIndexs[grp] = d->focusedItem;
              d->touchedItem = -1;
              acchkopt_redrawitem(ctl, d->focusedItem);
              ctl->ondraw(ctl);
              msg = aw_msg(0, 1, 0, 0);
            }
            else {
              d->items[d->focusedItem]->checked = (d->items[d->focusedItem]->checked) ? 0 : 1;
              d->touchedItem = -1;
              acchkopt_redrawitem(ctl, d->focusedItem);
              ctl->ondraw(ctl);
              msg = aw_msg(0, 1, 0, 0);
            }
          }
        }
      }
      break;
      
    case ATEV_DOWN: {
        if ((d->focusedItem < d->itemn - 1) && (d->draweditemn > 0)) {
          int prevfocus = d->focusedItem;
          d->focusedItem++;
          
          while (d->items[d->focusedItem]->isTitle) {
            d->focusedItem++;
            
            if (d->focusedItem > d->itemn - 1) {
              d->focusedItem = prevfocus;
              return 0;
            }
          }
          
          acchkopt_redrawitem(ctl, prevfocus);
          acchkopt_redrawitem(ctl, d->focusedItem);
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
          
          while (d->items[d->focusedItem]->isTitle) {
            d->focusedItem--;
            
            if (d->focusedItem < 0) {
              d->focusedItem = prevfocus;
              return 0;
            }
          }
          
          acchkopt_redrawitem(ctl, prevfocus);
          acchkopt_redrawitem(ctl, d->focusedItem);
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
byte acchkopt_onfocus(void * x) {
  ACONTROLP   ctl = (ACONTROLP) x;
  ACCHKOPTDP   d  = (ACCHKOPTDP) ctl->d;
  d->focused = 1;
  
  if ((d->focusedItem == -1) && (d->itemn > 0)) {
    d->focusedItem = 0;
  }
  
  if ((d->focusedItem != -1) && (d->draweditemn > 0)) {
    acchkopt_redrawitem(ctl, d->focusedItem);
  }
  
  ctl->ondraw(ctl);
  return 1;
}
void acchkopt_onblur(void * x) {
  ACONTROLP   ctl = (ACONTROLP) x;
  ACCHKOPTDP   d  = (ACCHKOPTDP) ctl->d;
  d->focused = 0;
  
  if ((d->focusedItem != -1) && (d->draweditemn > 0)) {
    acchkopt_redrawitem(ctl, d->focusedItem);
  }
  
  ctl->ondraw(ctl);
}
ACONTROLP acchkopt(
  AWINDOWP win,
  int x,
  int y,
  int w,
  int h
) {
  //-- Validate Minimum Size
  if (h < agdp() * 16) {
    h = agdp() * 16;
  }
  
  if (w < agdp() * 20) {
    w = agdp() * 20;
  }
  
  //-- Initializing Text Data
  ACCHKOPTDP d        = (ACCHKOPTDP) malloc(sizeof(ACCHKOPTD));
  memset(d, 0, sizeof(ACCHKOPTD));
  //-- Set Signature
  d->acheck_signature = 215;
  //-- Initializing Canvas
  ag_canvas(&d->control, w, h);
  ag_canvas(&d->control_focused, w, h);
  int minpadding = 4; // max(acfg()->roundsz,4);
  //-- Initializing Client Size
  d->clientWidth  = w - (agdp() * minpadding * 2);
  d->clientTextW  = d->clientWidth - (agdp() * 18) - (agdp() * acfg()->btnroundsz * 2);
  d->clientTextX  = (agdp() * 18) + (agdp() * acfg()->btnroundsz * 2);
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
  int i;
  
  for (i = 0; i < ACCHKOPT_MAX_GROUP; i++) {
    d->selectedIndexs[i] = -1;
  }
  
  d->groupCounts   = 0;
  d->groupCurrId   = -1;
  ACONTROLP ctl  = malloc(sizeof(ACONTROL));
  ctl->ondestroy = &acchkopt_ondestroy;
  ctl->oninput  = &acchkopt_oninput;
  ctl->ondraw   = &acchkopt_ondraw;
  ctl->onblur   = &acchkopt_onblur;
  ctl->onfocus  = &acchkopt_onfocus;
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