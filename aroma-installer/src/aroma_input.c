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
static  int       evtouch_rx      = 0;  //-- RAW X
static  int       evtouch_ry      = 0;  //-- RAW Y
static  int       evtouch_sx      = 0;  //-- Saved X
static  int       evtouch_sy      = 0;  //-- Saved Y
static  int       evtouch_x       = 0;  //-- Translated X (Ready to use)
static  int       evtouch_y       = 0;  //-- Translated Y (Ready to use)
static  int       evtouch_code    = 888;//-- Touch Virtual Code

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
float touch_div_x =7.75; float touch_div_y =7.75; int touch_add_x =50; int touch_add_y =50; //-- Used
float ttouch_div_x=7.75; float ttouch_div_y=7.75; int ttouch_add_x=50; int ttouch_add_y=50; //-- Temporary

//-- NON TRANSLATED CALIBRATING
void atouch_plaincalibrate(){
  ttouch_div_x = touch_div_x;
  ttouch_div_y = touch_div_x;
  ttouch_add_x = touch_div_x;
  ttouch_add_y = touch_div_x;
  touch_div_x = 1;
  touch_div_y = 1;
  touch_add_x = 0;
  touch_add_y = 0;
}

//-- RESTORE CALIBRATION DATA
void atouch_restorecalibrate(){
  touch_div_x = ttouch_div_x;
  touch_div_y = ttouch_div_x;
  touch_add_x = ttouch_div_x;
  touch_add_y = ttouch_div_x;
}

//-- SET CALIBRATION DATA
void atouch_set_calibrate(float dx, int ax, float dy, int ay){
  touch_div_x = (float) dx;
  touch_div_y = (float) dy;
  touch_add_x = ax;
  touch_add_y = ay;
  
  ttouch_div_x = touch_div_x;
  ttouch_div_y = touch_div_x;
  ttouch_add_x = touch_div_x;
  ttouch_add_y = touch_div_x;
}

//-- TRANSLATE RAW COORDINATE INTO TRANSLATED COORDINATE
void atouch_translate_raw(){
  evtouch_x = max(round(((float) evtouch_rx)/touch_div_x)-touch_add_x,0);
  evtouch_y = max(round(((float) evtouch_ry)/touch_div_y)-touch_add_y,0);
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

//-- INPUT CALLBACK
void ev_input_callback(int fd, short revents){
  if (revents&POLLIN) {
    struct input_event ev;
    int         r = read(fd, &ev, sizeof(ev));
    if (r == sizeof(ev)){
      //-- OK ITS READY FOR HANDLING
      
      switch (ev.type){
        //-- Real Key Input Event
        case EV_KEY:{
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
              //ev_post_message(KEY_DOWN,1);
              ev_post_message(KEY_DOWN,0);
              evrel_size=0;
            }
            else if (evrel_size<-8) {
              //-- UP
              //ev_post_message(KEY_UP,1);
              ev_post_message(KEY_UP,0);
              evrel_size=0;
            }
          }
          else if (ev.code == REL_X) {
            evrel_size += ev.value;
            if (evrel_size>8) {
              //-- RIGHT
              //ev_post_message(KEY_RIGHT,1);
              ev_post_message(KEY_RIGHT,0);
              evrel_size=0;
            }
            else if (evrel_size<-8) {
              //-- LEFT
              //ev_post_message(KEY_LEFT,1);
              ev_post_message(KEY_LEFT,0);
              evrel_size=0;
            }
          }
        }
        break;
        
        //-- Touch Input Event
        case EV_ABS:{
          if (ev.code==ABS_MT_TOUCH_MAJOR){
            byte tmptouch = (ev.value>0)?((evtouch_state==0)?1:2):((evtouch_state==0)?3:0);
            
            if (tmptouch!=3){
              atouch_translate_raw(); //-- Translate RAW
              
              //-- TOUCH UP
              if (tmptouch==1){
                evtouch_state = 1;
                evtouch_sx = evtouch_x;
                evtouch_sy = evtouch_y;
                ev_post_message(evtouch_code,1);
              }
              //-- TOUCH MOVE
              else if (tmptouch==2){
                //-- SNAP TOUCH MOVE
                if ((abs(evtouch_sx-evtouch_x)>=agdp())||(abs(evtouch_sy-evtouch_y)>=agdp())){
                  //-- IT MOVE MORE THAN DEVICE PIXELATE
                  evtouch_state = 2;
                  evtouch_sx = evtouch_x;
                  evtouch_sy = evtouch_y;
                  ev_post_message(evtouch_code,2);
                }
              }
              //-- TOUCH UP
              else{
                evtouch_state = 0;
                evtouch_sx = 0;
                evtouch_sy = 0;
                ev_post_message(evtouch_code,0);
              }
            }
          }
          else if ((ev.code==ABS_MT_POSITION_X)||(ev.code==ABS_X)){
            //-- GOT RAW TOUCH X COORDINATE
            if (ev.value>0) evtouch_rx = ev.value;
          }
          else if ((ev.code==ABS_MT_POSITION_Y)||(ev.code==ABS_Y)){
            //-- GOT RAW TOUCH Y COORDINATE
            if (ev.value>0) evtouch_ry = ev.value;
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
      if (!test_bit(EV_ABS, ev_bits) && !test_bit(EV_KEY, ev_bits) && !test_bit(EV_REL, ev_bits)) {
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
  return 0;
}

//-- RELEASE INPUT DEVICE
void ev_exit(void){
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
              return ATEV_SELECT;
            }
            else if (evtouch_x<(capiative_btnsz*2)){
              atev->k = KEY_MENU;
              return ATEV_MENU;
            }
            else if (evtouch_x<(capiative_btnsz*3)){
              atev->k = KEY_BACK;
              return ATEV_BACK;
            }
            else if (evtouch_x<(capiative_btnsz*4)){
              atev->k = KEY_SEARCH;
              return ATEV_MENU;
            }
          }
          // home,menu,back,search
        }
      }
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