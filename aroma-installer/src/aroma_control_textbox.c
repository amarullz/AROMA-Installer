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
 * AROMA UI: Textbox Window Control
 *
 */

#include "aroma.h"

/***************************[ TEXTBOX ]**************************/
typedef struct{
  CANVAS    client;
  CANVAS    control_focused;
  CANVAS    control;
  AKINETIC  akin;
  int       scrollY;
  int       maxScrollY;
  int       targetY;
  byte      focused;
  byte      isbigtxt;
  int       appendPos;
  byte      forceGlowTop;
  byte      isFixedText;
} ACTEXTD, * ACTEXTDP;
dword actext_oninput(void * x,int action,ATEV * atev){
  ACONTROLP ctl= (ACONTROLP) x;
  ACTEXTDP  d  = (ACTEXTDP) ctl->d;
  if (d->maxScrollY==0) return 0;
  dword msg = 0;
  switch (action){
    case ATEV_MOUSEDN:
      {
        akinetic_downhandler(&d->akin,atev->y);
      }
      break;
    case ATEV_MOUSEUP:
      {
        if (akinetic_uphandler(&d->akin,atev->y))
          ac_regfling(ctl,&d->akin,&d->scrollY,d->maxScrollY);
        else if ((d->scrollY<0)||(d->scrollY>d->maxScrollY)){
          ac_regbounce(ctl,&d->scrollY,d->maxScrollY);
        }
      }
      break;
    case ATEV_MOUSEMV:
      {
        if (atev->y!=0){
          int mv = akinetic_movehandler(&d->akin,atev->y);
          if (mv!=0){
            if ((d->scrollY<0)&&(mv<0)){
              float dumpsz = 0.6-(0.6*(((float) abs(d->scrollY))/(ctl->h/4)));
              d->scrollY+=floor(mv*dumpsz);
            }
            else if ((d->scrollY>d->maxScrollY)&&(mv>0)){
              float dumpsz = 0.6-(0.6*(((float) abs(d->scrollY-d->maxScrollY))/(ctl->h/4)));
              d->scrollY+=floor(mv*dumpsz);
            }
            else
              d->scrollY+=mv;

            if (d->scrollY<0-(ctl->h/4)) d->scrollY=0-(ctl->h/4);
            if (d->scrollY>d->maxScrollY+(ctl->h/4)) d->scrollY=d->maxScrollY+(ctl->h/4);
            msg=aw_msg(0,1,0,0);
            ctl->ondraw(ctl);
          }
        }
      }
      break;
      case ATEV_DOWN:
        {
          if (d->scrollY<d->maxScrollY){
            msg=aw_msg(0,1,1,0);
            int reqY = d->scrollY+ceil(ctl->h/8);
            if (reqY>d->maxScrollY) reqY = d->maxScrollY;
            d->targetY=reqY;
            ac_regscrollto(
              ctl,
              &d->scrollY,
              d->maxScrollY,
              reqY,
              &d->targetY,
              d->targetY
            );
          }
        }
      break;
      case ATEV_UP:
        {
          if (d->scrollY>0){
            msg=aw_msg(0,1,1,0);
            int reqY = d->scrollY-ceil(ctl->h/8);
            if (reqY<0) reqY = 0;
            d->targetY=reqY;
            ac_regscrollto(
              ctl,
              &d->scrollY,
              d->maxScrollY,
              reqY,
              &d->targetY,
              d->targetY
            );
          }
        }
      break;
  }
  return msg;
}
void actext_ondraw(void * x){
  ACONTROLP ctl= (ACONTROLP) x;
  ACTEXTDP  d  = (ACTEXTDP) ctl->d;
  CANVAS *  pc = &ctl->win->c;
  
  //-- Init Device Pixel Size
  int minpadding = max(acfg()->roundsz,4);
  int agdp3 = (agdp()*minpadding);
  int agdp6 = (agdp()*(minpadding*2));
  int agdpX = agdp6;
  
  if ((d->focused)&&(!d->isFixedText)){
    ag_draw(pc,&d->control_focused,ctl->x,ctl->y);
    ag_draw_ex(pc,&d->client,ctl->x+agdp3,ctl->y+agdp(),0,d->scrollY+agdp(),ctl->w-agdp6,ctl->h-(agdp()*2));
  }
  else{
    ag_draw(pc,&d->control,ctl->x,ctl->y);
    ag_draw_ex(pc,&d->client,ctl->x+agdp3,ctl->y+1,0,d->scrollY+1,ctl->w-agdp6,ctl->h-2);
  }
  
  
  if ((d->maxScrollY>0)||(d->forceGlowTop)){
    //-- Glow
    int i;
    byte isST=(d->scrollY>=agdp3)?1:0;
    byte isSB=(d->scrollY<=d->maxScrollY-agdp3)?1:0;
    if (d->forceGlowTop) isST=1;

    int add_t_y = 1;
    if (d->focused)
      add_t_y = agdp();
    for (i=0;i<agdpX;i++){
      byte alph = 255-round((((float) (i+1))/ ((float) agdpX))*230);
      if (isST)
        ag_rectopa(pc,ctl->x+agdp3,ctl->y+i+add_t_y,ctl->w-agdpX,1,acfg()->textbg,alph);
      if (isSB)
        ag_rectopa(pc,ctl->x+agdp3,(ctl->y+ctl->h)-(i+1)-add_t_y,ctl->w-agdpX,1,acfg()->textbg,alph);
    }
    
    if (d->maxScrollY>0){
      //-- Scrollbar
      int newh = ctl->h - agdp6;
      float scrdif    = ((float) newh) / ((float) d->client.h);
      int  scrollbarH = round(scrdif * newh);
      int  scrollbarY = round(scrdif * d->scrollY) + agdp3;
      if (d->scrollY<0){
        scrollbarY = agdp3;
        int alp = (1.0 - (((float) abs(d->scrollY)) / (((float) ctl->h)/4))) * 255;
        if (alp<0) alp = 0;
        ag_rectopa(pc,(ctl->w-agdp()-2)+ctl->x,scrollbarY+ctl->y,agdp(),scrollbarH,acfg()->scrollbar, alp);
      }
      else if (d->scrollY>d->maxScrollY){
        scrollbarY = round(scrdif * d->maxScrollY) + agdp3;
        int alp = (1.0 - (((float) abs(d->scrollY-d->maxScrollY)) / (((float) ctl->h)/4))) * 255;
        if (alp<0) alp = 0;
        ag_rectopa(pc,(ctl->w-agdp()-2)+ctl->x,scrollbarY+ctl->y,agdp(),scrollbarH,acfg()->scrollbar, alp);
      }
      else{
        ag_rect(pc,(ctl->w-agdp()-2)+ctl->x,scrollbarY+ctl->y,agdp(),scrollbarH,acfg()->scrollbar);
      }
    }
  }
}
void actext_ondestroy(void * x){
  ACONTROLP ctl= (ACONTROLP) x;
  ACTEXTDP  d  = (ACTEXTDP) ctl->d;
  ag_ccanvas(&d->control);
  ag_ccanvas(&d->control_focused);
  ag_ccanvas(&d->client);
  free(ctl->d);
}
byte actext_onfocus(void * x){
  ACONTROLP   ctl= (ACONTROLP) x;
  ACTEXTDP   d  = (ACTEXTDP) ctl->d;
  d->focused=1;
  ctl->ondraw(ctl);
  return 1;
}
void actext_onblur(void * x){
  ACONTROLP   ctl= (ACONTROLP) x;
  ACTEXTDP   d  = (ACTEXTDP) ctl->d;
  d->focused=0;
  ctl->ondraw(ctl);
}
void actext_appendtxt(ACONTROLP ctl,char * txt){
  ACTEXTDP   d  = (ACTEXTDP) ctl->d;
  int ch          = ag_txtheight(d->client.w,txt,d->isbigtxt);
  int canvas_h    = d->client.h;
  
  if ((d->appendPos+ch)>=canvas_h){
    int step_up = (d->appendPos+ch) - canvas_h;
    int y; int ynew=0;
    for (y=step_up; y<canvas_h; y++){
      color * rowdest = agxy(&d->client,0,ynew++);
      color * rowsrc  = agxy(&d->client,0,y);
      memcpy(rowdest,rowsrc,sizeof(color)*d->client.w);
    }
    d->appendPos -= step_up;
  }
  
  ag_rect(&d->client,0,d->appendPos,d->client.w,ch,acfg()->textbg);
  ag_text(&d->client,
    d->client.w,
    0,d->appendPos,
    txt,
    acfg()->textfg,
    d->isbigtxt);

  d->appendPos+=ch;
  
  /*
  int minpadding = max(acfg()->roundsz,4);
  int ch        = ag_txtheight(d->client.w,txt,d->isbigtxt);
  int my        = d->client.h-(agdp()*2); // -(agdp()*(minpadding*2));
  if ((d->appendPos+ch)>=my){
    if (d->appendPos<my){
      ch-=(my-d->appendPos);
    }
    int y; int ynew=0;
    for (y=ch;y<d->client.h;y++){
      color * rowdest = agxy(&d->client,0,ynew++);
      color * rowsrc  = agxy(&d->client,0,y);
      memcpy(rowdest,rowsrc,sizeof(color)*d->client.w);
    }
    int ypos = my-ch;
    ag_rect(&d->client,0,ypos,d->client.w,ch,acfg()->textbg);
    ag_text(&d->client,
      d->client.w,
      0,ypos,
      txt,
      acfg()->textfg,
      d->isbigtxt);
    d->forceGlowTop=1;
    d->appendPos=my;
  }
  else{
    ag_text(&d->client,
      d->client.w,
      0,d->appendPos,
      txt,
      acfg()->textfg,
      d->isbigtxt);
    d->appendPos+=ch;
  }
  */
  ctl->ondraw(ctl);
  aw_draw(ctl->win);
}
void actext_rebuild(
  ACONTROLP ctl,
  int x,
  int y,
  int w,
  int h,
  char * text,
  byte isbig,
  byte toBottom
){
  ACTEXTDP  d  = (ACTEXTDP) ctl->d;
  int minpadding = max(acfg()->roundsz,4);
  //-- Cleanup
  ag_ccanvas(&d->control);
  ag_ccanvas(&d->control_focused);
  ag_ccanvas(&d->client);
  memset(d,0,sizeof(ACTEXTD));
  
  //-- Rebuild
  //-- Validate Minimum Size
  if (h<agdp()*16) h=agdp()*16;
  if (w<agdp()*16) w=agdp()*16;
    
  //-- Initializing Client Area
  int cw            = w-(agdp()*(minpadding*2));
  int ch            = 0;
  if (text!=NULL)
    ch = ag_txtheight(cw,text,isbig)+(agdp()*(minpadding*2));
  else
    ch = h-(agdp()*2);

  //-- Initializing Canvas
  ag_canvas(&d->control,w,h);
  ag_canvas(&d->control_focused,w,h);
  ag_canvas(&d->client,cw,ch);
  
  //-- Draw Control
  ag_draw_ex(&d->control,ctl->win->bg,0,0,x,y,w,h);
  ag_roundgrad(&d->control,0,0,w,h,acfg()->border,acfg()->border_g,(agdp()*acfg()->roundsz));
  ag_roundgrad(&d->control,1,1,w-2,h-2,acfg()->textbg,acfg()->textbg,(agdp()*acfg()->roundsz)-1);
  
  //-- Draw Focused Control
  ag_draw_ex(&d->control_focused,ctl->win->bg,0,0,x,y,w,h);
  ag_roundgrad(&d->control_focused,0,0,w,h,acfg()->selectbg,acfg()->selectbg_g,(agdp()*acfg()->roundsz));
  ag_roundgrad(&d->control_focused,agdp(),agdp(),w-(agdp()*2),h-(agdp()*2),acfg()->textbg,acfg()->textbg,(agdp()*(acfg()->roundsz-1)));
  
  //-- Draw Client
  ag_rect(&d->client,0,0,cw,ch,acfg()->textbg);
  if (text!=NULL)
    ag_text(&d->client,cw,0,agdp()*minpadding,text,acfg()->textfg,isbig);
  
  d->isbigtxt    = isbig;
  d->targetY     = 0;
  d->focused     = 0;
  d->scrollY     = 0;
  d->appendPos   = agdp()*minpadding;
  d->forceGlowTop= 0;
  d->isFixedText = 0;
  if (text!=NULL)
    d->maxScrollY  = ch-(h-(agdp()*minpadding));
  else{
    d->maxScrollY  = 0;
    d->isFixedText = 1;
  }
  if (d->maxScrollY<0) d->maxScrollY=0;
  ctl->x        = x;
  ctl->y        = y;
  ctl->w        = w;
  ctl->h        = h;
  ctl->forceNS  = 0;
  
  if (toBottom){
    d->scrollY = d->maxScrollY;
  }
  
  ctl->ondraw(ctl);
  aw_draw(ctl->win);
}
ACONTROLP actext(
  AWINDOWP win,
  int x,
  int y,
  int w,
  int h,
  char * text,
  byte isbig
){
  //-- Validate Minimum Size
  if (h<agdp()*16) h=agdp()*16;
  if (w<agdp()*16) w=agdp()*16;
    
  //-- Initializing Client Area
  int minpadding = max(acfg()->roundsz,4);
  int cw            = w-(agdp()*(minpadding*2));
  int ch            = 0;
  if (text!=NULL)
    ch = ag_txtheight(cw,text,isbig)+(agdp()*(minpadding*2));
  else
    ch = h-(agdp()*2);
  
  //-- Initializing Text Data
  ACTEXTDP d        = (ACTEXTDP) malloc(sizeof(ACTEXTD));
  memset(d,0,sizeof(ACTEXTD));
  
  //-- Initializing Canvas
  ag_canvas(&d->control,w,h);
  ag_canvas(&d->control_focused,w,h);
  ag_canvas(&d->client,cw,ch);
  
  //-- Draw Control
  ag_draw_ex(&d->control,&win->c,0,0,x,y,w,h);
  ag_roundgrad(&d->control,0,0,w,h,acfg()->border,acfg()->border_g,(agdp()*acfg()->roundsz));
  ag_roundgrad(&d->control,1,1,w-2,h-2,acfg()->textbg,acfg()->textbg,(agdp()*acfg()->roundsz)-1);
  
  //-- Draw Focused Control
  ag_draw_ex(&d->control_focused,&win->c,0,0,x,y,w,h);
  ag_roundgrad(&d->control_focused,0,0,w,h,acfg()->selectbg,acfg()->selectbg_g,(agdp()*acfg()->roundsz));
  ag_roundgrad(&d->control_focused,agdp(),agdp(),w-(agdp()*2),h-(agdp()*2),acfg()->textbg,acfg()->textbg,(agdp()*(acfg()->roundsz-1)));
  
  
  //-- Draw Client
  ag_rect(&d->client,0,0,cw,ch,acfg()->textbg);
  if (text!=NULL)
    ag_text(&d->client,cw,0,agdp()*minpadding,text,acfg()->textfg,isbig);
  
  d->isbigtxt    = isbig;
  d->targetY     = 0;
  d->focused     = 0;
  d->scrollY     = 0;
  d->appendPos   = agdp()*minpadding;
  d->forceGlowTop= 0;
  d->isFixedText = 0;
  if (text!=NULL)
    d->maxScrollY  = ch-(h-(agdp()*minpadding));
  else{
    d->maxScrollY  = 0;
    d->isFixedText = 1;
  }
  if (d->maxScrollY<0) d->maxScrollY=0;
  
  ACONTROLP ctl  = malloc(sizeof(ACONTROL));
  ctl->ondestroy= &actext_ondestroy;
  ctl->oninput  = &actext_oninput;
  ctl->ondraw   = &actext_ondraw;
  ctl->onblur   = actext_onblur;
  ctl->onfocus  = actext_onfocus;
  ctl->win      = win;
  ctl->x        = x;
  ctl->y        = y;
  ctl->w        = w;
  ctl->h        = h;
  ctl->forceNS  = 0;
  ctl->d        = (void *) d;
  aw_add(win,ctl);
  return ctl;
}
