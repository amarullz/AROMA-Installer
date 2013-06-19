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
 * Installer Proccess
 *
 */

#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include "../aroma.h"

static byte      ai_run              = 0;
static int       ai_progani_pos      = 0;
static float     ai_progress_pos     = 0;
static float     ai_progress_fract   = 0;
static int       ai_progress_fract_n = 0;
static int       ai_progress_fract_c = 0;
static long      ai_progress_fract_l = 0;
static int       ai_progress_w     = 0;
static int       ai_prog_x         = 0;
static int       ai_prog_y         = 0;
static int       ai_prog_w         = 0;
static int       ai_prog_h         = 0;
static int       ai_prog_r         = 0;
static int       ai_prog_ox        = 0;
static int       ai_prog_oy        = 0;
static int       ai_prog_ow        = 0;
static int       ai_prog_oh        = 0;
static int       ai_prog_or        = 0;
static CANVAS  * ai_bg             = NULL;
static CANVAS  * ai_cv             = NULL;
static char      ai_progress_text[64];
static char      ai_progress_info[101];
static AWINDOWP  ai_win;
static ACONTROLP ai_buftxt;
static int       ai_return_status  = 0;

void ai_rebuildtxt(int cx, int cy, int cw, int ch) {
  char * buffer = NULL;
  struct stat st;
  
  if (stat(AROMA_INSTALL_TXT, &st) < 0) {
    return;
  }
  
  buffer = malloc(st.st_size + 1);
  
  if (buffer == NULL) {
    goto done;
  }
  
  FILE * f = fopen(AROMA_INSTALL_TXT, "rb");
  
  if (f == NULL) {
    goto done;
  }
  
  if (fread(buffer, 1, st.st_size, f) != st.st_size) {
    fclose(f);
    goto done;
  }
  
  buffer[st.st_size] = '\0';
  fclose(f);
done:
  actext_rebuild(
    ai_buftxt,
    cx, cy, cw, ch,
    ((buffer != NULL) ? buffer : ""),
    0, 1);
  free(buffer);
}
char * ai_fixlen(char * str, char * addstr) {
  int maxw = ai_prog_w - (ai_prog_or * 2) - ag_txtwidth(addstr, 0);
  int clen = ag_txtwidth(str, 0);
  
  if (clen < maxw) {
    return NULL;
  }
  
  int basepos = 0;
  int i = 0;
  char basestr[64];
  char allstr[128];
  memset(basestr, 0, 64);
  
  for (i = strlen(str) - 1; i >= 0; i--) {
    if (str[i] == '/') {
      basepos = i - 2;
      snprintf(basestr, 63, "%s", &(str[i]));
      
      if (i > 0) {
        snprintf(allstr, 127, "/%c%c..%s", str[1], str[2], basestr);
      }
      else {
        snprintf(allstr, 127, "%s", basestr);
      }
      
      break;
    }
  }
  
  if (basepos > 50) {
    basepos = 50;
  }
  
  do {
    if (basepos <= 0) {
      break;
    }
    
    char dirstr[64];
    memset(dirstr, 0, 64);
    memcpy(dirstr, str, basepos);
    snprintf(allstr, 127, "%s..%s", dirstr, basestr);
    clen = ag_txtwidth(allstr, 0);
    basepos--;
  }
  while (clen >= maxw);
  
  return strdup(allstr);
}
void ai_actionsavelog(char * name) {
}
void ai_dump_logs() {
  char dumpname[256];
  char msgtext[256];
  snprintf(dumpname, 255, "%s.log.txt", getArgv(1));
  snprintf(msgtext, 255, "Install Log will be saved into:\n\n<#060>%s</#>\n\nAre you sure you want to save it?", dumpname);
  byte res = aw_confirm(
               ai_win,
               "Save Install Log",
               msgtext,
               "@alert",
               NULL,
               NULL
             );
             
  if (res) {
    byte cpres = alib_copy(AROMA_INSTALL_LOG, dumpname);
    
    if (cpres == 1) {
      aw_alert(
        ai_win,
        "Save Install Log",
        "Install Logs has been saved...",
        "@info",
        NULL
      );
    }
    else {
      char errstr[3][64] = {
        "No Log Available",
        "Cannot create log file",
        "Error read & write log"
      };
      char errInfo[128];
      snprintf(errInfo, 128, "Cannot save the install logs:\n  %s", errstr[cpres - 2]);
      aw_alert(
        ai_win,
        "Save Install Log Error",
        "Install Logs has been saved...",
        "@alert",
        NULL
      );
    }
  }
}
static void * aroma_install_package(void * cookie) {
  //-- Extract update-binary
  int res = az_extract(AROMA_ORIB, AROMA_TMP "/update-binary");
  
  if (res == 0) {
    aw_post(aw_msg(15, 0, 0, 0));
    return NULL;
  }
  
  //-- Create Pipe
  int pipefd[2];
  pipe(pipefd);
  char ** argscmd       = malloc(sizeof(char *) * 5);
  char    binary[256];
  char    pipestr[10];
  //-- Init Arguments
  snprintf(binary, 255, "%s%s", AROMA_TMP, "/update-binary");
  snprintf(pipestr, 9, "%d", pipefd[1]);
  argscmd[0] = binary;
  argscmd[1] = getArgv(0);
  argscmd[2] = pipestr;
  argscmd[3] = getArgv(1);
  argscmd[4] = NULL;
  //-- Close Zip to Minimalize Memory Usage On Install
  az_close();
  //-- Start Installer
  pid_t pid = fork();
  // LOGS("Installer: Execute\n");
  
  if (pid == 0) {
    setenv("UPDATE_PACKAGE",  getArgv(1), 1);
    setenv("AROMA_TMP",       AROMA_TMP, 1);
    setenv("AROMA_VERSION",   AROMA_VERSION, 1);
    setenv("AROMA_BUILD",     AROMA_BUILD, 1);
    setenv("AROMA_BUILD_CN",  AROMA_BUILD_CN, 1);
    setenv("AROMA_NAME",      AROMA_NAME, 1);
    setenv("AROMA_COPY",      AROMA_COPY, 1);
    dup2(pipefd[1], STDOUT_FILENO);
    dup2(pipefd[1], STDERR_FILENO);
    close(pipefd[0]);
    execv(binary, argscmd);
    _exit(-1);
  }
  
  // LOGS("Installer: Initializing PIPE\n");
  close(pipefd[1]);
  //-- Set New Progress Text
  snprintf(ai_progress_text, 63, "Installing...");
  //-- Dump LOG
  FILE * fp = fopen(AROMA_INSTALL_LOG, "wb");
  FILE * fpi = fopen(AROMA_INSTALL_TXT, "wb");
  time_t rawtime;
  struct tm * timeinfo;
  time (&rawtime);
  timeinfo = localtime (&rawtime);
  fprintf(fp, AROMA_NAME " version " AROMA_VERSION "\n");
  fprintf(fp, "  " AROMA_COPY "\n\n");
  fprintf(fp, "ROM Name    : %s\n", acfg()->rom_name);
  fprintf(fp, "ROM Version : %s\n", acfg()->rom_version);
  fprintf(fp, "ROM Author  : %s\n", acfg()->rom_author);
  fprintf(fp, "Device      : %s\n", acfg()->rom_device);
  fprintf(fp, "Start at    : %s\n\n", asctime (timeinfo));
  //-- Start Reading Feedback
  char  buffer[1024];
  FILE * from_child = fdopen(pipefd[0], "r");
  // LOGS("Installer: Get Events\n");
  
  while (fgets(buffer, sizeof(buffer), from_child) != NULL) {
    char * bufall  = strdup(buffer);
    char * command = strtok(buffer, " \n");
    
    if (command == NULL) {
      free(bufall);
      continue;
    }
    else if (strcmp(command, "progress") == 0) {
      char * fraction_s    = strtok(NULL, " \n");
      char * numfiles_s    = strtok(NULL, " \n");
      float progsize      = strtof(fraction_s, NULL);
      ai_progress_fract_n = strtol(numfiles_s, NULL, 10);
      ai_progress_fract_c = 0;
      ai_progress_fract_l = alib_tick();
      
      if (ai_progress_fract_n > 0) {
        ai_progress_fract = progsize / ai_progress_fract_n;
      }
      else if (ai_progress_fract_n < 0) {
        ai_progress_fract = progsize / abs(ai_progress_fract_n);
      }
      else {
        ai_progress_fract = 0;
        ai_progress_pos   = progsize;
      }
    }
    else if (strcmp(command, "set_progress") == 0) {
      char * fraction_s = strtok(NULL, " \n");
      ai_progress_fract   = 0;
      ai_progress_fract_n = 0;
      ai_progress_fract_c = 0;
      ai_progress_pos     = strtof(fraction_s, NULL);
    }
    else if (strcmp(command, "firmware") == 0) {
      //-- Firmware Command
      fprintf(apipe(), "%s\n", ai_trim(bufall));
    }
    else if (strcmp(command, "ui_print") == 0) {
      char * str = strtok(NULL, "\n");
      
      if (str) {
        if (str[0] == '@') {
          char tmpbuf[256];
          snprintf(tmpbuf, 255, "<#selectbg_g><b>%s</b></#>", str + 1);
          actext_appendtxt(ai_buftxt, tmpbuf);
          fprintf(fpi, "%s\n", tmpbuf);
          char * t_trimmed = ai_trim(str + 1);
          snprintf(ai_progress_text, 63, "%s", t_trimmed);
          fprintf(fp, "%s\n", t_trimmed);
        }
        else {
          actext_appendtxt(ai_buftxt, str);
          fprintf(fpi, "%s\n", str);
          char * t_trimmed = ai_trim(str);
          snprintf(ai_progress_info, 100, "%s", t_trimmed);
          fprintf(fp, "  %s\n", t_trimmed);
        }
      }
    }
    else if (strcmp(command, "minzip:") == 0) {
      char * minzipcmd = ai_trim(strtok(NULL, "\""));
      
      if (strcmp(minzipcmd, "Extracted file") == 0) {
        char * filename = strtok(NULL, "\" \n");
        char * fstr = ai_fixlen(filename, "Extract:");
        
        if (fstr != NULL) {
          snprintf(ai_progress_info, 100, "<#selectbg_g>Extract:</#>%s", fstr);
          free(fstr);
        }
        else {
          snprintf(ai_progress_info, 100, "<#selectbg_g>Extract:</#>%s", filename);
        }
        
        fprintf(fp, "    Extract: %s\n", filename);
        
        if (ai_progress_fract_n > 0) {
          if (ai_progress_fract_c < ai_progress_fract_n) {
            ai_progress_fract_c++;
            ai_progress_pos += ai_progress_fract;
          }
        }
      }
    }
    else {
      char * str = ai_trim(bufall);
      fprintf(fp, "    %s\n", str);
    }
    
    // LOGS("Installer: Command(%s)\n", command);
    free(bufall);
  }
  
  // LOGS("Installer: Exited - Close Process Handler\n");
  fclose(from_child);
  //-- Get Return Status
  ai_return_status = 0;
  // LOGS("Installer: Wait For PID\n");
  waitpid(pid, &ai_return_status, 0);
  
  if (!WIFEXITED(ai_return_status) || WEXITSTATUS(ai_return_status) != 0) {
    snprintf(buffer, 1023, "Installer Error (Status %d)", WEXITSTATUS(ai_return_status));
  }
  else {
    snprintf(buffer, 1023, "Installer Sucessfull (Status %d)", WEXITSTATUS(ai_return_status));
  }
  
  // LOGS("Installer: Wait Finished\n");
  time (&rawtime);
  timeinfo = localtime (&rawtime);
  fprintf(fp, "\n\n%s\n", buffer);
  fprintf(fp, "\n\nEnd at : %s\n", asctime (timeinfo));
  fclose(fpi);
  fclose(fp);
  //-- Reopen Zip
  // LOGS("Installer: Reopen ZIP\n");
  az_init(getArgv(1));
  // LOGS("Installer: Post Finish message\n");
  aw_post(aw_msg(15, 0, 0, 0));
  return NULL;
}
static void * ac_progressthread(void * cookie) {
  //-- COLORS
  dword hl1 = ag_calchighlight(acfg()->selectbg, acfg()->selectbg_g);
  byte sg_r = ag_r(acfg()->progressglow);
  byte sg_g = ag_g(acfg()->progressglow);
  byte sg_b = ag_b(acfg()->progressglow);
  sg_r = min(sg_r * 1.4, 255);
  sg_g = min(sg_g * 1.4, 255);
  sg_b = min(sg_b * 1.4, 255);
  
  while (ai_run) {
    //-- CALCULATE PROGRESS BY TIME
    if (ai_progress_fract_n < 0) {
      long curtick  = alib_tick();
      int  targetc  = abs(ai_progress_fract_n);
      long  tickdiff = curtick - ai_progress_fract_l;
      
      if (tickdiff > 0) {
        long diffms          = tickdiff * 10;
        ai_progress_fract_l  = curtick;
        ai_progress_fract_n += diffms;
        
        if (ai_progress_fract_n >= 0) {
          diffms -= ai_progress_fract_n;
          ai_progress_fract_n = 0;
        }
        
        float curradd        = ai_progress_fract * diffms;
        ai_progress_pos     += curradd;
      }
    }
    
    //-- Safe Progress
    if (ai_progress_pos > 1) {
      ai_progress_pos = 1.0;
    }
    
    if (ai_progress_pos < 0) {
      ai_progress_pos = 0.0;
    }
    
    int prog_g = ai_prog_w; //-(ai_prog_r*2);
    int prog_w = round(ai_prog_w * ai_progress_pos);
    //-- Percent Text
    float prog_percent = 100 * ai_progress_pos;
    char prog_percent_str[10];
    snprintf(prog_percent_str, 9, "%0.2f%c", prog_percent, '%');
    int ptxt_p = agdp() * 5;
    int ptxt_y = ai_prog_oy - (ptxt_p + (ag_fontheight(0) * 2));
    int ptxt_w = ag_txtwidth(prog_percent_str, 0);
    int ptxt_x = (ai_prog_ox + ai_prog_ow) - (ptxt_w + ai_prog_or);
    int ptx1_x = ai_prog_ox + ai_prog_or;
    int ptx1_w = agw() - (agw() / 3);
    
    if (ai_progress_w < prog_w) {
      int diff       = ceil((prog_w - ai_progress_w) * 0.1);
      ai_progress_w += diff;
      
      if (ai_progress_w > prog_w) {
        ai_progress_w = prog_w;
      }
    }
    else if (ai_progress_w > prog_w) {
      int diff       = ceil((ai_progress_w - prog_w) * 0.1);
      ai_progress_w -= diff;
      
      if (ai_progress_w < prog_w) {
        ai_progress_w = prog_w;
      }
    }
    
    int issmall = -1;
    
    if (ai_progress_w < (ai_prog_r * 2)) {
      issmall = ai_progress_w;
      ai_progress_w = (ai_prog_r * 2);
    }
    
    ag_draw_ex(ai_cv, ai_bg, 0, ptxt_y, 0, ptxt_y, agw(), agh() - ptxt_y);
    int curr_prog_w = round(ai_prog_ow * ai_progress_pos);
    
    if (!atheme_draw("img.prograss.fill", ai_cv, ai_prog_ox, ai_prog_oy, curr_prog_w, ai_prog_oh)) {
      ag_roundgrad(ai_cv, ai_prog_x, ai_prog_y, ai_progress_w, ai_prog_h, acfg()->selectbg, acfg()->selectbg_g, ai_prog_r);
      ag_roundgrad_ex(ai_cv, ai_prog_x, ai_prog_y, ai_progress_w, ceil((ai_prog_h) / 2.0), LOWORD(hl1), HIWORD(hl1), ai_prog_r, 2, 2, 0, 0);
      
      if (issmall >= 0) {
        ag_draw_ex(ai_cv, ai_bg, ai_prog_x + issmall, ai_prog_oy, ai_prog_x + issmall, ai_prog_oy, (ai_prog_r * 2), ai_prog_oh);
      }
    }
    
    ag_textfs(ai_cv, ptx1_w, ptx1_x + 1, ptxt_y + 1, ai_progress_text, acfg()->winbg, 0);
    ag_texts (ai_cv, ptx1_w, ptx1_x  , ptxt_y  , ai_progress_text, acfg()->winfg, 0);
    ag_textfs(ai_cv, ai_prog_w - (ai_prog_or * 2), ptx1_x + 1, ptxt_y + 1 + ag_fontheight(0), ai_progress_info, acfg()->winbg, 0);
    ag_texts (ai_cv, ai_prog_w - (ai_prog_or * 2), ptx1_x  , ptxt_y + ag_fontheight(0) + agdp(), ai_progress_info, acfg()->winfg_gray, 0);
    ag_textfs(ai_cv, ptxt_w, ptxt_x + 1, ptxt_y + 1, prog_percent_str, acfg()->winbg, 0);
    ag_texts (ai_cv, ptxt_w, ptxt_x, ptxt_y, prog_percent_str, acfg()->winfg, 0);
    prog_g = ai_prog_w - (ai_prog_r * 2);
    
    if (++ai_progani_pos > 60) {
      ai_progani_pos = 0;
    }
    
    int x    = ai_progani_pos;
    int hpos = prog_g / 2;
    int vpos = ((prog_g + hpos) * x) / 60;
    int hhpos = prog_g / 4;
    int hph  = ai_prog_h / 2;
    int xx;
    int  sgmp = agdp() * 40;
    
    if ((vpos > 0) && (hpos > 0)) {
      for (xx = 0; xx < prog_g; xx++) {
        int alp     = 255;
        float alx   = 1.0;
        int vn = (vpos - xx) - hhpos;
        
        if ((vn > 0)) {
          if (vn < hhpos) {
            alp = (((hhpos - vn) * 255) / hhpos);
          }
          else if (vn < hpos) {
            alp = (((vn - hhpos) * 255) / hhpos);
          }
        }
        
        if (xx < sgmp) {
          alx = 1.0 - (((float) (sgmp - xx)) / sgmp);
        }
        else if (xx > prog_g - sgmp) {
          alx = 1.0 - (((float) (xx - (prog_g - sgmp))) / sgmp);
        }
        
        int alpha = min(max(alx * (255 - alp), 0), 255);
        int anix = ai_prog_x + ai_prog_r + xx;
        int yy;
        byte er = 0;
        byte eg = 0;
        byte eb = 0;
        
        for (yy = 0; yy < ai_prog_oh; yy++) {
          color * ic = agxy(ai_cv, anix, ai_prog_oy + yy);
          byte  l  = alpha * (0.5 + ((((float) yy + 1) / ((float) ai_prog_oh)) * 0.5));
          byte  ralpha = 255 - l;
          byte r = (byte) (((((int) ag_r(ic[0])) * ralpha) + (((int) sg_r) * l)) >> 8);
          byte g = (byte) (((((int) ag_g(ic[0])) * ralpha) + (((int) sg_g) * l)) >> 8);
          byte b = (byte) (((((int) ag_b(ic[0])) * ralpha) + (((int) sg_b) * l)) >> 8);
          r  = min(r + er, 255);
          g  = min(g + eg, 255);
          b  = min(b + eb, 255);
          byte nr  = ag_close_r(r);
          byte ng  = ag_close_g(g);
          byte nb  = ag_close_b(b);
          er = r - nr;
          eg = g - ng;
          eb = b - nb;
          ic[0] = ag_rgb(nr, ng, nb);
        }
      }
    }
    
    //ag_draw(NULL,ai_cv,0,0);
    //ag_sync();
    aw_draw(ai_win);
    usleep(160);
  }
  
  return NULL;
}
void aroma_init_install(
  CANVAS * bg,
  int cx, int cy, int cw, int ch,
  int px, int py, int pw, int ph
) {
  //-- Calculate Progress Location&Size
  ai_prog_oh = agdp() * 10;
  ai_prog_oy = 0;
  ai_prog_ox = px;
  ai_prog_ow = pw;
  
  if (ai_prog_oh > ph) {
    ai_prog_oh = ph;
  }
  else {
    ai_prog_oy = (ph / 2) - (ai_prog_oh / 2);
  }
  
  ai_prog_oy += py;
  ai_prog_or = ai_prog_oh / 2;
  //-- Draw Progress Holder Into BG
  dword hl1 = ag_calchighlight(acfg()->controlbg, acfg()->controlbg_g);
  
  if (!atheme_draw("img.progress", bg, px, ai_prog_oy, pw, ai_prog_oh)) {
    ag_roundgrad(bg, px, ai_prog_oy, pw, ai_prog_oh, acfg()->border, acfg()->border_g, ai_prog_or);
    ag_roundgrad(bg, px + 1, ai_prog_oy + 1, pw - 2, ai_prog_oh - 2,
                 ag_calculatealpha(acfg()->controlbg, 0xffff, 180),
                 ag_calculatealpha(acfg()->controlbg_g, 0xffff, 160), ai_prog_or - 1);
    ag_roundgrad(bg, px + 2, ai_prog_oy + 2, pw - 4, ai_prog_oh - 4, acfg()->controlbg, acfg()->controlbg_g, ai_prog_or - 2);
    ag_roundgrad_ex(bg, px + 2, ai_prog_oy + 2, pw - 4, ceil((ai_prog_oh - 4) / 2.0), LOWORD(hl1), HIWORD(hl1), ai_prog_or - 2, 2, 2, 0, 0);
  }
  
  //-- Calculate Progress Value Locations
  int hlfdp  = ceil(((float) agdp()) / 2);
  ai_prog_x = px + (hlfdp + 1);
  ai_prog_y = ai_prog_oy + (hlfdp + 1);
  ai_prog_h = ai_prog_oh - ((hlfdp * 2) + 2);
  ai_prog_w = pw - ((hlfdp * 2) + 2);
  ai_prog_r = ai_prog_or - (1 + hlfdp);
  snprintf(ai_progress_text, 63, "Initializing...");
  snprintf(ai_progress_info, 100, "");
}
int aroma_start_install(
  CANVAS * bg,
  int cx, int cy, int cw, int ch,
  int px, int py, int pw, int ph,
  CANVAS * cvf, int imgY, int chkFY, int chkFH
) {
  //-- Save Canvases
  ai_bg = bg;
  aroma_init_install(bg, cx, cy, cw, ch, px, py + (agdp() * 4), pw, ph);
  AWINDOWP hWin     = aw(bg);
  ai_win            = hWin;
  ai_cv             = &hWin->c;
  ai_progress_pos   = 0.0;
  ai_progress_w     = 0;
  ai_run            = 1;
  ai_buftxt         = actext(hWin, cx, cy + (agdp() * 5), cw, ch - (agdp() * 15), NULL, 0);
  aw_set_on_dialog(1);
  aw_show_ex(hWin, 2, 0, NULL);
  //aw_show(hWin);
  pthread_t threadProgress, threadInstaller;
  pthread_create(&threadProgress, NULL, ac_progressthread, NULL);
  pthread_create(&threadInstaller, NULL, aroma_install_package, NULL);
  byte ondispatch = 1;
  
  while (ondispatch) {
    dword msg = aw_dispatch(hWin);
    
    switch (aw_gm(msg)) {
      case 15: {
          // LOGS("Installer Dispatch: GOT Finish message\n");
          sleep(1);
          ai_run = 0;
          hWin->isActived = 0;
          pthread_join(threadProgress, NULL);
          // LOGS("pthread_join threadProgress\n");
          pthread_join(threadInstaller, NULL);
          // LOGS("pthread_join threadInstaller\n");
          // Draw Navigation
          int pad         = agdp() * 4;
          aui_drawnav(bg, 0, py - pad, agw(), ph + (pad * 2));
          ag_draw_ex(bg, cvf, 0, imgY, 0, 0, cvf->w, cvf->h);
          ag_draw(&hWin->c, bg, 0, 0);
          // Update Textbox
          ai_rebuildtxt(cx, chkFY, cw, chkFH);
          int nPad    = agdp() * 2;
          int nHeight = ph + agdp() * 4;
          int nWidth  = floor((agw() - (nPad * 2) - nHeight) / 2);
          int nY      = py - agdp() * 2;
          imgbtn(hWin, nPad, nY, nWidth, nHeight, NULL, "Save Logs", 4, 8);
          ACONTROLP nxtbtn = imgbtn(hWin, nPad + nWidth + nHeight, nY, nWidth, nHeight, aui_next_icon(), acfg()->text_next, 5, 6);
          // ACONTROLP menubtn = imgbtn(hWin, nPad + nWidth, nY, nHeight, nHeight, aui_menu_icon(), NULL, 4, 200);
          aw_show_ex(hWin, 4, 0, nxtbtn);
        }
        break;
        
      case 6: {
          ondispatch = 0;
        }
        break;
        
      case 8: {
          ai_dump_logs();
        }
        break;
    }
  }
  
  aw_set_on_dialog(0);
  aw_destroy(hWin);
  return WEXITSTATUS(ai_return_status);
}
