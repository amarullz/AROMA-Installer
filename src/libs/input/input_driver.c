/*
 * Copyright (C) 2011-2013 Ahmad Amarullah ( http://amarullz.com/ )
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
 * UNIVERSAL DEVICE - INPUT DRIVER
 *   Using Linux Input Device for Android
 *   Prefix : INDR_
 *
 */
#ifndef __AROMA_CORE_UNIVERSAL_INPUT_DRIVER__
#define __AROMA_CORE_UNIVERSAL_INPUT_DRIVER__

/*
 * Headers Includes
 *
 */
#include <linux/input.h>

/*
 * Defines & Macros
 *
 */
#define INDR_DEVPATH                  "/dev/input"
#define INDR_MAXDEV                   0xf
#define INDR_SIZEOF_BIT_ARRAY(bits)   ((bits + 7) / 8)
#define INDR_TEST_BIT(bit, array)     (array[bit/8] & (1<<(bit%8)))

/*
 * Enum : Device Type
 *
 */
enum {
  INDR_DEVCLASS_KEYBOARD    = 0x01,
  INDR_DEVCLASS_TOUCH       = 0x02,
  INDR_DEVCLASS_MULTITOUCH  = 0x04
};

/*
 * Enum : Position State
 *
 */
enum {
  INDR_POS_ST_SYNC_X    = 0x01,
  INDR_POS_ST_SYNC_Y    = 0x02,
  INDR_POS_ST_DOWNED    = 0x04,
  INDR_POS_ST_LASTSYNC  = 0x08,
  INDR_POS_ST_RLS_NEXT  = 0x10,
  INDR_POS_ST_IS_VKEY   = 0x20
};

/*
 * Structure : Position
 *
 */
typedef struct {
  int     x;                /* Last Raw X Event */
  int     y;                /* Last Raw Y Event */
  int     tx;               /* Translated X Position */
  int     ty;               /* Translated Y Position */
  int     vk;               /* Virtual Key Code ID */
  byte    state;            /* State */
  struct  input_absinfo xi; /* Calibrate X */
  struct  input_absinfo yi; /* Calibrate Y */
} INDR_POS, *INDR_POSP;

/*
 * Structure : Virtual Keys Data
 *
 */
typedef struct {
  int scan;                 /* Scan Code */
  int x;                    /* x */
  int y;                    /* y */
  int w;                    /* width */
  int h;                    /* height */
} INDR_VK, *INDR_VKP;

/*
 * Structure : Device Data
 *
 */
typedef struct {
  /* Device Into */
  int       id;              /* device id */
  byte      devclass;        /* Device Class */
  char      file[10];        /* Input Device Filename */
  char      name[64];        /* device name */
  byte      down;            /* pressed */
  int       vkn;             /* Virtual Key Count */
  INDR_VKP  vks;             /* Virtual Keys */
  INDR_POS  p;               /* ABS Position */
} INDR_DEVICE, *INDR_DEVICEP;

/*
 * Structure : Internal Driver Data
 *
 */
typedef struct {
  int           n;                /* Devices Count */
  struct pollfd fds[INDR_MAXDEV]; /* Devices Pool FD */
  INDR_DEVICE   dev[INDR_MAXDEV]; /* Devices Data */
  
  /* Configurations */
  byte          touch_swap_xy;    /* Swap X with Y */
  byte          touch_flip_x;     /* X was Flipped */
  byte          touch_flip_y;     /* Y was Flipped */
} INDR_INTERNAL,
*INDR_INTERNALP;

/*
 * Include Input Translator
 *
 */
#include "input_translate/translate_keyboard.c" /* Keyboard */
#include "input_translate/translate_touch.c" /* Touch */

/*
 * Forward Functions
 *
 */
void INDR_release(AINPUTP me);
byte INDR_getinput(AINPUTP me, AINPUT_EVENTP dest_ev);
byte INDR_init_device(INDR_INTERNALP mi, int fd, INDR_DEVICEP dev);

/*
 * Function : Check Blacklisted Devices
 *
 */
byte INDR_blacklist(char * name) {
  /* Not Blacklisted */
  return 0;
}

/*
 * Function : Dump Device Info
 *
 */
void INDR_dumpdev(INDR_DEVICEP dev) {
  /* Print Logs */
  LOGS("INDR Input Device: %s (%s) - Class : %x\n", dev->name, dev->file, dev->devclass);
  LOGS("  VKN : %d, CALIB : (%d,%d,%d,%d)\n", dev->vkn,
       dev->p.xi.minimum, dev->p.xi.maximum, dev->p.yi.minimum, dev->p.yi.maximum
      );
}

/*
 * Function : Init Input Device
 *
 */
byte INDR_init(AINPUTP me) {
  /* Allocating Internal Data */
  INDR_INTERNALP mi = (INDR_INTERNALP) malloc(sizeof(INDR_INTERNAL));
  /* Cleanup */
  memset(mi, 0, sizeof(INDR_INTERNAL));
  /* Set Internal Address */
  me->internal = (voidp) mi;
  /* Set Initial Value */
  mi->n = 0;
  /* Open Input Device Directory */
  DIR * dir = opendir(INDR_DEVPATH);
  
  if (dir != 0) {
    struct dirent * de; /* DIRENT */
    int             fd; /* Temporary Device FD */
    
    /* Read Input Device Directory */
    while ((de = readdir(dir))) {
      /* Continue if filename not contain "event" */
      if (strncmp(de->d_name, "event", 5)) {
        continue;
      }
      
      /* Open File Handler */
      fd = openat(dirfd(dir), de->d_name, O_RDONLY);
      
      /* Continue if openat failed */
      if (fd < 0) {
        continue;
      }
      
      /* Cleanup Device Data */
      memset(&mi->dev[mi->n], 0, sizeof(INDR_DEVICE));
      /* Set Device ID */
      mi->dev[mi->n].id     = mi->n;
      /* Set Device Filename */
      snprintf(mi->dev[mi->n].file, 10, "%s", de->d_name);
      
      /* Load virtualkeys if there are any */
      if (INDR_init_device(mi, fd, &mi->dev[mi->n])) {
        /* Dump Device Information */
        INDR_dumpdev(&mi->dev[mi->n]);
        /* Set Pooling Data and Monitor it */
        mi->fds[mi->n].fd     = fd;
        mi->fds[mi->n].events = POLLIN;
        /* Increment the polling count */
        mi->n++;
      }
      else {
        /* Dump Device Information */
        INDR_dumpdev(&mi->dev[mi->n]);
        /* Cleanup Device Data */
        memset(&mi->dev[mi->n], 0, sizeof(INDR_DEVICE));
        /* Don't Monitor This Device */
        close(fd);
        /* Ignore It */
      }
      
      /* Break when maximum device */
      if (mi->n == INDR_MAXDEV) {
        break;
      }
    }
    
    /* Close Dir */
    closedir(dir);
    
    /* Input Device Not Found */
    if (mi->n == 0) {
      /* Free Internal Data */
      free(mi);
      LOGE("INDR ERROR: Input Device Not Found...\n");
      /* Error */
      return 0;
    }
    
    /* Set Driver Callbacks */
    me->cb_release    = &INDR_release;
    me->cb_getinput   = &INDR_getinput;
    /* ok */
    return 1;
  }
  
  /* Free Internal Data */
  free(mi);
  LOGE("INDR ERROR: Can't access /dev/input...\n");
  /* Error */
  return 0;
}

/*
 * Function : Release Input Driver Instance
 *
 */
void INDR_release(AINPUTP me) {
  /* Is Input Instance Initialized ? */
  if (me == NULL) {
    return;
  }
  
  /* Get Internal Data */
  INDR_INTERNALP mi = (INDR_INTERNALP)
                      me->internal;
                      
  /* Release Devices Data */
  while (mi->n-- > 0) {
    /* Release Virtual Keys */
    if (mi->dev[mi->n].vkn) {
      free(mi->dev[mi->n].vks);
      mi->dev[mi->n].vkn = 0;
    }
    
    /* Close FD */
    close(mi->fds[mi->n].fd);
  }
  
  /* Free Internal Data */
  free(me->internal);
  me->internal = NULL;
}

/*
 * Function : Returns empty tokens
 *
 */
static char * INDR_strtok_r(
  char * str, const char * delim, char ** save_str) {
  if (!str) {
    if (!*save_str) {
      return NULL;
    }
    
    str = (*save_str) + 1;
  }
  
  *save_str = strpbrk(str, delim);
  
  if (*save_str) {
    **save_str = '\0';
  }
  
  return str;
}


/*
 * Function : Check Non Zero
 *
 */
static byte INDR_nonzero(const bytep array_s,
                         dword startIndex, dword endIndex) {
  const bytep end   = array_s + endIndex;
  bytep array = array_s + startIndex;
  
  while (array != end) {
    if (*(array++) != 0) {
      return 1;
    }
  }
  
  return 0;
}

/*
 * Function : Get Device Type
 *
 */
byte INDR_getdevclass(int fd) {
  /* Figure out the kinds of events the device reports. */
  byte keyBitmask[(KEY_MAX + 1) / 8];
  byte absBitmask[(ABS_MAX + 1) / 8];
  ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keyBitmask)), keyBitmask);
  ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(absBitmask)), absBitmask);
  /* Reset Return Value */
  byte ret = 0;
  /* Check Keyboard */
  byte haveKeyboardKeys =
    INDR_nonzero(keyBitmask, 0,
                 INDR_SIZEOF_BIT_ARRAY(BTN_MISC)) ||
    INDR_nonzero(keyBitmask,
                 INDR_SIZEOF_BIT_ARRAY(KEY_OK),
                 INDR_SIZEOF_BIT_ARRAY(KEY_MAX + 1));
  /* Check Gamepad */
  byte haveGamepadButtons =
    INDR_nonzero(keyBitmask,
                 INDR_SIZEOF_BIT_ARRAY(BTN_MISC),
                 INDR_SIZEOF_BIT_ARRAY(BTN_MOUSE)) ||
    INDR_nonzero(keyBitmask,
                 INDR_SIZEOF_BIT_ARRAY(BTN_JOYSTICK),
                 INDR_SIZEOF_BIT_ARRAY(BTN_DIGI));
                 
  if (haveKeyboardKeys) {
    ret |= INDR_DEVCLASS_KEYBOARD;
  }
  
  /* Check Touch Screen */
  if (INDR_TEST_BIT(ABS_MT_POSITION_X, absBitmask) &&
      INDR_TEST_BIT(ABS_MT_POSITION_Y, absBitmask)) {
    /* Multitouch */
    if (INDR_TEST_BIT(BTN_TOUCH, keyBitmask) ||
        !haveGamepadButtons) {
      ret |= INDR_DEVCLASS_TOUCH;
      ret |= INDR_DEVCLASS_MULTITOUCH;
    }
  }
  else if (INDR_TEST_BIT(BTN_TOUCH, keyBitmask) &&
           INDR_TEST_BIT(ABS_X, absBitmask) &&
           INDR_TEST_BIT(ABS_Y, absBitmask)) {
    /* Single Touch */
    ret |= INDR_DEVCLASS_TOUCH;
  }
  
  return ret;
}

/*
 * Function : Init Device
 *
 */
byte INDR_init_device(INDR_INTERNALP mi, int fd, INDR_DEVICEP dev) {
  /* Virtual Key Path */
  char    vk_path[PATH_MAX] = "/sys/board_properties/virtualkeys.";
  char  * ts = NULL;
  char    vks[2048];
  /* Get Device Name */
  ssize_t len = ioctl(fd, EVIOCGNAME(sizeof(dev->name)), dev->name);
  
  if (len <= 0) {
    LOGW("INDR ERROR: EVIOCGNAME for %d\n", dev->id);
    return 0;
  }
  
  /* Blacklisted Devices */
  if (INDR_blacklist(dev->name)) {
    return 0;
  }
  
  /* Get device class */
  dev->devclass = INDR_getdevclass(fd);
  
  /* If Class is none, Ignore it */
  if (!dev->devclass) {
    return 0;
  }
  
  /* Reset All Values */
  memset(&dev->p, 0, sizeof(INDR_POS));
  dev->p.tx       = -1;
  dev->p.ty       = -1;
  dev->p.vk       = -1;
  dev->vkn        = 0;
  dev->down       = 0;
  
  /* If Touchscreen, Get Calibration data & VirtualKeys */
  if ((dev->devclass & INDR_DEVCLASS_TOUCH)) {
    /* Calibration */
    if (dev->devclass & INDR_DEVCLASS_MULTITOUCH) {
      /* Get Multitouch Calibrations Data */
      ioctl(fd, EVIOCGABS(ABS_MT_POSITION_X), &dev->p.xi);
      ioctl(fd, EVIOCGABS(ABS_MT_POSITION_Y), &dev->p.yi);
    }
    else {
      /* Get Singletouch Calibrations Data */
      ioctl(fd, EVIOCGABS(ABS_X), &dev->p.xi);
      ioctl(fd, EVIOCGABS(ABS_Y), &dev->p.yi);
    }
    
    /* virtualkeys.{device_name} */
    strcat(vk_path, dev->name);
    /* Some devices split the keys from the touchscreen */
    int vk_fd = open(vk_path, O_RDONLY);
    
    if (vk_fd >= 0) {
      /* Read Contents */
      len = read(vk_fd, vks, sizeof(vks) - 1);
      close(vk_fd);
      
      /* Return False on Failed */
      if (len > 0) {
        /* Add string break */
        vks[len] = 0;
        
        /* Parse a line like:
         * keytype:keycode:centerx:centery:width:height:keytype2:keycode2:centerx2:...
         */
        for (ts = vks, dev->vkn = 1; *ts; ++ts) {
          if (*ts == ':') {
            dev->vkn++;
          }
        }
        
        dev->vkn /= 6;
        
        if (dev->vkn <= 0) {
          dev->vkn = 0;
        }
      }
    }
    
    /* Allocate Virtualkeys Count */
    if (dev->vkn > 0) {
      dev->vks = malloc(sizeof(INDR_VK) * dev->vkn);
      int i;
      
      for (i = 0; i < dev->vkn; i++) {
        char * token[6];
        int j;
        
        for (j = 0; j < 6; j++) {
          token[j] = INDR_strtok_r((i || j) ? NULL : vks, ":", &ts);
        }
        
        if (strcmp(token[0], "0x01") != 0) {
          continue;
        }
        
        /* Save It */
        dev->vks[i].scan  = strtol(token[1], NULL, 0);
        dev->vks[i].x     = strtol(token[2], NULL, 0);
        dev->vks[i].y     = strtol(token[3], NULL, 0);
        dev->vks[i].w     = strtol(token[4], NULL, 0);
        dev->vks[i].h     = strtol(token[5], NULL, 0);
        LOGS("  VIRTUALKEY[%s,%i] (%i,%i,%i,%i,%i)]\n",
             dev->file,
             i,
             dev->vks[i].scan,
             dev->vks[i].x,
             dev->vks[i].y,
             dev->vks[i].w,
             dev->vks[i].h
            );
      }
    }
  }
  
  /* OK */
  return 1;
}

/*
 * Function : Translate RAW data
 *
 */
byte INDR_translate(AINPUTP me, INDR_DEVICEP dev,
                    AINPUT_EVENTP dest_ev, struct input_event * ev) {
  if (dev->devclass & INDR_DEVCLASS_TOUCH) {
    /* It's Touch Device - input_translate/translate_touch.c */
    return INDR_translate_touch(me, dev, dest_ev, ev);
  }
  else if (dev->devclass & INDR_DEVCLASS_KEYBOARD) {
    /* It's Key Device - input_translate/translate_key.c */
    return INDR_translate_keyboard(me, dev, dest_ev, ev);
  }
  
  /* Don't Process It */
  return AINPUT_EV_RET_NONE;
}

/*
 * Function : Get Input
 *
 */
byte INDR_getinput(AINPUTP me, AINPUT_EVENTP dest_ev) {
  /* Get Internal Data */
  INDR_INTERNALP mi = (INDR_INTERNALP)
                      me->internal;
                      
  /* Polling Loop */
  do {
    int r = poll(mi->fds, mi->n, -1);
    
    if (me->internal == NULL) {
      /* If Released */
      break;
    }
    else if (r > 0) {
      /* Events Loop */
      int n;
      
      for (n = 0; n < mi->n; n++) {
        if (mi->fds[n].revents & POLLIN) {
          /* Read Data */
          struct input_event ev;
          r = read(mi->fds[n].fd, &ev, sizeof(ev));
          
          if (r == sizeof(ev)) {
            /* Translate It */
            byte translate_ret = INDR_translate(me, &mi->dev[n], dest_ev, &ev);
            
            /* Check */
            if (translate_ret != AINPUT_EV_RET_NONE) {
              /* Don't Process It */
              return translate_ret;
            }
          }
        }
      }
    }
  }
  while (me->internal != NULL);
  
  /* It was exit message */
  LOGV("INDR_getinput Input Driver Already Released\n");
  return AINPUT_EV_RET_EXIT;
}

/*
 * Function : AROMA CORE Init Driver Wrapper
 *
 */
byte __universal_input_driver_init(AINPUTP me) {
  return INDR_init(me);
}
#endif // __AROMA_CORE_UNIVERSAL_INPUT_DRIVER__
