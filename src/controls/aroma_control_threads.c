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
 * AROMA UI: Thread Manager for Window Controls
 *
 */
#include "../aroma.h"

/*************************[ SCROLL TO ]**************************/
typedef struct  {
  ACONTROLP     ctl;
  int     *     scrollY;
  int           requestY;
  int     *     requestHandler;
  int           requestValue;
} ASCROLLTODATA, * ASCROLLTODATAP;

static void * ac_scrolltothread(void * cookie) {
  ASCROLLTODATAP dt = (ASCROLLTODATAP) cookie;
  
  if (dt->ctl->win->isActived) {
    dt->ctl->win->threadnum++;
  }
  else {
    free(dt);
    return NULL;
  }
  
  dt->ctl->forceNS = 1;
  
  while (dt->scrollY[0] != dt->requestY) {
    int diff = floor(((float) (dt->scrollY[0] - dt->requestY)) * 0.5);
    
    if (abs(diff) < 1) {
      dt->scrollY[0] = dt->requestY;
    }
    else {
      dt->scrollY[0] -= diff;
    }
    
    //-- REDRAW
    dt->ctl->ondraw(dt->ctl);
    aw_draw(dt->ctl->win);
    
    if (dt->requestHandler[0] != dt->requestValue) {
      break;
    }
    
    if (!dt->ctl->win->isActived) {
      break;
    }
    
    if (ontouch()) {
      ACONTROLP nctl = (ACONTROLP) dt->ctl->win->controls[dt->ctl->win->touchIndex];
      
      if (nctl == dt->ctl) {
        break;
      }
    }
  }
  
  dt->ctl->forceNS = 0;
  dt->ctl->win->threadnum--;
  free(dt);
  return NULL;
}
void ac_regscrollto(
  ACONTROLP       ctl,
  int      *      scrollY,
  int             maxScrollY,
  int             requestY,
  int      *      requestHandler,
  int             requestValue
) {
  ASCROLLTODATAP fdt = (ASCROLLTODATAP) malloc(sizeof(ASCROLLTODATA));
  fdt->ctl          = ctl;
  fdt->scrollY      = scrollY;
  fdt->requestY     = requestY;
  fdt->requestHandler   = requestHandler;
  fdt->requestValue     = requestValue;
  
  if (fdt->requestY < 0) {
    fdt->requestY = 0;
  }
  
  if (fdt->requestY > maxScrollY) {
    fdt->requestY = maxScrollY;
  }
  
  if (fdt->requestY != fdt->scrollY[0]) {
    pthread_t threadscrollto;
    pthread_create(&threadscrollto, NULL, ac_scrolltothread, (void *) fdt);
    pthread_detach(threadscrollto);
  }
}

/*************************[ TAP WAIT ]**************************/
typedef struct  {
  ACONTROLP     ctl;
  int     *     moveY;
  int     *     flagpointer;
  int           flagvalue;
} APUSHWAITDATA, * APUSHWAITDATAP;
static void * ac_pushwaitthread(void * cookie) {
  APUSHWAITDATAP dt = (APUSHWAITDATAP) cookie;
  
  if (dt->ctl->win->isActived) {
    dt->ctl->win->threadnum++;
  }
  else {
    free(dt);
    return NULL;
  }
  
  int  waitsz  = 0;
  byte isvalid = 1;
  
  while (++waitsz < 180) {
    if (!dt->ctl->win->isActived) {
      isvalid = 0;
      break;
    }
    
    if (dt->moveY[0] == -50) {
      isvalid = 0;
      break;
    }
    
    usleep(500);
  }
  
  if ((isvalid) && (dt->moveY[0] != -50)) {
    dt->flagpointer[0] = dt->flagvalue;
    dt->ctl->ondraw(dt->ctl);
    aw_draw(dt->ctl->win);
  }
  
  dt->ctl->win->threadnum--;
  free(dt);
  return NULL;
}
void ac_regpushwait(
  ACONTROLP     ctl,
  int     *     moveY,
  int     *     flagpointer,
  int           flagvalue
) {
  APUSHWAITDATAP fdt = (APUSHWAITDATAP) malloc(sizeof(APUSHWAITDATA));
  fdt->ctl         = ctl;
  fdt->moveY       = moveY;
  fdt->flagpointer = flagpointer;
  fdt->flagvalue   = flagvalue;
  pthread_t threadpushwait;
  pthread_create(&threadpushwait, NULL, ac_pushwaitthread, (void *) fdt);
  pthread_detach(threadpushwait);
}


/*************************[ BOUNCE ]**************************/
typedef struct  {
  ACONTROLP     ctl;
  int     *     scrollY;
  int           maxScrollY;
} ABOUNCEDATA, * ABOUNCEDATAP;
static void * ac_bouncethread(void * cookie) {
  ABOUNCEDATAP dt = (ABOUNCEDATAP) cookie;
  
  if (dt->ctl->win->isActived) {
    dt->ctl->win->threadnum++;
  }
  else {
    free(dt);
    return NULL;
  }
  
  int bouncesz    = 0;
  byte bouncetype = 0;
  
  if (dt->scrollY[0] < 0) {
    bouncesz = abs(dt->scrollY[0]);
  }
  else if (dt->scrollY[0] > dt->maxScrollY) {
    bouncetype = 1;
    bouncesz   = dt->scrollY[0] - dt->maxScrollY;
  }
  
  while (bouncesz > 0) {
    if (dt->ctl->forceNS) {
      break;
    }
    
    bouncesz = floor(bouncesz * 0.3);
    
    if (bouncetype) {
      dt->scrollY[0] = dt->maxScrollY + bouncesz;
    }
    else {
      dt->scrollY[0] = 0 - bouncesz;
    }
    
    //-- REDRAW
    dt->ctl->ondraw(dt->ctl);
    aw_draw(dt->ctl->win);
    
    if (!dt->ctl->win->isActived) {
      break;
    }
    
    if (ontouch()) {
      ACONTROLP nctl = (ACONTROLP) dt->ctl->win->controls[dt->ctl->win->touchIndex];
      
      if (nctl == dt->ctl) {
        break;
      }
    }
    
    if (dt->scrollY[0] == 0) {
      break;
    }
    
    if (dt->scrollY[0] == dt->maxScrollY) {
      break;
    }
  }
  
  dt->ctl->win->threadnum--;
  free(dt);
  return NULL;
}
void ac_regbounce(
  ACONTROLP       ctl,
  int      *      scrollY,
  int             maxScrollY
) {
  ABOUNCEDATAP fdt = (ABOUNCEDATAP) malloc(sizeof(ABOUNCEDATA));
  fdt->ctl          = ctl;
  fdt->scrollY      = scrollY;
  fdt->maxScrollY   = maxScrollY;
  pthread_t threadbounce;
  pthread_create(&threadbounce, NULL, ac_bouncethread, (void *) fdt);
  pthread_detach(threadbounce);
}

/*************************[ FLING ]**************************/
typedef struct  {
  ACONTROLP     ctl;
  AKINETIC   *  akin;
  int     *     scrollY;
  int           maxScrollY;
} AFLINGDATA, * AFLINGDATAP;
static void * ac_flingthread(void * cookie) {
  AFLINGDATAP dt = (AFLINGDATAP) cookie;
  
  if (dt->ctl->win->isActived) {
    dt->ctl->win->threadnum++;
  }
  else {
    free(dt);
    return NULL;
  }
  
  int mz  = akinetic_fling(dt->akin);
  float vz = 0.0;
  
  while ((mz != 0) && (dt->ctl->win->isActived)) {
    if (dt->ctl->forceNS) {
      break;
    }
    
    int zz = ceil(dt->akin->velocity);
    /*vz+=dt->akin->velocity-zz;
    if (abs(vz)>=1){
      if (vz<0){
        vz+=1.0;
        zz--;
      }
      else{
        vz-=1.0;
        zz++;
      }
    }*/
    //if (zz!=0){
    dt->scrollY[0] += zz;
    dt->ctl->ondraw(dt->ctl);
    aw_draw(dt->ctl->win);
    //}
    
    if (!dt->ctl->win->isActived) {
      break;
    }
    
    if ((dt->scrollY[0] < 0 - (dt->ctl->h / 4)) || (dt->scrollY[0] > dt->maxScrollY + (dt->ctl->h / 4))) {
      break;
    }
    
    if (ontouch()) {
      ACONTROLP nctl = (ACONTROLP) dt->ctl->win->controls[dt->ctl->win->touchIndex];
      
      if (nctl == dt->ctl) {
        break;
      }
    }
    
    //usleep(4000);
    
    if ((dt->scrollY[0] < 0) || (dt->scrollY[0] > dt->maxScrollY)) {
      mz = akinetic_fling_dampered(dt->akin, 0.4);
    }
    else {
      mz = akinetic_fling(dt->akin);
    }
  }
  
  if (dt->ctl->win->isActived) {
    if ((dt->scrollY[0] < 0) || (dt->scrollY[0] > dt->maxScrollY)) {
      ac_regbounce(dt->ctl, dt->scrollY, dt->maxScrollY);
    }
  }
  
  dt->ctl->win->threadnum--;
  free(dt);
  return NULL;
}

void ac_regfling(
  ACONTROLP       ctl,
  AKINETIC    *   akin,
  int      *      scrollY,
  int             maxScrollY
) {
  AFLINGDATAP fdt = (AFLINGDATAP) malloc(sizeof(AFLINGDATA));
  fdt->ctl          = ctl;
  fdt->akin         = akin;
  fdt->scrollY      = scrollY;
  fdt->maxScrollY   = maxScrollY;
  pthread_t threadfling;
  pthread_create(&threadfling, NULL, ac_flingthread, (void *) fdt);
  pthread_detach(threadfling);
}