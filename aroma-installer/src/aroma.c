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
 * Main executable for AROMA Installer Binary
 *
 */
 
#include "aroma.h"
#include <sys/reboot.h>

//* 
//* GLOBAL UI VARIABLES
//* 
static  FILE*   acmd_pipe;
static  int     parent_pid = 0;
static  char    currArgv[2][256];
static  byte    reboot_opt = 0;

//* 
//* Pass Recovery PIPE
//* 
FILE* apipe(){
  return acmd_pipe;
}

//* 
//* Init Reboot Request
//* 
void a_check_reboot(){
  if (reboot_opt!=0){
    fprintf(apipe(),"ui_print\n");
    fprintf(apipe(),"ui_print Rebooting...\n");
    fprintf(apipe(),"ui_print\n");
    reboot(RB_AUTOBOOT);
  }
}

//* 
//* Set Reboot Request
//* 
void a_reboot(byte type){
  reboot_opt = type;
}

//* 
//* Get Command Argument
//* 
char* getArgv(int id){
  return currArgv[id];
}

//* 
//* Show Text Splash
//* 
void a_splash(char * spipe){
  int fd      = atoi(spipe);
  acmd_pipe   = fdopen(fd, "wb");
  setlinebuf(acmd_pipe);
  
  //#-- Print Info Into Recovery
  fprintf(apipe(),"ui_print\n");
  fprintf(apipe(),"ui_print Starting " AROMA_NAME " version " AROMA_VERSION "\n");
  fprintf(apipe(),"ui_print\n");
  fprintf(apipe(),"ui_print " AROMA_COPY "\n");
  fprintf(apipe(),"ui_print\n");
  fprintf(apipe(),"ui_print\n");
  usleep(1500000);
}

//* 
//* Init All Resources
//* 
void a_init_all(){
  //-- Init
  ui_init();                        //-- Init Event Handler
  ag_init();                        //-- Init Graphic Framebuffer
  ag_loadsmallfont("fonts/small");  //-- Init Small Font
  ag_loadbigfont("fonts/big");      //-- Init Big Font
}

//* 
//* Release All Resources
//* 
void a_release_all(){
  //-- Release All
  ag_closefonts();  //-- Release Fonts
  ev_exit();        //-- Release Input Engine
  az_close();       //-- Release Zip Handler
  ag_close();       //-- Release Graph Engine
  
}

//* 
//* AROMA Installer Main Executable
//* 
int main(int argc, char **argv) {
  int retval = 1;
  parent_pid = getppid();
  
  //-- Normal Updater Sequences
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);
  
  remove_directory(AROMA_TMP);
  create_directory(AROMA_TMP);

  //-- Initializing Header
  printf("Starting " AROMA_NAME " version " AROMA_VERSION "\n"
         "     " AROMA_COPY "\n");

  //-- Check Arguments
  if (argc != 4) {
      LOGE("Unexpected Number of Arguments (%d)\n", argc);
      return 1;
  }
  
  //-- Check CWM Version
  if ((argv[1][0] != '1' && argv[1][0] != '2' && argv[1][0] != '3') || argv[1][1] != '\0') {
      LOGE("Wrong Updater Binary API!!! Expected 1, 2, or 3, But got %s\n", argv[1]);
      return 2;
  }
  
  //-- Save to Argument
  snprintf(currArgv[0],255,"%s",argv[1]);
  snprintf(currArgv[1],255,"%s",argv[3]);
  
  //-- Init Pipe & Show Splash Info
  a_splash(argv[2]);
  if (az_init(argv[3])){
    a_init_all();
    if (parent_pid) kill(parent_pid,19);
    if (aui_start()){
      fprintf(apipe(),"ui_print\n");
      fprintf(apipe(),"ui_print " AROMA_NAME " Finished...\n");
      fprintf(apipe(),"ui_print\n");
      retval = 0;
    }
    if (parent_pid) kill(parent_pid,18);
    a_release_all();
  }
  
  //-- REMOVE AROMA TEMPORARY
  remove_directory(AROMA_TMP);
  
  //-- Check Reboot
  a_check_reboot();
  
  //-- Cleanup PIPE
  fclose(acmd_pipe);
  
  return retval;
}