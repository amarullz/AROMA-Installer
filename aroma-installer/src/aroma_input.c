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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/poll.h>
#include <linux/input.h>
#include <pthread.h>
#include "aroma.h"

//-- DEFINED
#define MAX_DEVICES 16
#define MAX_MISC_FDS 16
#define BITS_PER_LONG (sizeof(unsigned long) * 8)
#define BITS_TO_LONGS(x) (((x) + BITS_PER_LONG - 1) / BITS_PER_LONG)
#define test_bit(bit, array) ((array)[(bit)/BITS_PER_LONG] & (1 << ((bit) % BITS_PER_LONG)))

//-- GLOBAL EVENT VARIABLE
static  struct    pollfd ev_fds[MAX_DEVICES + MAX_MISC_FDS];
static  unsigned  ev_count        = 0;
static  unsigned  ev_dev_count    = 0;
static  unsigned  ev_misc_count   = 0;
static  char      key_pressed[KEY_MAX + 1];

//-- AROMA EVENT DATA
static  byte      evthread_active = 1;
static  byte      evtouch_state   = 0;  //-- 0: Up, 1: Down, 2: Move
static  byte      evtouch_alreadyu= 1;  //-- Already UP
static  int       evtouch_rx      = 0;  //-- RAW X
static  int       evtouch_ry      = 0;  //-- RAW Y
static  int       evtouch_sx      = 0;  //-- Saved X
static  int       evtouch_sy      = 0;  //-- Saved Y
static  int       evtouch_x       = 0;  //-- Translated X (Ready to use)
static  int       evtouch_y       = 0;  //-- Translated Y (Ready to use)
static  int       evtouch_code    = 888;//-- Touch Virtual Code
static  int       evtouch_tx      = 0;  //-- Temporary Translated X
static  int       evtouch_ty      = 0;  //-- Temporary Translated Y
static  byte      evtouch_locked  = 0;

//-- AROMA RELATIVE EVENT DATA
static  int       evrel_key       = -1;
static  int       evrel_val       = 0;
static  int       evrel_size      = 0;

//-- AROMA CUSTOM MESSAGE
static  dword     atouch_message_value      = 0;
static  int       atouch_message_code       = 889;

//-- KEY QUEUE
static  int       key_queue[256];
static  int       key_queue_len = 0;
static pthread_mutex_t key_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t key_queue_cond = PTHREAD_COND_INITIALIZER;

//-- PASS TOUCH STATE FUNCTIONS
int touchX()  { return evtouch_x; }
int touchY()  { return evtouch_y; }
int ontouch() { return ((evtouch_state==0)?0:1); }

//-- VIBRATE FUNCTION
int vibrate(int timeout_ms){
    char str[20];
    int fd;
    int ret;
    fd = open("/sys/class/timed_output/vibrator/enable", O_WRONLY);
    if (fd < 0) return -1;
    ret = snprintf(str, sizeof(str), "%d", timeout_ms);
    ret = write(fd, str, ret);
    close(fd);
    if (ret < 0)
       return -1;
    return 0;
}

//-- KEYPRESS MANAGER
int ui_key_pressed(int key){
    return key_pressed[key];
}
void set_key_pressed(int key,char val){
  key_pressed[key]=val;
}
int atmsg(){ return evtouch_code; }

//-- TOUCH CALIBRATION
float touch_div_x =7.90; float touch_div_y =7.90; int touch_add_x =20; int touch_add_y =20; //-- Used
float ttouch_div_x=7.90; float ttouch_div_y=7.90; int ttouch_add_x=20; int ttouch_add_y=20; //-- Temporary

//-- NON TRANSLATED CALIBRATING
void atouch_plaincalibrate(){
  ttouch_div_x = touch_div_x;
  ttouch_div_y = touch_div_y;
  ttouch_add_x = touch_add_x;
  ttouch_add_y = touch_add_y;
  touch_div_x = 1;
  touch_div_y = 1;
  touch_add_x = 0;
  touch_add_y = 0;
}

//-- RESTORE CALIBRATION DATA
void atouch_restorecalibrate(){
  touch_div_x = ttouch_div_x;
  touch_div_y = ttouch_div_y;
  touch_add_x = ttouch_add_x;
  touch_add_y = ttouch_add_y;
}

//-- SET CALIBRATION DATA
void atouch_set_calibrate(float dx, int ax, float dy, int ay){
  touch_div_x = dx;
  touch_div_y = dy;
  touch_add_x = ax;
  touch_add_y = ay;
  
  ttouch_div_x = touch_div_x;
  ttouch_div_y = touch_div_y;
  ttouch_add_x = touch_add_x;
  ttouch_add_y = touch_add_y;
}

//-- TRANSLATE RAW COORDINATE INTO TRANSLATED COORDINATE
void atouch_translate_raw(){
  evtouch_tx = max(round(((float) evtouch_rx)/touch_div_x)-touch_add_x,0);
  evtouch_ty = max(round(((float) evtouch_ry)/touch_div_y)-touch_add_y,0);
}

//-- INPUT EVENT POST MESSAGE
void ev_post_message(int key, int value){
  set_key_pressed(key,value);
  pthread_mutex_lock(&key_queue_mutex);
  const int queue_max = sizeof(key_queue) / sizeof(key_queue[0]);
  if (key_queue_len<queue_max){
    key_queue[key_queue_len++] = key;
    pthread_cond_signal(&key_queue_cond);
  }
  pthread_mutex_unlock(&key_queue_mutex);
}

//-- TOUCH HACK
static  byte      evtouch_thack   = 0;
static  long      evtouch_lastick = 0;
static void *ev_input_thack(void *cookie){
  while(evtouch_thack){
    if (evtouch_state!=0){
      if (evtouch_lastick<alib_tick()-10){
        evtouch_locked  = 1;
        evtouch_alreadyu= 1;
        evtouch_state   = 0;
        evtouch_sx      = 0;
        evtouch_sy      = 0;
        evtouch_rx      = 0;
        evtouch_ry      = 0;
        ev_post_message(evtouch_code,0);
      }
    }
    usleep(20);
  }
}
byte atouch_gethack(){
  return evtouch_thack;
}
void atouch_sethack(byte t){
  if (t!=evtouch_thack){
    if (t){
      evtouch_lastick = alib_tick();
      evtouch_thack   = t;
      pthread_t hack_thread_t;
      pthread_create(&hack_thread_t, NULL, ev_input_thack, NULL);
      pthread_detach(hack_thread_t);
    }
  }
  evtouch_thack=t;
}

//-- INPUT CALLBACK
byte evtouch_mt_syn = 0;
void ev_input_callback(int fd, short revents){
  if (revents&POLLIN) {
    struct input_event ev;
    int         r = read(fd, &ev, sizeof(ev));
    if (r == sizeof(ev)){
      //-- OK ITS READY FOR HANDLING
      
      switch (ev.type){
        //-- Real Key Input Event
        case EV_KEY:{
          if ((ev.code==330)&&(evtouch_alreadyu==0)&&(ev.value==0)){
            if (!evtouch_thack){
              evtouch_alreadyu=1;
              evtouch_locked=1;
              evtouch_state=0;
              evtouch_sx = 0;
              evtouch_sy = 0;
              evtouch_rx = 0;
              evtouch_ry = 0;
              evtouch_mt_syn=0;
              ev_post_message(evtouch_code,0);
            }
          }
          else
            ev_post_message(ev.code,ev.value);
        }
        break;
        
        //-- Relative Input Event
        case EV_REL:{
          if (evrel_key!=ev.code){
            evrel_key = ev.code;
            evrel_size= 0;
          }
          int evt=((ev.value<0)?-1:1);
          if (evrel_val!=evt){
            evrel_val = evt;
            evrel_size= 0;
          }
          if (ev.code==REL_Y) {
            evrel_size += ev.value;
            if (evrel_size>8) {
              //-- DOWN
              ev_post_message(KEY_DOWN,0);
              evrel_size=0;
            }
            else if (evrel_size<-8) {
              //-- UP
              ev_post_message(KEY_UP,0);
              evrel_size=0;
            }
          }
          else if (ev.code == REL_X) {
            evrel_size += ev.value;
            if (evrel_size>8) {
              //-- RIGHT
              ev_post_message(KEY_RIGHT,0);
              evrel_size=0;
            }
            else if (evrel_size<-8) {
              //-- LEFT
              ev_post_message(KEY_LEFT,0);
              evrel_size=0;
            }
          }
        }
        break;
        
        case EV_SYN:{
          if (ev.code==SYN_MT_REPORT){
            if (evtouch_state>0){
              if (evtouch_mt_syn==2){
                evtouch_mt_syn=1;
              }
              else if(evtouch_mt_syn==1){
                evtouch_mt_syn=0;
                if (evtouch_alreadyu==0){
                  if (!evtouch_thack){
                    evtouch_locked=1;
                    evtouch_alreadyu=1;
                    evtouch_state = 0;
                    evtouch_sx = 0;
                    evtouch_sy = 0;
                    evtouch_rx = 0;
                    evtouch_ry = 0;
                    ev_post_message(evtouch_code,0);
                  }
                }
              }
            }
            else if (evtouch_state==0){
              if (evtouch_mt_syn==3){
                evtouch_mt_syn=1;
                atouch_translate_raw();
                evtouch_locked=1;
                evtouch_alreadyu=0;
                evtouch_x=evtouch_tx;
                evtouch_y=evtouch_ty;
                evtouch_state = 1;
                evtouch_sx = evtouch_x;
                evtouch_sy = evtouch_y;
                ev_post_message(evtouch_code,1);
              }
            }
          }
        }break;
        
        //-- Touch Input Event
        case EV_ABS:{
          evtouch_lastick = alib_tick();
          
          if (ev.code==ABS_MT_TOUCH_MAJOR){
            evtouch_mt_syn = 2;
            if ((evtouch_rx>0)&&(evtouch_ry>0)){              
              byte tmptouch  = (ev.value>0)?((evtouch_state==0)?1:2):((evtouch_state==0)?3:0);              
              if (tmptouch!=3){
                atouch_translate_raw(); //-- Translate RAW
                //-- TOUCH DOWN
                if (tmptouch==1){
                  evtouch_locked=1;
                  evtouch_alreadyu=0;
                  evtouch_x=evtouch_tx;
                  evtouch_y=evtouch_ty;
                  evtouch_state = 1;
                  evtouch_sx = evtouch_x;
                  evtouch_sy = evtouch_y;
                  ev_post_message(evtouch_code,1);
                }
                //-- TOUCH MOVE
                else if ((tmptouch==2)&&(evtouch_alreadyu==0)){
                  int agdp2=agdp()*2;
                  //-- SNAP TOUCH MOVE
                  if ((abs(evtouch_sx-evtouch_tx)>=agdp2)||(abs(evtouch_sy-evtouch_ty)>=agdp2)){
                    //-- IT MOVE MORE THAN DEVICE PIXELATE
                    evtouch_locked=1;
                    evtouch_x=evtouch_tx;
                    evtouch_y=evtouch_ty;
                    evtouch_state = 2;
                    evtouch_sx = evtouch_x;
                    evtouch_sy = evtouch_y;
                    ev_post_message(evtouch_code,2);
                    
                    //evtouch_thack
                  }
                }
                //-- TOUCH UP
                else if ((tmptouch==0)&&(evtouch_alreadyu==0)){
                  if (!evtouch_thack){
                    evtouch_locked=1;
                    evtouch_alreadyu=1;
                    evtouch_state = 0;
                    evtouch_sx = 0;
                    evtouch_sy = 0;
                    evtouch_rx = 0;
                    evtouch_ry = 0;
                    evtouch_mt_syn=0;
                    ev_post_message(evtouch_code,0);
                  }
                }
              }
            }
            else{
              byte tmptouch  = (ev.value>0)?((evtouch_state==0)?1:2):((evtouch_state==0)?3:0);
              if ((tmptouch!=0)&&(tmptouch!=3)){
                evtouch_mt_syn=3;
                evtouch_locked=0;
              }
            }
          }
          else if ((ev.code==ABS_MT_POSITION_X)||(ev.code==ABS_X)){
            //-- GOT RAW TOUCH X COORDINATE
            if (!evtouch_locked){
              if (ev.value>0) evtouch_rx = ev.value;
            }
          }
          else if ((ev.code==ABS_MT_POSITION_Y)||(ev.code==ABS_Y)){
            //-- GOT RAW TOUCH Y COORDINATE
            if (!evtouch_locked){
              if (ev.value>0) evtouch_ry = ev.value;
            }
          }
        }
        break;
      }
    }
  }
}

//-- INPUT THREAD
static void *ev_input_thread(void *cookie){
  //-- Loop for Input
  while (evthread_active){
    //-- Wait For Input Event
    int r = poll(ev_fds, ev_count, -1);
    if (r >= 0){
      //-- New Key Event
      unsigned n;
      for (n=0; n<ev_count; n++) {
          if (ev_fds[n].revents & ev_fds[n].events)
            ev_input_callback(ev_fds[n].fd, ev_fds[n].revents);
      }
    }
  }
}

//-- INIT INPUT DEVICE
void ui_init(){
  ev_init();
}
int ev_init(){
  DIR *dir;
  struct dirent *de;
  int fd;
  dir = opendir("/dev/input");
  if(dir != 0) {
    while((de = readdir(dir))) {
      unsigned long ev_bits[BITS_TO_LONGS(EV_MAX)];
      if(strncmp(de->d_name,"event",5)) continue;
      fd = openat(dirfd(dir), de->d_name, O_RDONLY);
      if(fd < 0) continue;
      
      /* read the evbits of the input device */
      if (ioctl(fd, EVIOCGBIT(0, sizeof(ev_bits)), ev_bits) < 0) {
          close(fd);
          continue;
      }

      /* ABS, KEY & REL */
      if (!test_bit(EV_ABS, ev_bits) && !test_bit(EV_SYN, ev_bits) && !test_bit(EV_KEY, ev_bits) && !test_bit(EV_REL, ev_bits)) {
          close(fd);
          continue;
      }
      
      ev_fds[ev_count].fd       = fd;
      ev_fds[ev_count].events   = POLLIN;
      ev_count++;
      ev_dev_count++;
      if(ev_dev_count == MAX_DEVICES) break;
    }
  }
  
  //-- Create Watcher Thread
  evthread_active = 1;
  pthread_t input_thread_t;
  pthread_create(&input_thread_t, NULL, ev_input_thread, NULL);
  pthread_detach(input_thread_t);
  
  // atouch_sethack(1);
  return 0;
}

//-- RELEASE INPUT DEVICE
void ev_exit(void){
  evtouch_thack   = 0;
  evthread_active = 0;
  while (ev_count > 0) {
      close(ev_fds[--ev_count].fd);
  }
  ev_misc_count = 0;
  ev_dev_count = 0;
}

//-- SEND ATOUCH CUSTOM MESSAGE
void atouch_send_message(dword msg){
  atouch_message_value = msg;
  ev_post_message(atouch_message_code,0);
}

//-- Clear Queue
void ui_clear_key_queue() {
  pthread_mutex_lock(&key_queue_mutex);
  evtouch_locked=0;
  key_queue_len = 0;
  pthread_mutex_unlock(&key_queue_mutex);
}

//-- Wait For Key
int ui_wait_key(){
  pthread_mutex_lock(&key_queue_mutex);
  while (key_queue_len == 0){
    pthread_cond_wait(&key_queue_cond, &key_queue_mutex);
  }
  int key = key_queue[0];
  memcpy(&key_queue[0], &key_queue[1], sizeof(int) * --key_queue_len);
  pthread_mutex_unlock(&key_queue_mutex);
  return key;
}

//-- AROMA Input Handler
int atouch_wait(ATEV *atev){
  return atouch_wait_ex(atev,0);
}
int atouch_wait_ex(ATEV *atev, byte calibratingtouch){
  atev->x = -1;
  atev->y = -1;
  // if (prev_was_key) ui_clear_key_queue();
  while (1){
    int key = ui_wait_key();
    
    //-- Custom Message
    if (key==atouch_message_code){
      atev->msg = atouch_message_value;
      atev->d   = 0;
      atev->x   = 0;
      atev->y   = 0;
      atev->k   = 0;
      ui_clear_key_queue();
      atouch_message_value = 0;
      return ATEV_MESSAGE;
    }
    
    atev->d = ui_key_pressed(key);
    atev->k = key;
    
    if (key==evtouch_code){
      if ((evtouch_x>0)&&(evtouch_y>0)){
        //-- GENERIC TOUCH SCREEN INPUT EVENT
        if (((evtouch_x<=agw())&&(evtouch_y<=agh()))||(calibratingtouch)){
          atev->x = evtouch_x;
          atev->y = evtouch_y;
          evtouch_locked=0;
          switch(evtouch_state){
            case 1:  return ATEV_MOUSEDN; break;
            case 2:  return ATEV_MOUSEMV; break;
            default: return ATEV_MOUSEUP; break;
          }
        }
        //-- CAPIATIVE KEY INPUT EVENT
        else if(evtouch_y>(agh()+(agdp()*10))){
          int capiative_btnsz = agw()/4;
          if (evtouch_state==0){
            atev->d = 0;
            if (evtouch_x<capiative_btnsz){
              atev->k = KEY_HOME;
              evtouch_locked=0;
              return ATEV_SELECT;
            }
            else if (evtouch_x<(capiative_btnsz*2)){
              atev->k = KEY_MENU;
              evtouch_locked=0;
              return ATEV_MENU;
            }
            else if (evtouch_x<(capiative_btnsz*3)){
              atev->k = KEY_BACK;
              evtouch_locked=0;
              return ATEV_BACK;
            }
            else if (evtouch_x<(capiative_btnsz*4)){
              atev->k = KEY_SEARCH;
              evtouch_locked=0;
              return ATEV_MENU;
            }
          }
          // home,menu,back,search
        }
      }
      evtouch_locked=0;
    }
    else if ((key!=0)&&(key==acfg()->ckey_up))      return ATEV_UP;
    else if ((key!=0)&&(key==acfg()->ckey_down))    return ATEV_DOWN;
    else if ((key!=0)&&(key==acfg()->ckey_select))  return ATEV_SELECT;
    else if ((key!=0)&&(key==acfg()->ckey_back))    return ATEV_BACK;
    else if ((key!=0)&&(key==acfg()->ckey_menu))    return ATEV_MENU;
    else{
      /* DEFINED KEYS */
      switch (key){
        /* RIGHT */
        case KEY_RIGHT: return ATEV_RIGHT; break;
        /* LEFT */
        case KEY_LEFT:  return ATEV_LEFT; break;
        
        /* DOWN */
        case KEY_DOWN:
        case KEY_CAPSLOCK:
        case KEY_VOLUMEDOWN:
          return ATEV_DOWN; break;
        
        /* UP */
        case KEY_UP:
        case KEY_LEFTSHIFT:
        case KEY_VOLUMEUP:
          return ATEV_UP; break;
        
        /* SELECT */
        case KEY_LEFTBRACE:
        case KEY_POWER:
        case KEY_HOME:
        case BTN_MOUSE:
        case KEY_ENTER:
        case KEY_CENTER:
        case KEY_CAMERA:
        case KEY_F21:
        case KEY_SEND:
          return ATEV_SELECT; break;
        
        /* SHOW MENU */
        case KEY_SEARCH:
        case 229:
        case KEY_MENU:
          return ATEV_MENU; break;
        
        /* BACK */
        case KEY_END:
        case KEY_BACKSPACE:
        case KEY_BACK:
          return ATEV_BACK; break;
      }
    }
  }
  return 0;
}
//-- 