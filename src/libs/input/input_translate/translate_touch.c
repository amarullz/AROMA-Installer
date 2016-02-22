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
 * UNIVERSAL DEVICE - TRANSLATOR FOR TOUCH DEVICE
 *   Prefix : INDR_
 *
 */
#ifndef __AROMA_CORE_UNIVERSAL_INPUT_DRIVER_TRANSLATE_TOUCH__
#define __AROMA_CORE_UNIVERSAL_INPUT_DRIVER_TRANSLATE_TOUCH__

/*
 * Defines
 *
 */
#define ABS_MT_POSITION   0x2a

/*
 * Function : Calculate touch event with calibration data
 *
 */
static byte INDR_calibrate(AINPUTP me, INDR_POSP p, int * x, int * y) {
  /* No Need Calculating Calibration */
  if (p->xi.minimum == p->xi.maximum || p->yi.minimum == p->yi.maximum) {
    *x = p->x;
    *y = p->y;
    return 1;
  }
  
  /* Get Internal Data */
  INDR_INTERNALP mi = (INDR_INTERNALP)
                      me->internal;
  /* Screen */
  int fb_width  = me->screen_width;
  int fb_height = me->screen_height;
  
  if (mi->touch_swap_xy) {
    fb_width  = me->screen_height;
    fb_height = me->screen_width;
  }
  
  /* Calculation */
  *x = (p->x - p->xi.minimum) * (fb_width - 1) / (p->xi.maximum - p->xi.minimum);
  *y = (p->y - p->yi.minimum) * (fb_height - 1) / (p->yi.maximum - p->yi.minimum);
  
  /* Check Result */
  if (*x >= 0 && *x < fb_width &&
      *y >= 0 && *y < fb_height) {
    return 1;
  }
  
  /* Not OK */
  return 0;
}

/*
 * Function : Translate RAW Multitouch Touch Device
 *
 */
byte INDR_translate_touch(AINPUTP me, INDR_DEVICEP dev,
                          AINPUT_EVENTP dest_ev, struct input_event * ev) {
  /* Get Internal Data */
  INDR_INTERNALP mi = (INDR_INTERNALP)
                      me->internal;
  /* DUMP RAW EVENTS */
  //ALOGRT("INDR RAW TOUCH: T=%i, C=%i, V=%i",ev->type,ev->code,ev->value);
  static int MT_TRACKING_IS_UNTOUCHED = 0;
  static int TOUCH_RELEASE_NEXTSYN = 0;
  
  /* Process EV_ABS Event */
  if (ev->type == EV_ABS) {
    switch (ev->code) {
      case ABS_X:
        /* X Only Event */
        dev->p.state    |= INDR_POS_ST_SYNC_X;
        dev->p.x         = ev->value;
        break;
        
      case ABS_Y:
        /* Y Only Event */
        dev->p.state    |= INDR_POS_ST_SYNC_Y;
        dev->p.y         = ev->value;
        break;
        
      case ABS_MT_POSITION_X:
        /* Multitouch X Only Event */
        dev->p.state    |= INDR_POS_ST_SYNC_X;
        dev->p.x         = ev->value;
        break;
        
      case ABS_MT_POSITION_Y:
        /* Multitouch Y Only Event */
        dev->p.state    |= INDR_POS_ST_SYNC_Y;
        dev->p.y         = ev->value;
        break;
        
      case ABS_MT_POSITION:
        /* Multitouch XY Event */
        dev->p.state    |= INDR_POS_ST_SYNC_X | INDR_POS_ST_SYNC_Y;
        if (dev->p.x != 0 && dev->p.y != 0) {
          if (ev->value == (1 << 31)) {
            dev->p.state  |= INDR_POS_ST_LASTSYNC;
            dev->p.x = 0;
            dev->p.y = 0;
          }
          else {
            dev->p.state  &= ~INDR_POS_ST_LASTSYNC;
            dev->p.x = (ev->value & 0x7FFF0000) >> 16;
            dev->p.y = (ev->value & 0xFFFF);
          }
        
          ev->type = EV_SYN;
          ev->code = SYN_REPORT;
        }
        break;
        
      case ABS_MT_TOUCH_MAJOR:
      case ABS_MT_PRESSURE:
      
        /* Multitouch Pressure Event */
        if (ev->value == 0) {
          /* Screen UnTouched */
          dev->p.state |= INDR_POS_ST_RLS_NEXT;
          dev->p.x = 0;
          dev->p.y = 0;
        }
        
        break;
        
      case ABS_MT_TRACKING_ID:
        if (ev->value < 0) {
          /* Screen UnTouched */
          dev->p.state |= INDR_POS_ST_RLS_NEXT;
          dev->p.x = 0;
          dev->p.y = 0;
          TOUCH_RELEASE_NEXTSYN = 1;
          MT_TRACKING_IS_UNTOUCHED = 1;
        }
        
        break;
        
      default:
        /* Unknown Event */
        goto return_none;
    }
  }
  
  /* Process EV_SYN Event */
  if (ev->type == EV_SYN) {
    if (ev->code == SYN_MT_REPORT) {
      /* Return on SYN_MT_REPORT */
      goto return_none;
    }
    else if (ev->code != SYN_REPORT) {
      /* Return and clear syn on non SYN_REPORT */
      goto return_clear_sync;
    }
    
    if (((dev->p.state & (INDR_POS_ST_LASTSYNC | INDR_POS_ST_RLS_NEXT)) && !MT_TRACKING_IS_UNTOUCHED) ||
        (MT_TRACKING_IS_UNTOUCHED && TOUCH_RELEASE_NEXTSYN == 1)) {
      /* Set Destination Coordinate */
      TOUCH_RELEASE_NEXTSYN = 0;
      dest_ev->x      = dev->p.tx;
      dest_ev->y      = dev->p.ty;
      
      /* Sometime It Reported Twice, So Check This Value */
      if ((dev->p.tx == -1) || (dev->p.tx == -1)) {
        /* LOG RAW */
        //ALOGRT("INDR Got Double EV_SYN-UP Event. Ignore It.");
        dev->p.state    &= ~INDR_POS_ST_DOWNED;
        dev->p.state    &= ~INDR_POS_ST_RLS_NEXT;
        goto return_clear_sync;
      }
      
      /* Reset Translated Coordinate */
      dev->p.tx       = -1;
      dev->p.ty       = -1;
      /* Remove Down Flag */
      dev->p.state    &= ~INDR_POS_ST_DOWNED;
      /* Reset Release Next */
      dev->p.state    &= ~INDR_POS_ST_RLS_NEXT;
      
      /* It was touch up event if not on virtualkey */
      if (!(dev->p.state & INDR_POS_ST_IS_VKEY)) {
        /* Fill Destination Event */
        dest_ev->type   = AINPUT_EV_TYPE_TOUCH;
        dest_ev->state  = AINPUT_EV_STATE_UP;
        dest_ev->key    = 0;
      }
      else {
        /* Reset Virtual Key Flag */
        dev->p.state    &= ~INDR_POS_ST_IS_VKEY;
        /* Set Custom Key Event */
        struct input_event key_ev;
        key_ev.type = EV_KEY;
        key_ev.code = dev->vks[dev->p.vk].scan;
        /* State Was Cancel by default */
        key_ev.value = 3;
        /* Check If Still Touch Inside Virtual Key */
        int xd = abs(dev->vks[dev->p.vk].x - dest_ev->x);
        int yd = abs(dev->vks[dev->p.vk].y - dest_ev->y);
        
        if ((xd < dev->vks[dev->p.vk].w / 2) && (yd < dev->vks[dev->p.vk].h / 2)) {
          /* It Still On Virtual Key. Set As UP */
          key_ev.value = 0;
          /* LOG RAW */
          //ALOGRT("INDR VIRTUALKEY UP : [%i,%i] on %ix%ipx\n",dev->p.vk,key_ev.code,xd,yd);
        }
        else {
          /* LOG RAW */
          //ALOGRT("INDR VIRTUALKEY CANCEL : [%i,%i] on %ix%ipx\n",dev->p.vk,key_ev.code,xd,yd);
        }
        
        /* Reset Virtual Key ID */
        dev->p.vk    = -1;
        /* If on Virtual Key - Send as keyboard event */
        return INDR_translate_keyboard(me, dev, dest_ev, &key_ev);
      }
      
      return AINPUT_EV_RET_TOUCH;
    }
    
    /* Set on EV_SYN */
    dev->p.state  |= INDR_POS_ST_LASTSYNC;
    /* Calibrated X, Y */
    int cx = -1;
    int cy = -1;
    
    /* Check if X and Y has been synced */
    if ((dev->p.state & INDR_POS_ST_SYNC_X) && (dev->p.state & INDR_POS_ST_SYNC_Y)) {
      if (!INDR_calibrate(me, &dev->p, &cx, &cy)) {
        goto return_none;
      }
    }
    else {
      /* If Error */
      goto return_none;
    }
    
    /* Swap & Flip Handler */
    if (mi->touch_swap_xy) {
      cx ^= cy;
      cy ^= cx;
      cx ^= cy;
    }
    
    if (mi->touch_flip_x) {
      cx = me->screen_width - cx;
    }
    
    if (mi->touch_flip_y) {
      cy = me->screen_height - cy;
    }
    
    /* If we have nothing useful to report, skip it */
    if (cx == -1 || cy == -1) {
      goto return_none;
    }
    
    /* Reset Last Sync XY Event */
    dev->p.state    &= ~INDR_POS_ST_SYNC_X;
    dev->p.state    &= ~INDR_POS_ST_SYNC_Y;
    
    /* On first touch */
    if (!(dev->p.state & INDR_POS_ST_DOWNED)) {
      /* Set Downed */
      dev->p.state |= INDR_POS_ST_DOWNED;
      /* See if we're at a virtual key,
       * Attempt mapping to virtual key
       */
      int i;
      
      for (i = 0; i < dev->vkn; i++) {
        int xd = abs(dev->vks[i].x - cx);
        int yd = abs(dev->vks[i].y - cy);
        
        if ((xd < dev->vks[i].w / 2) && (yd < dev->vks[i].h / 2)) {
          /* Set as virtual key */
          dev->p.state |= INDR_POS_ST_IS_VKEY;
          /* Set Virtual Key ID */
          dev->p.vk    = i;
          /* Set Translated Coordinat */
          dev->p.tx    = cx;
          dev->p.ty    = cy;
          /* Set as Custom Key Event */
          struct input_event key_ev;
          key_ev.type = EV_KEY;
          key_ev.code = dev->vks[i].scan;
          /* Key Event State = Down */
          key_ev.value = 1;
          /* LOG RAW */
          //ALOGRT("INDR VIRTUALKEY DOWN : [%i,%i] on %ix%ipx\n",i,key_ev.code,xd,yd);
          /* If on Virtual Key - Send as keyboard event */
          return INDR_translate_keyboard(me, dev, dest_ev, &key_ev);
        }
      }
      
      /* Set destination state as down event */
      dest_ev->state  = AINPUT_EV_STATE_DOWN;
    }
    /* On Touch Move */
    else {
      /* If it was virtual key, ignore it */
      if ((dev->p.state & INDR_POS_ST_IS_VKEY)) {
        /* Set Translated Coordinat
         * Needed for cancel virtual key event
         */
        dev->p.tx    = cx;
        dev->p.ty    = cy;
        /* Don't Send Any Event */
        goto return_none;
      }
      
      /* Set destination state as move event */
      dest_ev->state  = AINPUT_EV_STATE_MOVE;
    }
    
    /* Set Translated Coordinat */
    dev->p.tx    = cx;
    dev->p.ty    = cy;
    /* Fill Destination Event */
    dest_ev->type   = AINPUT_EV_TYPE_TOUCH;
    dest_ev->key    = 0;
    dest_ev->x      = dev->p.tx;
    dest_ev->y      = dev->p.ty;
    /* Set as Touch Event */
    return AINPUT_EV_RET_TOUCH;
  }
  
return_clear_sync:
  /* Reset Last Sync Event */
  dev->p.state    &= ~INDR_POS_ST_LASTSYNC;
return_none:
  return AINPUT_EV_RET_NONE;
}

#endif // __AROMA_CORE_UNIVERSAL_INPUT_DRIVER_TRANSLATE_TOUCH__
