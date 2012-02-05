#include "aroma.h"

FILE* acmd_pipe;
FILE* apipe(){ return acmd_pipe; }
int   parent_pid = 0;
char currArgv[2][256];
char* getArgv(int id){
  return currArgv[id];
}
void a_splash(char * spipe){
  // printf("\nTEST 1: r:%i, c:%i, f:%i\n",round(4.439),ceil(4.123),floor(7.83));
  
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
  usleep(500000);
}
void a_init_all(){
  //-- Init
  ui_init();
  ag_init();
  ag_loadsmallfont("small");
  ag_loadbigfont("big");
}
void a_release_all(){
  //-- Release All
  ag_closefonts();  //-- Release Fonts
  ev_exit();        //-- Release Input Engine
  az_close();       //-- Release Zip Handler
  ag_close();       //-- Release Graph Engine
  
}

//**********[ AROMA MAIN EXECUTABLE ]**********//
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
  
  //-- Cleanup All
  fclose(acmd_pipe);
  remove_directory(AROMA_TMP);
  
  return retval;
}