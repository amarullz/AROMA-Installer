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
 * FROM AROMA CORE 2
 * =================
 * AROMA CORE - Input Device Handler
 *
 * Proccess taken from awesome TWRP Input Handler:
 * https://github.com/TeamWin/Team-Win-Recovery-Project/blob/master/minuitwrp/events.c
 *
 * BIG THANKS TO : agrabren
 *
 */
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
 * Input Event Hook and Manager
 *
 */


#include "../../aroma.h"

/* Input Define */
#define AIP_DEVICE  "/dev/input"
#define AIP_MAXDEV              16
#define AIP_SYN_REPORT          0x00
#define AIP_SYN_CONFIG          0x01
#define AIP_ABS_X			          0x00
#define AIP_ABS_Y			          0x01
#define AIP_SYN_MT_REPORT       0x02
#define AIP_ABS_MT_POSITION     0x2a
#define AIP_ABS_MT_AMPLITUDE    0x2b
#define AIP_ABS_MT_SLOT         0x2f
#define AIP_ABS_MT_TOUCH_MAJOR  0x30
#define AIP_ABS_MT_TOUCH_MINOR  0x31
#define AIP_ABS_MT_WIDTH_MAJOR  0x32
#define AIP_ABS_MT_WIDTH_MINOR  0x33
#define AIP_ABS_MT_ORIENTATION  0x34
#define AIP_ABS_MT_POSITION_X   0x35
#define AIP_ABS_MT_POSITION_Y   0x36
#define AIP_ABS_MT_TOOL_TYPE    0x37
#define AIP_ABS_MT_BLOB_ID      0x38
#define AIP_ABS_MT_TRACKING_ID  0x39
#define AIP_ABS_MT_PRESSURE     0x3a
#define AIP_ABS_MT_DISTANCE     0x3b

#define AIP_TRANS_IGNONE        0x0
#define AIP_TRANS_KEY           0x1
#define AIP_TRANS_TOUCH         0x2
#define AIP_TRANS_NONE          0x3


/* Position Structure */
typedef struct {
  int x, y;
  int synced;
  struct input_absinfo xi, yi;
} AIP_POSITION, * AIP_POSITIONP;

/* Virtualkey Structure */
typedef struct {
    int scan;
    int x;
    int y;
    int w;
    int h;
} AIP_VK, * AIP_VKP;

/* Input Event Structure */
typedef struct {
  int               fd_id;
  AIP_VKP           vks;
  int               vkn;
  char              device_name[64];
  byte              ignored;
  byte              down;
  AIP_POSITION      p;
  AIP_POSITION      mt_p;
} AIP_EV, * AIP_EVP;

/* Input Events Structure */
typedef struct {
  int           n;
  struct pollfd fds[AIP_MAXDEV];
  AIP_EV        evs[AIP_MAXDEV];
  
  /* CONFIG */
  byte              touch_swap_xy;
  byte              touch_flip_x;
  byte              touch_flip_y;
} AIP_VARS, * AIP_VARSP;

AIP_VARSP _aip=NULL;

/* Returns empty tokens */
static char * aipStrTokR(char *str, const char *delim, char **save_str){
  if(!str){
    if(!*save_str){
      return NULL;
    }
    str = (*save_str) + 1;
  }
  *save_str = strpbrk(str, delim);
  if (*save_str){
    **save_str = '\0';
  }  
  return str;
}

/* Init Event Device Properties */
static byte aipInitEventDev(AIP_EVP e){
  /* Variables */
  char    vk_path[PATH_MAX] = "/sys/board_properties/virtualkeys.";
  char    vks[2048], *ts    = NULL;
  ssize_t len;
  int     vk_fd;
  int     i;

  e->vkn = 0;
  len = strlen(vk_path);
  len = ioctl(_aip->fds[e->fd_id].fd, EVIOCGNAME(sizeof(e->device_name)), e->device_name);
  if (len <= 0){
      return 0;
  }

  /* Blacklist these "input" devices */
  if (strcmp(e->device_name, "bma250") == 0){
      e->ignored = 1;
  }

  /* virtualkeys.{device_name} */
  strcat(vk_path, e->device_name);

  /* Some devices split the keys from the touchscreen */
  e->vkn = 0;
  vk_fd = open(vk_path, O_RDONLY);
  if (vk_fd >= 0){
    /* Read Contents */
    len = read(vk_fd, vks, sizeof(vks)-1);
    close(vk_fd);
    
    /* Return False on Failed */
    if (len<=0){
      return 0;
    }
    
    /* Add string break */
    vks[len] = '\0';
    
    /* Parse a line like:
        keytype:keycode:centerx:centery:width:height:keytype2:keycode2:centerx2:...
    */
    for (ts=vks, e->vkn=1; *ts; ++ts) {
      if (*ts == ':'){
        e->vkn++;
      }
    }
    
    e->vkn /= 6;
    if (e->vkn <= 0){
      return 0;
    }
    e->down = 0;
  }

  /* IOCTL ABS DEVICE */
  ioctl(_aip->fds[e->fd_id].fd, EVIOCGABS(AIP_ABS_X), &e->p.xi);
  ioctl(_aip->fds[e->fd_id].fd, EVIOCGABS(AIP_ABS_Y), &e->p.yi);
  ioctl(_aip->fds[e->fd_id].fd, EVIOCGABS(AIP_ABS_MT_POSITION_X), &e->mt_p.xi);
  ioctl(_aip->fds[e->fd_id].fd, EVIOCGABS(AIP_ABS_MT_POSITION_Y), &e->mt_p.yi);
  e->p.synced = 0;
  e->mt_p.synced = 0;
  
  /* LOGS */
  printf("\nDEVICE NAME: %s - %s\n", e->device_name, vk_path);
  printf("EV ST: minX: %d  maxX: %d  minY: %d  maxY: %d\n", e->p.xi.minimum, e->p.xi.maximum, e->p.yi.minimum, e->p.yi.maximum);
  printf("EV MT: minX: %d  maxX: %d  minY: %d  maxY: %d\n", e->mt_p.xi.minimum, e->mt_p.xi.maximum, e->mt_p.yi.minimum, e->mt_p.yi.maximum);


  /* Allocate Virtualkeys Count */
  e->vks = malloc(sizeof(AIP_VK) * e->vkn);
  for (i=0; i<e->vkn; ++i) {
    char * token[6];
    int j;

    for (j=0; j<6; ++j) {
      token[j] = aipStrTokR((i||j)?NULL:vks, ":", &ts);
    }

    if (strcmp(token[0],"0x01") != 0) {
      continue;
    }

    /* Dump It */
    e->vks[i].scan  = strtol(token[1], NULL, 0);
    e->vks[i].x     = strtol(token[2], NULL, 0);
    e->vks[i].y     = strtol(token[3], NULL, 0);
    e->vks[i].w     = strtol(token[4], NULL, 0);
    e->vks[i].h     = strtol(token[5], NULL, 0);
  }
  
  /* OK */
  return 1;
}

/* Input Events Init */
byte aipInit(){
  /* Open Input Device Directory */
	DIR * dir = opendir(AIP_DEVICE);
	
  if(dir!=0) {
    struct dirent *de;
    int fd;
    
    /* Init Input Events Variable */
    _aip  = (AIP_VARSP) malloc(sizeof(AIP_VARS));
    if (!_aip){
      closedir(dir);
      return 0;
    }
    memset(_aip, 0, sizeof(AIP_VARS));
    _aip->touch_swap_xy=0;
    _aip->touch_flip_x=0;
    _aip->touch_flip_y=0;
    
    /* Read Input Device Directory */
    while((de = readdir(dir))) {
      /* Continue if filename not contain "event" */
      if(strncmp(de->d_name,"event",5)){
        continue;
      }
      
      /* Open File Handler */
      fd = openat(dirfd(dir), de->d_name, O_RDONLY);
      
      /* Continue if openat failed */
      if(fd < 0){
        continue;
      }
      
      /* Save fd Into Events Variable */
      _aip->fds[_aip->n].fd     = fd;
      _aip->fds[_aip->n].events = POLLIN;
      _aip->evs[_aip->n].fd_id  = _aip->n;
      
      /* Load virtualkeys if there are any */
      aipInitEventDev(&_aip->evs[_aip->n]);
      
      /* Increment the count */
      _aip->n++;
      
      /* Break when maximum device */
      if(_aip->n == AIP_MAXDEV){
        break;
      }
    }
    
    /* Close Dir */
    closedir(dir);
  }
  
  /* Success */
  return 1;
}

/* Release Events */
void aipRelease(){
  if (_aip!=NULL){
    while (_aip->n-->0){
      if (_aip->evs[_aip->n].vkn){
  		  free(_aip->evs[_aip->n].vks);
  		  _aip->evs[_aip->n].vkn = 0;
  	  }
      close(_aip->fds[_aip->n].fd);
    }
    free(_aip);
  }
}

/* Calculating Touch Screen */
static int aipCalculateTouch(AIP_POSITIONP p, int *x, int *y){
  if (p->xi.minimum == p->xi.maximum || p->yi.minimum == p->yi.maximum){
      *x = p->x;
      *y = p->y;
      return 0;
  }
  int fb_width  = 0;
  int fb_height = 0;
  if (_aip->touch_swap_xy){
    fb_width  = agh();
    fb_height = agw();
  }
  else{
    fb_width  = agw();
    fb_height = agh();
  }
  
  *x = (p->x - p->xi.minimum) * (fb_width - 1) / (p->xi.maximum - p->xi.minimum);
  *y = (p->y - p->yi.minimum) * (fb_height -1) / (p->yi.maximum - p->yi.minimum);

  if (*x >= 0 && *x < fb_width &&
      *y >= 0 && *y < fb_height)
  {
      return 0;
  }
  
  return 1;
}

/* Translate Event */
static byte aipTranslateEvent(AIP_EVP e, struct input_event * ev){
  static int downX                        = -1;
  static int downY                        = -1;
  static int discard                      = 0;
  static int lastWasSynReport             = 0;
  static int touchReleaseOnNextSynReport  = 0;
  int i;
  int x, y;
  
  /* Not Used */
  if (e->ignored){
    return 1;
  }
  
  if ((ev->type==EV_REL) && (ev->code == REL_Z)){
    /*  This appears to be an accelerometer or
        another strange input device.
        It's not the touchscreen.
     */
    e->ignored = 1;
    return AIP_TRANS_IGNONE;
  }

	/*  Handle keyboard events, value of 1
	    indicates key down, 0 indicates key up
	 */
	if (ev->type == EV_KEY) {
		return AIP_TRANS_KEY;
	}

  /* Possibly Touchscreen */
  if (ev->type == EV_ABS) {
    switch (ev->code) {
      case AIP_ABS_X:
        {
          e->p.synced |= 0x01;
          e->p.x = ev->value;
          printf("EV: %s => ABS_X  %d\n", e->device_name, ev->value);
        }
        break;

      case AIP_ABS_Y:
        {
          e->p.synced |= 0x02;
          e->p.y = ev->value;
          printf("EV: %s => ABS_Y  %d\n", e->device_name, ev->value);
        }
        break;

      case AIP_ABS_MT_POSITION:
        {
          e->mt_p.synced = 0x03;
          if (ev->value == (1 << 31)){
              e->mt_p.x = 0;
              e->mt_p.y = 0;
              lastWasSynReport = 1;
          }
          else{
              lastWasSynReport = 0;
              e->mt_p.x = (ev->value & 0x7FFF0000) >> 16;
              e->mt_p.y = (ev->value & 0xFFFF);
          }
        }
        break;

        case AIP_ABS_MT_TOUCH_MAJOR:
          {
            if (ev->value == 0)
            {
                e->mt_p.x = 0;
                e->mt_p.y = 0;
                touchReleaseOnNextSynReport = 1;
            }
          }
          break;

		    case AIP_ABS_MT_PRESSURE:
		      {
            if (ev->value == 0){
                e->mt_p.x = 0;
                e->mt_p.y = 0;
                touchReleaseOnNextSynReport = 1;
            }
          }
          break;

		    case AIP_ABS_MT_POSITION_X:
		      {
            e->mt_p.synced |= 0x01;
            e->mt_p.x = ev->value;
          }
          break;

        case AIP_ABS_MT_POSITION_Y:
          {
            e->mt_p.synced |= 0x02;
            e->mt_p.y = ev->value;
          }
          break;
        default:
          {
            return AIP_TRANS_NONE;
          }
    }

    if (ev->code != AIP_ABS_MT_POSITION){
        lastWasSynReport = 0;
        return AIP_TRANS_NONE;
    }
  }

  if (ev->code != AIP_ABS_MT_POSITION && 
      (ev->type != EV_SYN || 
        (ev->code != AIP_SYN_REPORT && ev->code != AIP_SYN_MT_REPORT)
      )
     ){
    lastWasSynReport = 0;
    return AIP_TRANS_NONE;
  }
  if (ev->code == AIP_SYN_MT_REPORT){
    return AIP_TRANS_NONE;
  }

  if (lastWasSynReport == 1 || touchReleaseOnNextSynReport == 1){
    /* Reset the value */
    touchReleaseOnNextSynReport = 0;
    
    /* We are a finger-up state */
    if (!discard){
      /* Report the key up */
      ev->type = EV_ABS;
      ev->code = 0;
      ev->value = (downX << 16) | downY;
    }
    
    downX = -1;
    downY = -1;
    if (discard){
      discard = 0;
      return AIP_TRANS_NONE;
    }
    return AIP_TRANS_TOUCH;
  }
  
  lastWasSynReport = 1;
  if (e->p.synced & 0x03){
    aipCalculateTouch(&e->p, &x, &y);
  }
  else if (e->mt_p.synced & 0x03){
    aipCalculateTouch(&e->mt_p, &x, &y);
  }
  else{
    return AIP_TRANS_NONE;
  }

  /* Swap & Flip Handler */
  if (_aip->touch_swap_xy){
    x ^= y;
    y ^= x;
    x ^= y;
  }
  if (_aip->touch_flip_x){
    x = agw() - x;
  }
  if (_aip->touch_flip_y){
    y = agh() - y;
  }
  
  /* Clear the current sync states */
  e->p.synced = e->mt_p.synced = 0;

  /* If we have nothing useful to report, skip it */
  if (x == -1 || y == -1){
    return AIP_TRANS_NONE;
  }

  /* On first touch, see if we're at a virtual key */
  if (downX == -1){
    /* Attempt mapping to virtual key */
    for (i=0; i<e->vkn; ++i){
      int xd = abs(e->vks[i].x - x);
      int yd = abs(e->vks[i].y - y);

      if (xd < e->vks[i].w/2 && yd < e->vks[i].h/2){
          ev->type = EV_KEY;
          ev->code = e->vks[i].scan;
          ev->value= 1;
          
          /* vibrate(VIBRATOR_TIME_MS); */
          
          discard = 1;
          downX = 0;
          return AIP_TRANS_KEY;
      }
    }
  }

  /* If we were originally a button press, discard this event */
  if (discard){
      return AIP_TRANS_NONE;
  }

  /* Record where we started the touch for
     deciding if this is a key or a scroll */
  downX = x;
  downY = y;
  
  ev->type = EV_ABS;
  ev->code = 1;
  ev->value = (x << 16) | y;
  return AIP_TRANS_TOUCH;
}

/* Get Input Event */
byte aipGetInput(struct input_event *ev, byte dont_wait){
  if (_aip==NULL){
    return 0;
  }
  int r;
  int n;
  do {
    r = poll(_aip->fds, _aip->n, dont_wait?0:-1);
    if(r>0){
      for(n=0;n<_aip->n;n++) {
        if(_aip->fds[n].revents & POLLIN) {
          r = read(_aip->fds[n].fd, ev, sizeof(*ev));
          if(r==sizeof(*ev)) {
            byte tret = aipTranslateEvent(&_aip->evs[n], ev);
            if ((tret==AIP_TRANS_KEY)||(tret==AIP_TRANS_TOUCH)){
              return tret;
            }
          }
        }
      }
    }
  }
  while(dont_wait==0);
  return 0;
}
