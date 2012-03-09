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
#include <sys/reboot.h>
#include <mcheck.h>
#include "../aroma.h"

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
    usleep(2000000);
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
}

//* 
//* Release All Resources
//* 
void a_release_all(){
  //-- Release All
  ag_closefonts();  //-- Release Fonts
  LOGS("Font Released\n");
  ev_exit();        //-- Release Input Engine
  LOGS("Input Released\n");
  az_close();       //-- Release Zip Handler
  LOGS("Archive Released\n");
  ag_close();       //-- Release Graph Engine
  LOGS("Graph Released\n");
}

//* 
//* AROMA Installer Main Executable
//* 
int main(int argc, char **argv) {
#ifndef _AROMA_NODEBUG
  remove_directory("/tmp/aroma-memory");
  aroma_memory_debug_init();
#endif

  int retval = 1;
  parent_pid = getppid();
  
  LOGS("Initializing\n");
  
  //-- Normal Updater Sequences
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);
  
  //-- Init Temporaty
  remove_directory(AROMA_TMP);
  unlink(AROMA_TMP_S);
  create_directory(AROMA_TMP);
  symlink(AROMA_TMP,AROMA_TMP_S);

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
  LOGS("Saving Arguments\n");
  snprintf(currArgv[0],255,"%s",argv[1]);
  snprintf(currArgv[1],255,"%s",argv[3]);
  
  //-- Init Pipe & Show Splash Info
  a_splash(argv[2]);
  
  //-- Init Zip
  LOGS("Open Archive\n");
  if (az_init(argv[3])){
    
    //-- Initializing All Resources
    LOGS("Initializing Resource\n");
    a_init_all();
    
    //-- Mute Parent Thread
    if (parent_pid){
      LOGS("Mute Parent\n");
      aroma_memory_parentpid(parent_pid);
      kill(parent_pid,19);
    }
    
    //-- Starting AROMA Installer UI
    LOGS("Starting Interface\n");
    if (aui_start()){
      fprintf(apipe(),"ui_print\n");
      fprintf(apipe(),"ui_print " AROMA_NAME " Finished...\n");
      fprintf(apipe(),"ui_print\n");
      retval = 0;
    }
    
    //-- Close Graph Thread
    LOGS("Close Graph Thread\n");
    ag_close_thread();
    
    //-- Wait Thread Exit
    usleep(300000);
    
    //-- Unmute Parent
    if (parent_pid){
      LOGS("Unmute Parent\n");
      kill(parent_pid,18);
    }
    
    //-- Wait Until Clean Up
    usleep(200000);
    
    //-- Release All Resource
    LOGS("Starting Release\n");
    a_release_all();
  }
  else
    LOGE("Cannot Open Archive\n");
  
  //-- REMOVE AROMA TEMPORARY
  LOGS("Cleanup Temporary\n");
  usleep(500000);
  unlink(AROMA_TMP_S);
  remove_directory(AROMA_TMP);
  
  //-- Check Reboot Request
  LOGS("Check For Reboot\n");
  a_check_reboot();
  
  //-- Cleanup PIPE
  LOGS("Closing Recovery Pipe\n");
  fclose(acmd_pipe);

#ifndef _AROMA_NODEBUG
  aroma_dump_malloc();
#endif

  //-- Return Exit Status
  return retval;
}