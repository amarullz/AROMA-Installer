/*
 * Copyright (C) 2011 amarullz - xda-developers
 *
 */
#include <fcntl.h>
#include <dirent.h>
#include <sys/poll.h>
#include <limits.h>
#include <linux/input.h>
#include <pthread.h>
#include <stdarg.h>
#include <sys/time.h>
#include <time.h>
#include "aroma.h"

//--- KEYS
#define MAX_DEVICES 16
#define VIBRATOR_TIMEOUT_FILE	"/sys/class/timed_output/vibrator/enable"
#define VIBRATOR_TIME_MS	50
#define PRESS_THRESHHOLD    10
#define ABS_MT_POSITION_X 0x35
#define ABS_MT_POSITION_Y 0x36
#define ABS_MT_TOUCH_MAJOR 0x30
#define SYN_MT_REPORT 2
struct virtualkey {
    int scancode;
    int centerx, centery;
    int width, height;
};
struct position {
    int x, y;
    int pressed;
    struct input_absinfo xi, yi;
};
struct ev {
    struct pollfd *fd;
    struct virtualkey *vks;
    int vk_count;
    struct position p, mt_p;
    int sent, mt_idx;
};

int  ATOUCH_TOUCHMSG= 330;
byte ATOUCH_INIT    = 0;
//330
int atouch_pressed    = 0;
int atouch_x          = -50;
int atouch_y          = -50;
int atouch_onmove     = 0;
extern int __system(const char *command);
static int ui_has_initialized = 0;
static int ui_log_stdout = 1;
static pthread_mutex_t key_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t key_queue_cond = PTHREAD_COND_INITIALIZER;
static int key_queue[256], key_queue_len = 0;
static volatile char key_pressed[KEY_MAX + 1];
int prevwasnav = 0;
dword atouch_message_value      = 0;
long atouch_last_hack = 0;
byte atouch_pressed_h = 0;

int touchX(){ return atouch_x; }
int touchY(){ return atouch_y; }
int ontouch(){ return atouch_pressed; }
int atmsg(){ return ATOUCH_TOUCHMSG; }
int atouch_wait(ATEV *atev){
  atev->x = -1;
  atev->y = -1;
  while (1){
    if (prevwasnav) ui_clear_key_queue();
    prevwasnav=0;
    int key = ui_wait_key();
    if (key==-2){
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
    if (key==ATOUCH_TOUCHMSG){
      if ((atouch_x>0)&&(atouch_y>0)){
        atev->x = atouch_x;
        atev->y = atouch_y;
        ui_clear_key_queue();
        if ((!atouch_pressed_h)&&(atouch_onmove)&&(atev->d!=0)){
          return ATEV_MOUSEMV;
        }
        else if ((!atouch_pressed_h)&&((atouch_pressed)||(ATOUCH_INIT==1)||(ATOUCH_INIT==3))){
          return ATEV_MOUSEDN;
        }
        else{
          return ATEV_MOUSEUP;
        }
      }
    }
    else if ((key!=0)&&(key==acfg()->ckey_up)) return ATEV_UP;
    else if ((key!=0)&&(key==acfg()->ckey_down)) return ATEV_DOWN;
    else if ((key!=0)&&(key==acfg()->ckey_select)) return ATEV_SELECT;
    else if ((key!=0)&&(key==acfg()->ckey_back)) return ATEV_BACK;
    else if ((key!=0)&&(key==acfg()->ckey_menu)) return ATEV_MENU;
    else{
      switch (key){
        case KEY_RIGHT: prevwasnav=1; return ATEV_RIGHT; break;
        case KEY_LEFT: prevwasnav=1; return ATEV_LEFT; break;

        case KEY_DOWN: prevwasnav=1;
        case KEY_CAPSLOCK:
        case KEY_VOLUMEDOWN:
        case 53: /* Defy */
          return ATEV_DOWN; break;
        
        case KEY_UP: prevwasnav=1;
        case KEY_LEFTSHIFT:
        case KEY_VOLUMEUP:
        case 51: /* Defy */
          return ATEV_UP; break;
        
        case 31:  /* Defy */
        case KEY_LEFTBRACE: case KEY_POWER: case KEY_HOME: case BTN_MOUSE: case KEY_ENTER:
        case KEY_CENTER: case KEY_CAMERA: case KEY_F21: case KEY_SEND:
          return ATEV_SELECT; break;
        
        case KEY_SEARCH: case 229:
        case KEY_MENU: return ATEV_MENU; break;
          
        case KEY_END: case KEY_BACKSPACE:
        case KEY_BACK: return ATEV_BACK; break;
      }
    }
  }
  return 0;
}
int touch_div_x = 8;
int touch_div_y = 8;
int touch_add_x = -20;
int touch_add_y = -20;
int ttouch_div_x = 8;
int ttouch_div_y = 8;
int ttouch_add_x = -20;
int ttouch_add_y = -20;
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
void atouch_restorecalibrate(){
  touch_div_x = ttouch_div_x;
  touch_div_y = ttouch_div_x;
  touch_add_x = ttouch_div_x;
  touch_add_y = ttouch_div_x;
}
void atouch_set_calibrate(int dx, int ax, int dy, int ay){
  touch_div_x = dx;
  touch_div_y = dy;
  touch_add_x = ax;
  touch_add_y = ay;
}
static void *atouch_init1thread(void *cookie){
  while ((ATOUCH_INIT==1)||(ATOUCH_INIT==3)){
    if (key_pressed[ATOUCH_TOUCHMSG]){
      if (atouch_last_hack<alib_tick()-6){
        atouch_pressed_h              = 1;
        key_pressed[ATOUCH_TOUCHMSG]  = 0;  
        pthread_mutex_lock(&key_queue_mutex);
        const int queue_max = sizeof(key_queue) / sizeof(key_queue[0]);
        if (key_queue_len < queue_max) {
          key_queue[key_queue_len++] = ATOUCH_TOUCHMSG;
          pthread_cond_signal(&key_queue_cond);
        }
        pthread_mutex_unlock(&key_queue_mutex);
      }
    }
    usleep(100);
  }
}
int atouch_firstevent(){
  pthread_mutex_lock(&key_queue_mutex);
  const int queue_max = sizeof(key_queue) / sizeof(key_queue[0]);
  if (key_queue_len < queue_max) {
    key_queue[key_queue_len++] = ATOUCH_TOUCHMSG;
    pthread_cond_signal(&key_queue_cond);
  }
  pthread_mutex_unlock(&key_queue_mutex);
}
int atouch_event(struct input_event *ev,int x, int y,int evget){
  if (evget==1){
    if ((x<1)&&(y<1)){
      return 0;
    }
    atouch_last_hack = alib_tick();
    if ((atouch_pressed)||(ATOUCH_INIT==1)||(ATOUCH_INIT==3)){
      int nx = max(x/touch_div_x+touch_add_x,0);
      int ny = max(y/touch_div_y+touch_add_y,0);
      
      if ((abs(nx-atouch_x)>=agdp())||(abs(ny-atouch_y)>=agdp())){
        if (atouch_x==-50)
          atouch_onmove = 0;
        else
          atouch_onmove = 1;
        
        atouch_x = nx;
        atouch_y = ny;
        ev->code = ATOUCH_TOUCHMSG;
        ev->value= atouch_onmove?2:3;
        ev->type=EV_KEY;
        if ((ATOUCH_INIT==1)||(ATOUCH_INIT==3)){
          if (atouch_pressed_h) atouch_onmove=0;
          atouch_pressed_h = 0;
          ev->value=atouch_onmove?2:1;
          key_pressed[ATOUCH_TOUCHMSG] = ev->value;
          atouch_firstevent();
          return 1;
        }
        return 1;
      }
    }
    else if ((x!=0)&&(y!=0)){
      if (ATOUCH_INIT==0){
        ATOUCH_INIT = 1;
        atouch_x         = -50;
        atouch_y         = -50;
        atouch_pressed_h = 0;
        pthread_t dirtytouch;
        pthread_create(&dirtytouch, NULL, atouch_init1thread, NULL);
        pthread_detach(dirtytouch);
        return 1;
      }
    }
  }
  else{
    atouch_pressed = ev->value;
    if (atouch_pressed==1){
      atouch_x         = -50;
      atouch_y         = -50;
    }
  }
  return 0;
}        

static struct pollfd ev_fds[MAX_DEVICES];
static struct ev evs[MAX_DEVICES];
static unsigned ev_count = 0;
static inline int ABS(int x) {
    return x<0?-x:x;
}

int vibrate(int timeout_ms)
{
    char str[20];
    int fd;
    int ret;
    fd = open(VIBRATOR_TIMEOUT_FILE, O_WRONLY);
    if (fd < 0) return -1;
    ret = snprintf(str, sizeof(str), "%d", timeout_ms);
    ret = write(fd, str, ret);
    close(fd);
    if (ret < 0)
       return -1;
    return 0;
}

/* Returns empty tokens */
static char *vk_strtok_r(char *str, const char *delim, char **save_str)
{
    if(!str) {
        if(!*save_str) return NULL;
        str = (*save_str) + 1;
    }
    *save_str = strpbrk(str, delim);
    if(*save_str) **save_str = '\0';
    return str;
}

static int vk_init(struct ev *e)
{
    char vk_path[PATH_MAX] = "/sys/board_properties/virtualkeys.";
    char vks[2048], *ts;
    ssize_t len;
    int vk_fd;
    int i;

    e->vk_count = 0;

    len = strlen(vk_path);
    len = ioctl(e->fd->fd, EVIOCGNAME(sizeof(vk_path) - len), vk_path + len);
    if (len <= 0)
        return -1;

    vk_fd = open(vk_path, O_RDONLY);
    if (vk_fd < 0)
        return -1;

    len = read(vk_fd, vks, sizeof(vks)-1);
    close(vk_fd);
    if (len <= 0)
        return -1;

    vks[len] = '\0';

    /* Parse a line like:
        keytype:keycode:centerx:centery:width:height:keytype2:keycode2:centerx2:...
    */
    for (ts = vks, e->vk_count = 1; *ts; ++ts) {
        if (*ts == ':')
            ++e->vk_count;
    }

    if (e->vk_count % 6) {
        LOGW("minui: %s is %d %% 6\n", vk_path, e->vk_count % 6);
    }
    e->vk_count /= 6;
    if (e->vk_count <= 0)
        return -1;

    e->sent = 0;
    e->mt_idx = 0;

    ioctl(e->fd->fd, EVIOCGABS(ABS_X), &e->p.xi);
    ioctl(e->fd->fd, EVIOCGABS(ABS_Y), &e->p.yi);
    e->p.pressed = 0;

    ioctl(e->fd->fd, EVIOCGABS(ABS_MT_POSITION_X), &e->mt_p.xi);
    ioctl(e->fd->fd, EVIOCGABS(ABS_MT_POSITION_Y), &e->mt_p.yi);
    e->mt_p.pressed = 0;

    e->vks = malloc(sizeof(*e->vks) * e->vk_count);

    for (i = 0; i < e->vk_count; ++i) {
        char *token[6];
        int j;

        for (j = 0; j < 6; ++j) {
            token[j] = vk_strtok_r((i||j)?NULL:vks, ":", &ts);
        }

        if (strcmp(token[0], "0x01") != 0) {
            /* Java does string compare, so we do too. */
            LOGW("minui: %s: ignoring unknown virtual key type %s\n", vk_path, token[0]);
            continue;
        }

        e->vks[i].scancode = strtol(token[1], NULL, 0);
        e->vks[i].centerx = strtol(token[2], NULL, 0);
        e->vks[i].centery = strtol(token[3], NULL, 0);
        e->vks[i].width = strtol(token[4], NULL, 0);
        e->vks[i].height = strtol(token[5], NULL, 0);
    }

    return 0;
}

int ev_init(void)
{
    DIR *dir;
    struct dirent *de;
    int fd;

    dir = opendir("/dev/input");
    if(dir != 0) {
        while((de = readdir(dir))) {
//            fprintf(stderr,"/dev/input/%s\n", de->d_name);
            if(strncmp(de->d_name,"event",5)) continue;
            fd = openat(dirfd(dir), de->d_name, O_RDONLY);
            if(fd < 0) continue;

            ev_fds[ev_count].fd = fd;
            ev_fds[ev_count].events = POLLIN;
            evs[ev_count].fd = &ev_fds[ev_count];

            /* Load virtualkeys if there are any */
            vk_init(&evs[ev_count]);

            ev_count++;
            if(ev_count == MAX_DEVICES) break;
        }
    }

    return 0;
}

void ev_exit(void)
{
    ATOUCH_INIT = 4; //-- Ensure Exit Dirty Touch Thread
    while (ev_count-- > 0) {
    	if (evs[ev_count].vk_count) {
    		free(evs[ev_count].vks);
    		evs[ev_count].vk_count = 0;
    	}
      close(ev_fds[ev_count].fd);
    }
    usleep(100);
}

static int vk_inside_display(__s32 value, struct input_absinfo *info, int screen_size)
{
    int screen_pos;
    if (info->minimum == info->maximum)
        return 0;
    screen_pos = (value - info->minimum) * (screen_size - 1) / (info->maximum - info->minimum);
    return (screen_pos >= 0 && screen_pos < screen_size);
}

static int vk_tp_to_screen(struct position *p, int *x, int *y)
{
  
  if (p->xi.minimum == p->xi.maximum || p->yi.minimum == p->yi.maximum)
        return 0;
  
  *x = (p->x - p->xi.minimum) * (agw() - 1) / (p->xi.maximum - p->xi.minimum);
  *y = (p->y - p->yi.minimum) * (agh() - 1) / (p->yi.maximum - p->yi.minimum);
  
  
  if (*x >= 0 && *x < agw() &&
         *y >= 0 && *y < agw()) {
          
      return 0;
  }
  
  
  return 1;
}

/* Translate a virtual key in to a real key event, if needed */
/* Returns non-zero when the event should be consumed */
static int vk_modify(struct ev *e, struct input_event *ev)
{
    int i;
    int x, y;
    
    if (ev->type == EV_KEY) {
        if (ev->code == BTN_TOUCH){
            e->p.pressed = ev->value;
            
        }
        return 0;
    }
    
    if (ev->type == EV_ABS) {
      
        switch (ev->code) {
          case ABS_X:
              e->p.x = ev->value;
              return !vk_inside_display(e->p.x, &e->p.xi, agw());
          case ABS_Y:
              e->p.y = ev->value;
              return !vk_inside_display(e->p.y, &e->p.yi, agh());
          case ABS_MT_POSITION_X:
              if (e->mt_idx) return 1;
              e->mt_p.x = ev->value;
              return !vk_inside_display(e->mt_p.x, &e->mt_p.xi, agw());
          case ABS_MT_POSITION_Y:
              if (e->mt_idx) return 1;
              e->mt_p.y = ev->value;
              return !vk_inside_display(e->mt_p.y, &e->mt_p.yi, agh());
          case ABS_MT_TOUCH_MAJOR:
              if (e->mt_idx) return 1;
              if (e->sent)
                  e->mt_p.pressed = (ev->value > 0);
              else
                  e->mt_p.pressed = (ev->value > PRESS_THRESHHOLD);
              return 0;
        }
        return 0;
    }
    
    if (atouch_event(ev,e->p.x,e->p.y,1)){
      return 0;
    }
    if (atouch_event(ev,e->mt_p.x,e->mt_p.y,1)){
      return 0;
    }

    if (ev->type != EV_SYN)
        return 0;

    if (ev->code == SYN_MT_REPORT) {
        /* Ignore the rest of the points */
        ++e->mt_idx;
        return 1;
    }
    if (ev->code != SYN_REPORT)
        return 0;

    /* Report complete */

    e->mt_idx = 0;

    if (!e->p.pressed && !e->mt_p.pressed) {
        /* No touch */
        e->sent = 0;
        return 0;
    }

    if (!(e->p.pressed && vk_tp_to_screen(&e->p, &x, &y)) &&
            !(e->mt_p.pressed && vk_tp_to_screen(&e->mt_p, &x, &y))) {
        
        /* No touch inside vk area */
        return 0;
    }
    
    

    if (e->sent) {
        /* We've already sent a fake key for this touch */
        return 1;
    }

    /* The screen is being touched on the vk area */
    e->sent = 1;

    for (i = 0; i < e->vk_count; ++i) {
        int xd = ABS(e->vks[i].centerx - x);
        int yd = ABS(e->vks[i].centery - y);
        if (xd < e->vks[i].width/2 && yd < e->vks[i].height/2) {
            /* Fake a key event */
            ev->type = EV_KEY;
            ev->code = e->vks[i].scancode;
            ev->value = 1;

            vibrate(VIBRATOR_TIME_MS);
            return 0;
        }
    }

    return 1;
}

int ev_get(struct input_event *ev, unsigned dont_wait)
{
    int r;
    unsigned n;

    do {
        r = poll(ev_fds, ev_count, dont_wait ? 0 : -1);

        if(r > 0) {
            for(n = 0; n < ev_count; n++) {
                if(ev_fds[n].revents & POLLIN) {
                    r = read(ev_fds[n].fd, ev, sizeof(*ev));
                    if(r == sizeof(*ev)) {
                        if (!vk_modify(&evs[n], ev))
                            return 0;
                    }
                }
            }
        }
    } while(dont_wait == 0);

    return -1;
}
// Reads input events, handles special hot keys, and adds to the key queue.
static void *input_thread(void *cookie)
{
    int rel_sum = 0;
    int fake_key = 0;
    for (;;) {
        // wait for the next key event
        struct input_event ev;
        do {
            ev_get(&ev, 0);
            if (ev.type == EV_SYN) {
                continue;
            } else if (ev.type == EV_REL) {
                if (ev.code == REL_Y) {
                    rel_sum += ev.value;
                    if (rel_sum > 6) {
                        fake_key = 1;
                        ev.type = EV_KEY;
                        ev.code = KEY_DOWN;
                        ev.value = 1;
                        rel_sum = 0;
                    } else if (rel_sum < -6) {
                        fake_key = 1;
                        ev.type = EV_KEY;
                        ev.code = KEY_UP;
                        ev.value = 1;
                        rel_sum = 0;
                    }
                }
                else if (ev.code == REL_X) {
                    rel_sum += ev.value;
                    if (rel_sum > 6) {
                        fake_key = 1;
                        ev.type = EV_KEY;
                        ev.code = KEY_RIGHT;
                        ev.value = 1;
                        rel_sum = 0;
                    } else if (rel_sum < -6) {
                        fake_key = 1;
                        ev.type = EV_KEY;
                        ev.code = KEY_LEFT;
                        ev.value = 1;
                        rel_sum = 0;
                    }
                }
            } else {
                rel_sum = 0;
            }
        } while (ev.type != EV_KEY || ev.code > KEY_MAX);
        pthread_mutex_lock(&key_queue_mutex);
        if (!fake_key) {
          if (ATOUCH_INIT==1){
            //printf("Checking\n");
            ATOUCH_INIT       = 0;
            switch (ev.code){
              case BTN_TOUCH: case BTN_STYLUS: case BTN_STYLUS2:
                ATOUCH_INIT       = 2;
                ATOUCH_TOUCHMSG   = ev.code;
                atouch_pressed    = 1;
                ev.value          = 0;
              break;
              case KEY_RIGHT: case KEY_LEFT: case KEY_DOWN: case KEY_VOLUMEDOWN: case KEY_UP: case KEY_VOLUMEUP:
              case BTN_MOUSE: case KEY_ENTER: case KEY_CENTER: case KEY_CAMERA: case KEY_F21: case KEY_SEND:
              case KEY_MENU: case KEY_HOME: case KEY_BACK: case KEY_SEARCH:
                ATOUCH_INIT       = 3;
                break;
              default:
                ATOUCH_INIT       = 2;
                ATOUCH_TOUCHMSG   = ev.code;
                atouch_pressed    = 1;
                ev.value          = 0;
              break;
            }
          }
          if ((ev.code==ATOUCH_TOUCHMSG)&&(ev.value!=2)&&(ev.value!=3)){
            atouch_event(&ev,-1,-1,0);
          }
          else if((ev.value==3)&&(ev.code==ATOUCH_TOUCHMSG))
            ev.value=1;
          key_pressed[ev.code] = ev.value;
        }
        fake_key = 0;
        const int queue_max = sizeof(key_queue) / sizeof(key_queue[0]);
        if (ev.value >= 0 && key_queue_len < queue_max) {
            key_queue[key_queue_len++] = ev.code;
            pthread_cond_signal(&key_queue_cond);
        }
        pthread_mutex_unlock(&key_queue_mutex);
    }
    return NULL;
}

void ui_init(void)
{
    ui_has_initialized = 1;
    ev_init();
    pthread_t t;
    pthread_create(&t, NULL, input_thread, NULL);
}
int ui_wait_key()
{
    pthread_mutex_lock(&key_queue_mutex);
    while (key_queue_len == 0) {
        pthread_cond_wait(&key_queue_cond, &key_queue_mutex);
    }
    int key = key_queue[0];
    memcpy(&key_queue[0], &key_queue[1], sizeof(int) * --key_queue_len);
    pthread_mutex_unlock(&key_queue_mutex);
    
    return key;
}
int ui_key_pressed(int key)
{
    // This is a volatile static array, don't bother locking
    return key_pressed[key];
}
void set_key_pressed(int key,char val){
  key_pressed[key]=val;
}
void ui_clear_key_queue() {
    pthread_mutex_lock(&key_queue_mutex);
    key_queue_len = 0;
    pthread_mutex_unlock(&key_queue_mutex);
}
void atouch_send_message(dword msg){
  pthread_mutex_lock(&key_queue_mutex);
  const int queue_max = sizeof(key_queue) / sizeof(key_queue[0]);
  if (key_queue_len < queue_max) {
    atouch_message_value = msg;
    key_queue[key_queue_len++] = -2;
    pthread_cond_signal(&key_queue_cond);
  }
  pthread_mutex_unlock(&key_queue_mutex);
}

//-- 