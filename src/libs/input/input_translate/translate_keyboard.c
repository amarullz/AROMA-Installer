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
 * UNIVERSAL DEVICE - TRANSLATOR FOR KEY DEVICE
 *   Prefix : INDR_
 *
 */
#ifndef __AROMA_CORE_UNIVERSAL_INPUT_DRIVER_TRANSLATE_KEY__
#define __AROMA_CORE_UNIVERSAL_INPUT_DRIVER_TRANSLATE_KEY__
/*
 * Function : Translate RAW Keyboard data
 *
 */
byte INDR_translate_keyboard(AINPUTP me, INDR_DEVICEP dev,
                             AINPUT_EVENTP dest_ev, struct input_event * ev) {
  /* DUMP RAW EVENTS */
  //ALOGRT("INDR RAW KEY: T=%i, C=%i, V=%i",ev->type,ev->code,ev->value);
  if (ev->type == EV_KEY) {
    /* Fill Destination Event */
    dest_ev->type   = AINPUT_EV_TYPE_KEY;
    dest_ev->key    = ev->code;
    dest_ev->x      = 0;
    dest_ev->y      = 0;
    /* Check State */
    dest_ev->state      = AINPUT_EV_STATE_CANCEL;
    
    switch (ev->value) {
      case 0:
        dest_ev->state  = AINPUT_EV_STATE_UP;
        break;
        
      case 1:
        dest_ev->state  = AINPUT_EV_STATE_DOWN;
        break;
    }
    
    /* Translate Key Code to AROMA-CORE Return Code */
    switch (ev->code) {
        /* Select Key */
      case KEY_LEFTBRACE:
      case KEY_POWER:
      case KEY_HOME:
      case BTN_MOUSE:
      case KEY_ENTER:
      case KEY_CAMERA:
      case KEY_F21:
      case KEY_SEND:
      case KEY_END:
      case 0xE8:
        return AINPUT_EV_RET_SELECT;
        break;
        
        /* Menu Key */
      case KEY_SEARCH:
      case KEY_MENU:
      case 0xE5:
        return AINPUT_EV_RET_MENU;
        break;
        
        /* Back Key */
      case KEY_BACKSPACE:
      case KEY_BACK:
        return AINPUT_EV_RET_BACK;
        break;
        
        /* Up Key */
      case KEY_UP:
      case KEY_LEFTSHIFT:
      case KEY_VOLUMEUP:
      case KEY_LEFT:
        return AINPUT_EV_RET_UP;
        break;
        
        /* Down Key */
      case KEY_DOWN:
      case KEY_CAPSLOCK:
      case KEY_VOLUMEDOWN:
      case KEY_RIGHT:
        return AINPUT_EV_RET_DOWN;
        break;
    }
    
    /* Process as Raw Key Code */
    return AINPUT_EV_RET_RAWKEY;
  }
  
  /* Don't Process It */
  return AINPUT_EV_RET_NONE;
}
#endif // __AROMA_CORE_UNIVERSAL_INPUT_DRIVER_TRANSLATE_KEY__
