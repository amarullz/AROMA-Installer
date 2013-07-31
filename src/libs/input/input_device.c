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
 * Input Event Hook and Manager
 *
 */
#define ALOGE(...) LOGE(__VA_ARGS__); fprintf(stdout, "\n");
#define ALOGI(...) LOGS(__VA_ARGS__); fprintf(stdout, "\n");
#define ALOGS(...) LOGS(__VA_ARGS__); fprintf(stdout, "\n");
#define ALOGW(...) LOGW(__VA_ARGS__);
#define ALOGV(...) LOGV(__VA_ARGS__);
#define ALOGT(...) LOGD(__VA_ARGS__);
#define ALOGRT(...) LOGD(__VA_ARGS__);

/* Touch Key Code */
#define AINPUT_KEYCODE_MAX      0x200
#define AINPUT_TOUCH_KEYCODE    (AINPUT_KEYCODE_MAX-1)

/* Pointer Integers */
typedef byte * bytep;
typedef word * wordp;
typedef dword * dwordp;
typedef void * voidp;

/* AROMA Core Input Instance */
typedef struct _AINPUT_STRUCT         AINPUT;
typedef struct _AINPUT_STRUCT    *    AINPUTP;
typedef struct _AINPUT_EVENT_STRUCT   AINPUT_EVENT;
typedef struct _AINPUT_EVENT_STRUCT * AINPUT_EVENTP;

/* AROMA Core Input Driver Dynamic Functions Structure */
typedef void  (*AINPUT_CB_RELEASE)    (AINPUTP);
typedef byte  (*AINPUT_CB_GETINPUT)   (AINPUTP, AINPUT_EVENTP);
typedef byte  (*AINPUT_CB_CONFIG)     (AINPUTP, const char *, const char *, dword);

/* AROMA Core Input Instance Struct */
struct _AINPUT_STRUCT {
  /* Driver Specific Data */
  voidp internal;                       /* Internal Data for Input Driver */
  
  /* Driver Callbacks */
  AINPUT_CB_RELEASE    cb_release;      /* Driver Release */
  AINPUT_CB_GETINPUT   cb_getinput;     /* Get Input Event - return AINPUT_EV_RET_ */
  AINPUT_CB_CONFIG     cb_config;       /* Runtime Configuration */
  
  /* AROMA CORE Runtime Data */
  int   screen_width;                   /* Screen Width */
  int   screen_height;                  /* Screen Height */
  byte  key_pressed[AINPUT_KEYCODE_MAX / 8];
  
  /* Touch Move Informations */
  // long  touch_last_tick;
  int   touch_last_x;
  int   touch_last_y;
};

/* AROMA Core Input Event Struct */
struct _AINPUT_EVENT_STRUCT {
  byte  type;                           /* Look at AINPUT_EV_TYPE_ */
  int   key;                            /* Key Code */
  byte  state;                          /* Look at AINPUT_EV_STATE_ */
  int   x;                              /* Touch x coordinate */
  int   y;                              /* Touch y coordinate */
};

/* Input Event Type */
#define AINPUT_EV_TYPE_NONE     0x00      /* Won't processed */
#define AINPUT_EV_TYPE_KEY      0x01      /* Keypad/Keyboards */
#define AINPUT_EV_TYPE_TOUCH    0x02      /* Touch Screen */

/* Get Input Return Values */
#define AINPUT_EV_RET_NONE      0x00      /* Ignore  */
#define AINPUT_EV_RET_SELECT    0x01      /* Enter, Home Button, Select, Power, etc */
#define AINPUT_EV_RET_MENU      0x02      /* Menu or you can also use Search Button */
#define AINPUT_EV_RET_BACK      0x03      /* Back Button */
#define AINPUT_EV_RET_UP        0x04      /* Move Up Button - Volume Up, etc */
#define AINPUT_EV_RET_DOWN      0x05      /* Move Down Button - Volume Down, etc */
#define AINPUT_EV_RET_TOUCH     0x06      /* Touch Screen Event */
#define AINPUT_EV_RET_RAWKEY    0x07      /* AINPUT_EVENTP->key is Raw Key Code */
#define AINPUT_EV_RET_EXIT      0xcc      /* Exit Event / Halt Event Handler */
#define AINPUT_EV_RET_ERROR     0xdd      /* Contain Fatal Error */

/* Input Event State */
#define AINPUT_EV_STATE_UP      0x00      /* Key/Touch Up */
#define AINPUT_EV_STATE_DOWN    0x01      /* Key/Touch Down */
#define AINPUT_EV_STATE_MOVE    0x02      /* Touch Move Coordinate */
#define AINPUT_EV_STATE_CANCEL  0x00      /* Key Up But Canceled / Dont Processed */


#include "input_driver.c"

AINPUTP _aip = NULL;

/* Input Events Init */
byte aipInit() {
  /* Check Instance */
  if (_aip != NULL) {
    LOGE("Input Handler Instance Already Initialized");
    goto return_error;
  }
  
  /* Allocating Input Instance */
  _aip = (AINPUTP) malloc(sizeof(AINPUT));
  /* Cleanup */
  memset(_aip, 0, sizeof(AINPUT));
  /* Set Screen Information */
  _aip->screen_width    = agw();
  _aip->screen_height   = agh();
  // _aip->touch_last_tick = aTick();
  _aip->touch_last_x    = 0;
  _aip->touch_last_y    = 0;
  /* Init Driver */
  LOGV("Initializing Input Driver");
  
  if (!INDR_init(_aip)) {
    LOGE("Input Driver Init Function ERROR!");
    goto return_error_clean;
  }
  
  /* Check Callbacks */
  if ((_aip->cb_release == NULL) || (_aip->cb_getinput == NULL)) {
    LOGE("Some Input Callback is NULL");
    goto return_error_clean;
  }
  
  /* OK */
  LOGV("Input Handler was Initialized");
  goto return_ok;
return_error_clean:
  free(_aip);
  _aip = NULL;
return_error:
  return 0;
return_ok:
  return 1;
}

/* Release Events */
void aipRelease() {
  /* Check Instance */
  if (_aip == NULL) {
    LOGE("Input Handler Instance not initialized Yet!");
    return;
  }
  
  /* Release Driver */
  LOGS("Releasing Input Driver");
  _aip->cb_release(_aip);
  /* Free Instance */
  LOGV("Releasing Input Handler Instance");
  free(_aip);
}

/* Set Key Pressed */
byte aipSetKeyPress(int code, byte state) {
  if (code <= AINPUT_KEYCODE_MAX) {
    byte bit_pos   = 1 << (code % 8);
    
    switch (state) {
      case AINPUT_EV_STATE_DOWN:
        _aip->key_pressed[code >> 3] |= bit_pos;
        break;
        
      case AINPUT_EV_STATE_UP:
        //case AINPUT_EV_STATE_CANCEL:
        _aip->key_pressed[code >> 3] &= ~bit_pos;
        break;
    }
    
    return 1;
  }
  
  return 0;
}

/* Get Key Pressed */
byte aipGetKeyPressed(int code) {
  if (code <= AINPUT_KEYCODE_MAX) {
    byte bit_pos   = 1 << (code % 8);
    
    if ((_aip->key_pressed[code >> 3]&bit_pos)) {
      return 1;
    }
    
    return 0;
  }
  
  return 0;
}

/* Get Touch Pressed */
byte aipGetTouchPressed() {
  return aipGetKeyPressed(AINPUT_TOUCH_KEYCODE);
}

/* Get Input Event */
byte aipGetInput(AINPUT_EVENTP e) {
  /* Clean destination variable */
  memset(e, 0, sizeof(AINPUT_EVENT));
  
  /* Check Instance */
  if (_aip == NULL) {
    /* Log Verbose */
    LOGW("aipGetInput Input Handler Instance may already released");
    return AINPUT_EV_RET_ERROR;
  }
  
  /* Loop Until Event Type != AINPUT_EV_TYPE_NONE and _aip!=NULL */
  while (_aip != NULL) {
    /* Call Driver getinput callback */
    byte ret = _aip->cb_getinput(_aip, e);
    
    /* Check Return Value */
    switch (ret) {
      case AINPUT_EV_RET_NONE:
        /* Continue */
        break;
        
      case AINPUT_EV_RET_EXIT:
        /* Clean destination variable */
        memset(e, 0, sizeof(AINPUT_EVENT));
        LOGV("aipGetInput got AINPUT_EV_TYPE_EXIT Signal");
        return ret;
        break;
        
      case AINPUT_EV_RET_ERROR:
        /* Clean destination variable */
        memset(e, 0, sizeof(AINPUT_EVENT));
        LOGE("aipGetInput got AINPUT_EV_RET_ERROR Signal");
        return ret;
        break;
        
      case AINPUT_EV_RET_TOUCH: {
          /* Filter Move Event to Prevent Flooding Move Messages */
          if (e->state == AINPUT_EV_STATE_MOVE) {
            /* 16ms Wait - ignore the floods
            int difx = abs(_aip->touch_last_x - e->x);
            int dify = abs(_aip->touch_last_y - e->y);
            
            if ((difx + dify) >= agdp()) {
              if (_aip->touch_last_tick < aTick() - 2) {
            */
            aipSetKeyPress(AINPUT_TOUCH_KEYCODE, e->state);
            /* Set Last Move Info */
            _aip->touch_last_x = e->x;
            _aip->touch_last_x = e->y;
            // _aip->touch_last_tick = aTick();
            return ret;
            /*
              }
            }*/
            /* Continue */
          }
          else {
            aipSetKeyPress(AINPUT_TOUCH_KEYCODE, e->state);
            /* Set Last Move Info */
            _aip->touch_last_x = e->x;
            _aip->touch_last_x = e->y;
            // _aip->touch_last_tick = aTick();
            return ret;
          }
        }
        break;
        
      default:
        /* Send Value */
        aipSetKeyPress(e->key, e->state);
        return ret;
        break;
    }
    
    /* Clean destination variable */
    memset(e, 0, sizeof(AINPUT_EVENT));
  }
  
  return AINPUT_EV_RET_EXIT;
}
