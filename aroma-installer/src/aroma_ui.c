#include <sys/stat.h>
#include <errno.h>
#include "aroma.h"
#include "edify/expr.h"

/*************
 *
 *  GLOBAL VARIABLE & MACRO
 *
 ************/
int aparse_installpos = 0;
int aparse_backpos    = 0;
int aparse_startpos   = 0;
int aparse_isback     = 0;
int aui_isbgredraw    = 0;
int aui_minY          = 0;
CANVAS aui_bg;
CANVAS aui_win_bg;
#define _AROMA_BACKABLE_INIT() int func_pos = argv[0]->start; if ((func_pos<aparse_installpos)||(func_pos<aparse_startpos)){ aparse_backpos = func_pos; return StringValue(strdup("")); }
#define _AROMA_BACKABLE_FINISH() if (func_pos==-4) { return NULL; } aparse_backpos = func_pos;

/*************
 *
 *  DRAW BACKGROUND
 *
 ************/
void aui_bg_redraw(){
  if (!aui_isbgredraw) return;
  
  ag_blank(&aui_bg);
  int elmP  = agdp()*4;
  int capH  = ag_fontheight(1) + (elmP*2);
  aui_minY  = capH;
  ag_roundgrad(&aui_bg,0,0,agw(),agh(),acfg()->winbg,acfg()->winbg_g,acfg()->winroundsz*agdp()+2);
  ag_roundgrad_ex(&aui_bg,0,0,agw(),capH,acfg()->titlebg,acfg()->titlebg_g,(acfg()->winroundsz*agdp())-2,1,1,0,0);
  
  aui_isbgredraw = 0;
}
/*************
 *
 *  INIT BACKGROUND
 *
 ************/
void aui_initwinbg(char * titlev){
  char title[32];
  snprintf(title,31,"%s",titlev);
  aui_bg_redraw();
  int elmP  = agdp()*4;
  int titW  = ag_txtwidth(title,1);
  ag_draw(&aui_win_bg,&aui_bg,0,0);
  ag_textf(&aui_win_bg,titW,((agw()/2)-(titW/2))+1,elmP+1,title,acfg()->titlebg_g,1);
  ag_text(&aui_win_bg,titW,(agw()/2)-(titW/2),elmP,title,acfg()->titlefg,1);
}
/*************
 *
 *  FILEPROP FUNCTIONS
 *
 ************/
#define MAX_FILE_GETPROP_SIZE    65536
//-- Get File Prop - Function
char * aui_filegetprop(char * filename,char *key){
  char* result = NULL;
  char* buffer = NULL;
  struct stat st;
  
  if (stat(filename, &st) < 0) return NULL;
  if (st.st_size > MAX_FILE_GETPROP_SIZE) return NULL;
  
  buffer = malloc(st.st_size+1);
  if (buffer == NULL)
    goto done;
  
  FILE* f = fopen(filename, "rb");
  if (f == NULL) goto done;
  if (fread(buffer, 1, st.st_size, f) != st.st_size){
      fclose(f);
      goto done;
  }
  
  buffer[st.st_size] = '\0';
  fclose(f);
  char* line = strtok(buffer, "\n");
  
  do {
      while (*line && isspace(*line)) ++line;
      if (*line == '\0' || *line == '#') continue;
      char* equal = strchr(line, '=');
      if (equal == NULL) goto done;

      char* key_end = equal-1;
      while (key_end > line && isspace(*key_end)) --key_end;
      key_end[1] = '\0';

      if (strcmp(key, line) != 0) continue;

      char* val_start = equal+1;
      while(*val_start && isspace(*val_start)) ++val_start;

      char* val_end = val_start + strlen(val_start)-1;
      while (val_end > val_start && isspace(*val_end)) --val_end;
      val_end[1] = '\0';

      result = strdup(val_start);
      break;
  } while ((line = strtok(NULL, "\n")));
done:
  free(buffer);
  return result;
}
/*************
 *
 *  FileGetProp Edify
 *
 ************/
Value* FileGetPropFn(const char* name, State* state, int argc, Expr* argv[]) {
  ag_setbusy();
    char* result = NULL;
    char* filename;
    char* key;
    if (ReadArgs(state, argv, 2, &filename, &key) < 0) return NULL;
    result = aui_filegetprop(filename,key);
    if (result == NULL) result = strdup("");
  done:
    free(filename);
    free(key);
    return StringValue(result);
}
/*************
 *
 *  Get File Content
 *
 ************/
char * aui_getFileContent(char * name){
  AZMEM filedata;
  if (!az_readmem(&filedata,name,0)) return NULL;
  return filedata.data;
}
/*************
 *
 *  Get Variable
 *
 ************/
char * aui_getVariable(char * name){
  char save_prop_name[256];
  snprintf(save_prop_name,255,"%s/__.%s.__.var",AROMA_TMP,name);
  char* buffer = NULL;
  struct stat st;
  if (stat(save_prop_name,&st) < 0) return NULL;
  if (st.st_size>MAX_FILE_GETPROP_SIZE) return NULL;
  buffer = malloc(st.st_size+1);
  if (buffer == NULL) goto done;
  FILE* f = fopen(save_prop_name, "rb");
  if (f == NULL) goto done;
  if (fread(buffer, 1, st.st_size, f) != st.st_size){
      fclose(f);
      goto done;
  }
  buffer[st.st_size] = '\0';
  fclose(f);
  return buffer;
done:
  free(buffer);
  return NULL;
}
/*************
 *
 *  Write To TMP File
 *
 ************/
void aui_writeFile(char * name, char * value){
  char save_prop_name[256];
  snprintf(save_prop_name,255,"%s/%s",AROMA_TMP,name);
  FILE * fp = fopen(save_prop_name,"wb");
  if (fp!=NULL){
    fwrite(value,1,strlen(value),fp);
    fclose(fp);
  }
}
/*************
 *
 *  Set Vaariable
 *
 ************/
void aui_setVariable(char * name, char * value){
  char save_prop_name[256];
  snprintf(save_prop_name,255,"%s/__.%s.__.var",AROMA_TMP,name);
  FILE * fp = fopen(save_prop_name,"wb");
  if (fp!=NULL){
    fwrite(value,1,strlen(value),fp);
    fclose(fp);
  }
}
void aui_appendVariable(char * name, char * value){
  char save_prop_name[256];
  snprintf(save_prop_name,255,"%s/__.%s.__.var",AROMA_TMP,name);
  FILE * fp = fopen(save_prop_name,"ab");
  if (fp!=NULL){
    fwrite(value,1,strlen(value),fp);
    fclose(fp);
  }
}
void aui_prependVariable(char * name, char * value){
  char save_prop_name[256];
  snprintf(save_prop_name,255,"%s/__.%s.__.var",AROMA_TMP,name);
  char * buf = aui_getVariable(name);
  FILE * fp = fopen(save_prop_name,"wb");
  if (fp!=NULL){
    fwrite(value,1,strlen(value),fp);
    if (buf!=NULL){
      fwrite(buf,1,strlen(buf),fp);
    }
    fclose(fp);
  }
  if (buf!=NULL){
    free(buf);
  }
}

/*************
 *
 *  Get Zip Content to String
 *
 ************/
Value* AROMA_GETFILECONTENTS(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc!=1) {
    return ErrorAbort(state, "%s() expects 1 args (zip entry path), got %d", name, argc);
  }
  ag_setbusy();
  int i;
  char** args = ReadVarArgs(state, argc, argv);
  if (args==NULL) return NULL;
  
  char * buf = aui_getFileContent(args[0]);
  for (i=0;i<argc;++i) {
    free(args[i]);
  }
  free(args);
  
  if (buf!=NULL){
    return StringValue(buf);
  }  
  return StringValue(strdup(""));
}
/*************
 *
 *  Show Please Wait Progress
 *
 ************/
Value* AROMA_ONPLEASEWAIT(const char* name, State* state, int argc, Expr* argv[]) {
  int func_pos = argv[0]->start; 
  if (func_pos<aparse_startpos){
    return StringValue(strdup(""));
  }
  
  if (argc!=1) {
    return ErrorAbort(state, "%s() expects 1 args (wait text), got %d", name, argc);
  }
  int i;
  char** args = ReadVarArgs(state, argc, argv);
  if (args==NULL) return NULL;
    
  char txt[32];
  snprintf(txt,31,args[0]);
  ag_setbusy_withtext(txt);
  for (i=0;i<argc;++i) {
    free(args[i]);
  }
  free(args);
  return StringValue(strdup(""));
}
/*************
 *
 *  Get Zip Contents From Aroma Path
 *
 ************/
Value* AROMA_GETFILECONTENTSA(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc!=1) {
    return ErrorAbort(state, "%s() expects 1 args (zip entry path in aroma dir), got %d", name, argc);
  }
  ag_setbusy();
  int i;
  char** args = ReadVarArgs(state, argc, argv);
  if (args==NULL) return NULL;
  char save_prop_name[256];
  snprintf(save_prop_name,255,"%s/%s",AROMA_DIR,args[0]);
  char * buf = aui_getFileContent(save_prop_name);
  for (i=0;i<argc;++i) {
    free(args[i]);
  }
  free(args);
  
  if (buf!=NULL){
    return StringValue(buf);
  }  
  return StringValue(strdup(""));
}
/*************
 *
 *  Write String Into TMP File
 *
 ************/
Value* AROMA_WRITETOTMP(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc!=2) {
    return ErrorAbort(state, "%s() expects 2 args (tmp filename, value), got %d", name, argc);
  }
  ag_setbusy();
  int i;
  char** args = ReadVarArgs(state, argc, argv);
  if (args==NULL) return NULL;
  
  aui_writeFile(args[0],args[1]);
  
  for (i=0;i<argc;++i) {
    free(args[i]);
  }
  free(args);
  return StringValue(strdup(""));
}
/*************
 *
 *  Get Variable
 *
 ************/
Value* AROMA_GETVAR(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc!=1) {
    return ErrorAbort(state, "%s() expects 1 args (variable name), got %d", name, argc);
  }
  ag_setbusy();
  int i;
  char** args = ReadVarArgs(state, argc, argv);
  if (args==NULL) return NULL;
  
  char * buf = aui_getVariable(args[0]);
  for (i=0;i<argc;++i) {
    free(args[i]);
  }
  free(args);
  if (buf!=NULL){
    return StringValue(buf);
  }
  return StringValue(strdup(""));
}
/*************
 *
 *  Integer Comparison Function
 *
 ************/
Value* AROMA_CMP(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc!=3) {
    return ErrorAbort(state, "%s() expects 3 args (val1, logic, val2), got %d", name, argc);
  }
  ag_setbusy();
  int i;
  char** args = ReadVarArgs(state, argc, argv);
  if (args==NULL) return NULL;
  
  byte ret = 0;
  int val1 = atoi(args[0]);
  int val2 = atoi(args[2]);
  
  if (strcmp(args[1],"==")==0){
    ret = (val1==val2)?1:0;
  }
  else if (strcmp(args[1],">")==0){
    ret = (val1>val2)?1:0;
  }
  else if (strcmp(args[1],"<")==0){
    ret = (val1<val2)?1:0;
  }
  else if (strcmp(args[1],">=")==0){
    ret = (val1>=val2)?1:0;
  }
  else if (strcmp(args[1],"<=")==0){
    ret = (val1<=val2)?1:0;
  }
  else if (strcmp(args[1],"!=")==0){
    ret = (val1!=val2)?1:0;
  }
  
  for (i=0;i<argc;++i) {
    free(args[i]);
  }
  free(args);
  if (ret){
    return StringValue(strdup("1"));
  }
  return StringValue(strdup(""));
}

/*************
 *
 *  INLINE IF
 *
 ************/
Value* AROMA_IIF(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc!=3) {
    return ErrorAbort(state, "%s() expects 3 args (logic, trueval, falseval), got %d", name, argc);
  }
  ag_setbusy();
  int i;
  char** args = ReadVarArgs(state, argc, argv);
  if (args==NULL) return NULL;
  
  char * ret = NULL;
  if (args[0][0]=='\0'){
    ret = strdup(args[2]);
  }
  else{
    ret = strdup(args[1]);
  }
  
  for (i=0;i<argc;++i) {
    free(args[i]);
  }
  free(args);

  return StringValue(ret);
}
/*************
 *
 *  Integer Basic Math Function
 *
 ************/
Value* AROMA_MATH(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc!=3) {
    return ErrorAbort(state, "%s() expects 3 args (val1, operator, val2), got %d", name, argc);
  }
  ag_setbusy();
  int i;
  char** args = ReadVarArgs(state, argc, argv);
  if (args==NULL) return NULL;
  
  byte ret = 0;
  int val1 = atoi(args[0]);
  int val2 = atoi(args[2]);
  
  if (strcmp(args[1],"+")==0){
    ret = val1+val2;
  }
  else if (strcmp(args[1],"-")==0){
    ret = val1-val2;
  }
  else if (strcmp(args[1],"*")==0){
    ret = val1*val2;
  }
  else if (strcmp(args[1],"/")==0){
    ret = val1/val2;
  }
  else if (strcmp(args[1],"\%")==0){
    ret = val1%val2;
  }
  
  for (i=0;i<argc;++i) {
    free(args[i]);
  }
  free(args);
  
  char retstr[128];
  snprintf(retstr,128,"%i",ret);
  StringValue(strdup(retstr));
}
/*************
 *
 *  Set Variable
 *
 ************/
Value* AROMA_SETVAR(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc!=2) {
    return ErrorAbort(state, "%s() expects 2 args (variable name, value), got %d", name, argc);
  }
  ag_setbusy();
  int i;
  char** args = ReadVarArgs(state, argc, argv);
  if (args==NULL) return NULL;
  
  aui_setVariable(args[0],args[1]);
  
  for (i=0;i<argc;++i) {
    free(args[i]);
  }
  free(args);
  return StringValue(strdup(""));
}
/*************
 *
 *  Append Variable
 *
 ************/
Value* AROMA_APPENDVAR(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc!=2) {
    return ErrorAbort(state, "%s() expects 2 args (variable name, value), got %d", name, argc);
  }
  ag_setbusy();
  int i;
  char** args = ReadVarArgs(state, argc, argv);
  if (args==NULL) return NULL;
  
  aui_appendVariable(args[0],args[1]);
  
  for (i=0;i<argc;++i) {
    free(args[i]);
  }
  free(args);
  return StringValue(strdup(""));
}
/*************
 *
 *  Prepend Variable
 *
 ************/
Value* AROMA_PREPENDVAR(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc!=2) {
    return ErrorAbort(state, "%s() expects 2 args (variable name, value), got %d", name, argc);
  }
  ag_setbusy();
  int i;
  char** args = ReadVarArgs(state, argc, argv);
  if (args==NULL) return NULL;
  
  aui_prependVariable(args[0],args[1]);
  
  for (i=0;i<argc;++i) {
    free(args[i]);
  }
  free(args);
  return StringValue(strdup(""));
}
/*************
 *
 *  CALIBRATE TOUCH SCREEN
 *
 ************/
Value* AROMA_CALIBRATE(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc != 4) {
    return ErrorAbort(state, "%s() expects 4 args (div-x, add-x, div-y, add-y), got %d", name, argc);
  }
  ag_setbusy();
  char* dx;
  char* dy;
  char* ax;
  char* ay;
  if (ReadArgs(state, argv, 4, &dx, &ax, &dy, &ay) < 0){
      return NULL;
  }
  atouch_set_calibrate((float) atof(dx),atoi(ax),(float) atof(dy),atoi(ay));
  free(dx);
  free(dy);
  free(ax);
  free(ay);
  return StringValue(strdup(""));
}
/*************
 *
 *  SET UI COLORS
 *
 ************/
Value* AROMA_SETCOLOR(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc != 2) {
    return ErrorAbort(state, "%s() expects 2 args (color type, hexcolor in string), got %d", name, argc);
  }
  ag_setbusy();
  char* typ;
  char* val;
  if (ReadArgs(state, argv, 2, &typ, &val) < 0){
      return NULL;
  }
  color cl = strtocolor(val);
  if (strcmp(typ,"winbg") == 0) acfg()->winbg=cl;
  else if (strcmp(typ,"winbg_g") == 0) acfg()->winbg_g=cl;
  else if (strcmp(typ,"textbg") == 0) acfg()->textbg=cl;
  else if (strcmp(typ,"textfg") == 0) acfg()->textfg=cl;
  else if (strcmp(typ,"textfg_gray") == 0) acfg()->textfg_gray=cl;
  else if (strcmp(typ,"controlbg") == 0) acfg()->controlbg=cl;
  else if (strcmp(typ,"controlbg_g") == 0) acfg()->controlbg_g=cl;
  else if (strcmp(typ,"controlfg") == 0) acfg()->controlfg=cl;
  else if (strcmp(typ,"selectbg") == 0) acfg()->selectbg=cl;
  else if (strcmp(typ,"selectbg_g") == 0) acfg()->selectbg_g=cl;
  else if (strcmp(typ,"selectfg") == 0) acfg()->selectfg=cl;
  else if (strcmp(typ,"titlebg") == 0) acfg()->titlebg=cl;
  else if (strcmp(typ,"titlebg_g") == 0) acfg()->titlebg_g=cl;
  else if (strcmp(typ,"titlefg") == 0) acfg()->titlefg=cl;
  else if (strcmp(typ,"scrollbar") == 0) acfg()->scrollbar=cl;
  else if (strcmp(typ,"navbg") == 0) acfg()->navbg=cl;
  else if (strcmp(typ,"navbg_g") == 0) acfg()->navbg_g=cl;
  else if (strcmp(typ,"border") == 0) acfg()->border=cl;
  else if (strcmp(typ,"border_g") == 0) acfg()->border_g=cl;
    
  free(val);
  free(typ);
  aui_isbgredraw = 1;
  return StringValue(strdup(""));
}
/*************
 *
 *  SET CONFIG
 *
 ************/
Value* AROMA_INI_SET(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc != 2) {
    return ErrorAbort(state, "%s() expects 2 args (config name, config value in string), got %d", name, argc);
  }
  ag_setbusy();
  char* typ;
  char* val;
  if (ReadArgs(state, argv, 2, &typ, &val) < 0){
      return NULL;
  }
  byte valint = (byte) min(atoi(val),255);
  int  valkey = (int) atoi(val);
  
  //-- Property
  if (strcmp(typ,"roundsize") == 0) acfg()->roundsz=valint;
  else if (strcmp(typ,"button_roundsize") == 0) acfg()->btnroundsz=valint;
  else if (strcmp(typ,"window_roundsize") == 0) acfg()->winroundsz=valint;
  else if (strcmp(typ,"transition_frame") == 0) acfg()->fadeframes=valint;
  else if (strcmp(typ,"text_ok") == 0) snprintf(acfg()->text_ok,31,val);
  else if (strcmp(typ,"text_next") == 0) snprintf(acfg()->text_next,31,val);
  else if (strcmp(typ,"text_back") == 0) snprintf(acfg()->text_back,31,val);
    
  else if (strcmp(typ,"rom_name") == 0) snprintf(acfg()->rom_name,63,val);
  else if (strcmp(typ,"rom_version") == 0) snprintf(acfg()->rom_version,63,val);
  else if (strcmp(typ,"rom_author") == 0) snprintf(acfg()->rom_author,63,val);
  else if (strcmp(typ,"rom_device") == 0) snprintf(acfg()->rom_device,63,val);
  
  else if (strcmp(typ,"customkeycode_up") == 0) acfg()->ckey_up=valkey;
  else if (strcmp(typ,"customkeycode_down") == 0) acfg()->ckey_down=valkey;
  else if (strcmp(typ,"customkeycode_select") == 0) acfg()->ckey_select=valkey;
  else if (strcmp(typ,"customkeycode_back") == 0) acfg()->ckey_back=valkey;
  else if (strcmp(typ,"customkeycode_menu") == 0) acfg()->ckey_menu=valkey;
  
   
  free(val);
  free(typ);
  aui_isbgredraw = 1;
  return StringValue(strdup(""));
}
/*************
 *
 *  SHOW SPLASH SCREEN
 *
 ************/
//-- Splash Screen
Value* AROMA_SPLASH(const char* name, State* state, int argc, Expr* argv[]) {
  int func_pos = argv[0]->start; 
  if (func_pos<aparse_startpos){
    return StringValue(strdup(""));
  }
  
  if (argc != 2) {
    return ErrorAbort(state, "%s() expects 2 args (delay in milisecond, image name), got %d", name, argc);
  }
  ag_setbusy();
  char* delay;
  char* imgname;
  if (ReadArgs(state, argv, 2, &delay, &imgname) < 0){
      return NULL;
  }
  int delayint = atoi(delay);
  
  //-- Set Temporary
  CANVAS tmpbg;
  ag_canvas(&tmpbg,agw(),agh());
  ag_draw(&tmpbg,agc(),0,0);
  
  //-- Fade To Half Black
  //ag_rectopa(agc(),0,0,agw(),agh(),0x0000,160);
  
  //-- Create Splash BG
  CANVAS splashbg;
  ag_canvas(&splashbg,agw(),agh());
  ag_blur(&splashbg,agc(),agdp()*2);
  
  //-- Load PNG
  PNGCANVAS ap;
  if (apng_load(&ap,imgname)){
    apng_draw(&splashbg,&ap,(agw()/2)-(ap.w/2),(agh()/2)-(ap.h/2));
    apng_close(&ap);
  }
  ag_draw(NULL,&splashbg,0,0);
  
  //-- Wait The Fade
  ag_sync_fade(acfg()->fadeframes);
  
  free(delay);
  free(imgname);
  
  //-- WAIT
  usleep(1000*delayint);
  
  //-- Redraw Prev Display
  ag_draw(NULL,&tmpbg,0,0);
  ag_sync_fade_wait(acfg()->fadeframes);
  
  //-- Cleanup
  ag_ccanvas(&splashbg);
  ag_ccanvas(&tmpbg);
  
  return StringValue(strdup(""));
}
/*************
 *
 *  VIEW UI (String)
 *
 ************/
Value* AROMA_VIEWUI(const char* name, State* state, int argc, Expr* argv[]) {
  _AROMA_BACKABLE_INIT();

  if (argc!=3) {
    return ErrorAbort(state, "%s() expects 3 args (title,desc,ico), got %d", name, argc);
  }
  ag_setbusy();
  int i;
  char** args = ReadVarArgs(state, argc, argv);
  if (args==NULL) return NULL;
  
  aui_initwinbg(args[0]);
  char text[1024];
  snprintf(text,1023,"%s",args[1]);
  
  int pad         = agdp() * 4;
  int chkW        = agw() - (pad*2);
  int bntH        = agdp() * 20;
  int chkH        = agh() - ( aui_minY + bntH + (pad*4));
  int chkY        = aui_minY + pad;
  int btnY        = chkY + chkH + (pad*2);
  ag_roundgrad_ex(&aui_win_bg,0,btnY-pad,agw(),bntH+(pad*2),acfg()->navbg,acfg()->navbg_g,(acfg()->winroundsz*agdp())-2,0,0,1,1);
    
  //-- Load Icon
  PNGCANVAS ap;
  byte imgE       = 0;
  int  imgA       = 0;
  int  imgW       = 0;
  int  imgH       = 0;
  int  tifX       = pad*2;
  int  imgX       = pad;
  int  tifY       = chkY;
  int  imgY       = chkY;
  if (apng_load(&ap,args[2])){
    imgE  = 1;
    imgW  = min(ap.w,agdp()*30);
    imgH  = min(ap.h,agdp()*30);
    imgA  = imgW;
    tifX += imgA; 
  }
  int txtH        = ag_txtheight(chkW-((pad*2)+imgA),text,0);
  if (imgE){
    if (txtH<imgH){
      tifY+= (imgH-txtH)/2;
      txtH=imgH;
    }
    apng_draw_ex(&aui_win_bg,&ap,imgX,imgY,0,0,imgW,imgH);
    apng_close(&ap);
  }
  
  //-- Draw Text
  ag_textf(&aui_win_bg,chkW-((pad*2)+imgA),tifX+1,tifY+1,text,acfg()->textbg,0);
  ag_text(&aui_win_bg,chkW-((pad*2)+imgA),tifX,tifY,text,acfg()->textfg,0);
  
  //-- Resize Checkbox Size & Pos
  chkY+=txtH+pad;
  chkH-=txtH+pad;
  
  AWINDOWP hWin   = aw(&aui_win_bg);

  if ((aparse_backpos>0)&&(aparse_backpos>aparse_installpos)){
    //-- BACK BUTTON
    acbutton(
      hWin,
      pad,btnY,(chkW/2)-(agdp()*2),bntH,acfg()->text_back,0,
      5
    );
  }
  //-- NEXT BUTTON
  ACONTROLP nxtbtn=acbutton(
    hWin,
    pad+(agdp()*2)+(chkW/2),btnY,(chkW/2)-(agdp()*2),bntH,acfg()->text_next,0,
    6
  );

  //-- Free Arguments
  for (i=0;i<argc;++i) {
    free(args[i]);
  }
  free(args);
  
  aw_show(hWin);
  aw_setfocus(hWin,nxtbtn);
  byte ondispatch     = 1;
  while(ondispatch){
    dword msg=aw_dispatch(hWin);
    switch (aw_gm(msg)){
      case 6:{
          ondispatch = 0;
      }
      break;
      case 5:{
        //-- BACK
        if ((aparse_backpos>0)&&(aparse_backpos>aparse_installpos)) {
          aparse_startpos = aparse_backpos;
          aparse_backpos  = 0;
          aparse_isback   = 1;
          ondispatch      = 0;
        }
      }
      break;
      case 4:{
        //-- EXIT
        func_pos        = -4;
        ondispatch      = 0;
      }
      break;
    }
  }
  //-- Window
  aw_destroy(hWin);
  
  if (aparse_isback) return NULL;
  _AROMA_BACKABLE_FINISH();
  return StringValue(strdup(""));
}
/*************
 *
 *  SHOW TEXT UI
 *
 ************/
Value* AROMA_TEXTUI(const char* name, State* state, int argc, Expr* argv[]) {
  _AROMA_BACKABLE_INIT();
  byte isplain = (strcmp(name,"textbox")==0)?1:0;
  // textbox(title,desc,ico,text);
  if ((isplain)&&(argc!=4)) {
    return ErrorAbort(state, "%s() expects 4 args (title,desc,ico,text), got %d", name, argc);
  }
  else if ((!isplain)&&(argc!=6)) {
    return ErrorAbort(state, "%s() expects 5 args (title,desc,ico,text,agreetext,unchkmessage), got %d", name, argc);
  }
  ag_setbusy();
  int i;
  char** args = ReadVarArgs(state, argc, argv);
  if (args==NULL) return NULL;
  
  aui_initwinbg(args[0]);
  char text[256];
  snprintf(text,255,"%s",args[1]);
  
  char unchkmsg[256];
  if (!isplain) snprintf(unchkmsg,255,"%s",args[5]);
  char titletxt[32];
  snprintf(titletxt,31,"%s",args[0]);
  
  int pad         = agdp() * 4;
  int chkW        = agw() - (pad*2);
  int bntH        = agdp() * 20;
  int chkH        = agh() - ( aui_minY + bntH + (pad*4));
  int chkY        = aui_minY + pad;
  int btnY        = chkY + chkH + (pad*2);
  ag_roundgrad_ex(&aui_win_bg,0,btnY-pad,agw(),bntH+(pad*2),acfg()->navbg,acfg()->navbg_g,(acfg()->winroundsz*agdp())-2,0,0,1,1);
    
  //-- Load Icon
  PNGCANVAS ap;
  byte imgE       = 0;
  int  imgA       = 0;
  int  imgW       = 0;
  int  imgH       = 0;
  int  tifX       = pad*2;
  int  imgX       = pad;
  int  tifY       = chkY;
  int  imgY       = chkY;
  if (apng_load(&ap,args[2])){
    imgE  = 1;
    imgW  = min(ap.w,agdp()*30);
    imgH  = min(ap.h,agdp()*30);
    imgA  = imgW;
    tifX += imgA; 
  }
  int txtH        = ag_txtheight(chkW-((pad*2)+imgA),text,0);
  if (imgE){
    if (txtH<imgH){
      tifY+= (imgH-txtH)/2;
      txtH=imgH;
    }
    apng_draw_ex(&aui_win_bg,&ap,imgX,imgY,0,0,imgW,imgH);
    apng_close(&ap);
  }
  
  //-- Draw Text
  ag_textf(&aui_win_bg,chkW-((pad*2)+imgA),tifX+1,tifY+1,text,acfg()->textbg,0);
  ag_text(&aui_win_bg,chkW-((pad*2)+imgA),tifX,tifY,text,acfg()->textfg,0);
  
  //-- Resize Checkbox Size & Pos
  chkY+=txtH+pad;
  chkH-=txtH+pad;
  
  AWINDOWP hWin   = aw(&aui_win_bg);
  ACONTROLP txtbox;
  ACONTROLP agreecb;
  
  if (isplain){
    txtbox = actext(hWin,pad,chkY,chkW,chkH,args[3],0);
  }
  else{
    //-- Check Box
    int chkaH         = agdp()*20;
    int textBoxH      = chkH-(chkaH+pad);
    txtbox            = actext(hWin,pad,chkY,chkW,textBoxH,args[3],0);
    agreecb           = accb(hWin,pad*2,chkY+textBoxH+pad,chkW-(pad*2),chkaH,args[4],0);
  }

  if ((aparse_backpos>0)&&(aparse_backpos>aparse_installpos)){
    //-- BACK BUTTON
    acbutton(
      hWin,
      pad,btnY,(chkW/2)-(agdp()*2),bntH,acfg()->text_back,0,
      5
    );
  }
  //-- NEXT BUTTON
  ACONTROLP nxtbtn=acbutton(
    hWin,
    pad+(agdp()*2)+(chkW/2),btnY,(chkW/2)-(agdp()*2),bntH,acfg()->text_next,0,
    6
  );

  //-- Free Arguments
  for (i=0;i<argc;++i) {
    free(args[i]);
  }
  free(args);
  
  aw_show(hWin);
  aw_setfocus(hWin,nxtbtn);
  byte ondispatch     = 1;
  while(ondispatch){
    dword msg=aw_dispatch(hWin);
    switch (aw_gm(msg)){
      case 6:{
        if (!isplain){
          if (!accb_ischecked(agreecb)){
            aw_alert(hWin,titletxt,unchkmsg,"icons/alert",acfg()->text_ok);
          }
          else
            ondispatch = 0;
        }
        else{
          ondispatch = 0;
        }
      }
      break;
      case 5:{
        //-- BACK
        if ((aparse_backpos>0)&&(aparse_backpos>aparse_installpos)) {
          aparse_startpos = aparse_backpos;
          aparse_backpos  = 0;
          aparse_isback   = 1;
          ondispatch      = 0;
        }
      }
      break;
      case 4:{
        //-- EXIT
        func_pos        = -4;
        ondispatch      = 0;
      }
      break;
    }
  }
  //-- Window
  aw_destroy(hWin);
  
  if (aparse_isback) return NULL;
  _AROMA_BACKABLE_FINISH();
  return StringValue(strdup(""));
}
/*************
 *
 *  SHOW CHECKBOX UI
 *
 ************/
Value* AROMA_CHECKBOX(const char* name, State* state, int argc, Expr* argv[]) {
  _AROMA_BACKABLE_INIT();
  if (argc<7) {
    return ErrorAbort(state, "%s() expects more than 7 args, got %d", name, argc);
  }
  else if ((argc-4)%3!=0){
    return ErrorAbort(state, "%s() expects 4 args + 3 args per items, got %d", name, argc);
  }
  ag_setbusy();
  int i;
  char** args = ReadVarArgs(state, argc, argv);
  if (args==NULL) return NULL;
  
  aui_initwinbg(args[0]);
  char save_prop_name[256];
  char text[256];
  snprintf(save_prop_name,255,"%s/%s",AROMA_TMP,args[3]);
  snprintf(text,255,"%s",args[1]);
  
  int pad         = agdp() * 4;
  int chkW        = agw() - (pad*2);
  int bntH        = agdp() * 20;
  int chkH        = agh() - ( aui_minY + bntH + (pad*4));
  int chkY        = aui_minY + pad;
  int btnY        = chkY + chkH + (pad*2);
  ag_roundgrad_ex(&aui_win_bg,0,btnY-pad,agw(),bntH+(pad*2),acfg()->navbg,acfg()->navbg_g,(acfg()->winroundsz*agdp())-2,0,0,1,1);
    
  //-- Load Icon
  PNGCANVAS ap;
  byte imgE       = 0;
  int  imgA       = 0;
  int  imgW       = 0;
  int  imgH       = 0;
  int  tifX       = pad*2;
  int  imgX       = pad;
  int  tifY       = chkY;
  int  imgY       = chkY;
  if (apng_load(&ap,args[2])){
    imgE  = 1;
    imgW  = min(ap.w,agdp()*30);
    imgH  = min(ap.h,agdp()*30);
    imgA  = imgW;
    tifX += imgA; 
  }
  int txtH        = ag_txtheight(chkW-((pad*2)+imgA),text,0);
  if (imgE){
    if (txtH<imgH){
      tifY+= (imgH-txtH)/2;
      txtH=imgH;
    }
    apng_draw_ex(&aui_win_bg,&ap,imgX,imgY,0,0,imgW,imgH);
    apng_close(&ap);
  }
  
  //-- Draw Text
  ag_textf(&aui_win_bg,chkW-((pad*2)+imgA),tifX+1,tifY+1,text,acfg()->textbg,0);
  ag_text(&aui_win_bg,chkW-((pad*2)+imgA),tifX,tifY,text,acfg()->textfg,0);
  
  //-- Resize Checkbox Size & Pos
  chkY+=txtH+pad;
  chkH-=txtH+pad;
  
  AWINDOWP hWin   = aw(&aui_win_bg);
  
  //-- Check Box
  ACONTROLP chk1  = accheck(hWin,pad,chkY,chkW,chkH);
  
  if ((aparse_backpos>0)&&(aparse_backpos>aparse_installpos)){
    //-- BACK BUTTON
    acbutton(
      hWin,
      pad,btnY,(chkW/2)-(agdp()*2),bntH,acfg()->text_back,0,
      5
    );
  }
  //-- NEXT BUTTON
  ACONTROLP nxtbtn=acbutton(
    hWin,
    pad+(agdp()*2)+(chkW/2),btnY,(chkW/2)-(agdp()*2),bntH,acfg()->text_next,0,
    6
  );
  //-- Populate Checkbox Items
  char save_prop_key[64];
  int idx = 0;
  int group_id = 0;
  for (i=4;i<argc;i+=3) {
    byte defchk = (byte) atoi(args[i+2]);
    if (defchk==3){
    }
    else if (defchk==2){
      if (accheck_addgroup(chk1,args[i],args[i+1])){
        group_id++;
        idx=0;
      }
    }
    else{
      idx++;
      snprintf(save_prop_key,63,"item.%d.%d",group_id,idx);
      char * res = aui_filegetprop(save_prop_name,save_prop_key);
      if (res!=NULL){
        defchk = (strcmp(res,"1")==0)?1:0;
        free(res);
      }
      accheck_add(chk1,args[i],args[i+1],defchk);
    }
  }
  
  
  //-- Free Arguments
  for (i=0;i<argc;++i) {
    free(args[i]);
  }
  free(args);
  aw_show(hWin);
  aw_setfocus(hWin,nxtbtn);
  byte ondispatch     = 1;
  
  while(ondispatch){
    dword msg=aw_dispatch(hWin);
    switch (aw_gm(msg)){
      case 6: ondispatch = 0; break;
      case 5:{
        //-- BACK
        if ((aparse_backpos>0)&&(aparse_backpos>aparse_installpos)) {
          aparse_startpos = aparse_backpos;
          aparse_backpos  = 0;
          aparse_isback   = 1;
          ondispatch      = 0;
        }
      }
      break;
      case 4:{
        //-- EXIT
        func_pos        = -4;
        ondispatch      = 0;
      }
      break;
    }
  }
  //-- Collecting Items:
  FILE * fp = fopen(save_prop_name,"wb");
  if (fp!=NULL){
    int itemcnt = accheck_itemcount(chk1);
    for (i=0;i<itemcnt;i++) {
      if (!accheck_isgroup(chk1,i)){
        byte state = accheck_ischecked(chk1,i);
        snprintf(save_prop_key,63,"item.%d.%d=%d\n",accheck_getgroup(chk1,i),accheck_getgroupid(chk1,i)+1,state);
        fwrite(save_prop_key,1,strlen(save_prop_key),fp);
      }
    }
    fclose(fp);
  }
  
  //-- Window
  aw_destroy(hWin);
  
  if (aparse_isback) return NULL;
  _AROMA_BACKABLE_FINISH();
  return StringValue(strdup(""));
}
/*************
 *
 *  SHOW SELECTBOX UI
 *
 ************/
Value* AROMA_SELECTBOX(const char* name, State* state, int argc, Expr* argv[]) {
  _AROMA_BACKABLE_INIT();
  if (argc<7) {
    return ErrorAbort(state, "%s() expects more than 7 args, got %d", name, argc);
  }
  else if ((argc-4)%3!=0){
    return ErrorAbort(state, "%s() expects 4 args + 3 args per items, got %d", name, argc);
  }
  ag_setbusy();
  int i;
  char** args = ReadVarArgs(state, argc, argv);
  if (args==NULL) return NULL;
  
  aui_initwinbg(args[0]);
  char save_prop_name[256];
  char text[256];
  snprintf(save_prop_name,255,"%s/%s",AROMA_TMP,args[3]);
  //snprintf(save_prop_name,255,"%s",args[3]);
  snprintf(text,255,"%s",args[1]);
  
  int pad         = agdp() * 4;
  int chkW        = agw() - (pad*2);
  int bntH        = agdp() * 20;
  int chkH        = agh() - ( aui_minY + bntH + (pad*4));
  int chkY        = aui_minY + pad;
  int btnY        = chkY + chkH + (pad*2);
  ag_roundgrad_ex(&aui_win_bg,0,btnY-pad,agw(),bntH+(pad*2),acfg()->navbg,acfg()->navbg_g,(acfg()->winroundsz*agdp())-2,0,0,1,1);
    
  //-- Load Icon
  PNGCANVAS ap;
  byte imgE       = 0;
  int  imgA       = 0;
  int  imgW       = 0;
  int  imgH       = 0;
  int  tifX       = pad*2;
  int  imgX       = pad;
  int  tifY       = chkY;
  int  imgY       = chkY;
  if (apng_load(&ap,args[2])){
    imgE  = 1;
    imgW  = min(ap.w,agdp()*30);
    imgH  = min(ap.h,agdp()*30);
    imgA  = imgW;
    tifX += imgA; 
  }
  int txtH        = ag_txtheight(chkW-((pad*2)+imgA),text,0);
  if (imgE){
    if (txtH<imgH){
      tifY+= (imgH-txtH)/2;
      txtH=imgH;
    }
    apng_draw_ex(&aui_win_bg,&ap,imgX,imgY,0,0,imgW,imgH);
    apng_close(&ap);
  }
  
  //-- Draw Text
  ag_textf(&aui_win_bg,chkW-((pad*2)+imgA),tifX+1,tifY+1,text,acfg()->textbg,0);
  ag_text(&aui_win_bg,chkW-((pad*2)+imgA),tifX,tifY,text,acfg()->textfg,0);
  
  //-- Resize Checkbox Size & Pos
  chkY+=txtH+pad;
  chkH-=txtH+pad;
  
  AWINDOWP hWin   = aw(&aui_win_bg);
  
  //-- Check Box
  ACONTROLP opt1  = acopt(hWin,pad,chkY,chkW,chkH);
  
  if ((aparse_backpos>0)&&(aparse_backpos>aparse_installpos)){
    //-- BACK BUTTON
    acbutton(
      hWin,
      pad,btnY,(chkW/2)-(agdp()*2),bntH,acfg()->text_back,0,
      5
    );
  }
  //-- NEXT BUTTON
  ACONTROLP nxtbtn = acbutton(
    hWin,
    pad+(agdp()*2)+(chkW/2),btnY,(chkW/2)-(agdp()*2),bntH,acfg()->text_next,0,
    6
  );
  
  char save_prop_key[64];
  //-- Populate Checkbox Items
  int group_id = 0;
  int idx      = 0;
  for (i=4;i<argc;i+=3) {
    byte defchk = (byte) atoi(args[i+2]);
    if (defchk==3){
    }
    else if (defchk==2){
      if (acopt_addgroup(opt1,args[i],args[i+1])){
        group_id++;
        idx      = 0;
      }
    }
    else{
      idx++;
      snprintf(save_prop_key,63,"selected.%d",group_id);
      char * savedsel = aui_filegetprop(save_prop_name,save_prop_key);
      snprintf(save_prop_key,63,"%d",idx);
      if (savedsel!=NULL){
        defchk = (strcmp(savedsel,save_prop_key)==0)?1:0;
        free(savedsel);
      }
      acopt_add(opt1,args[i],args[i+1],defchk);
    }
  }
  
  //-- Free Arguments
  for (i=0;i<argc;++i) {
    free(args[i]);
  }
  free(args);
  aw_show(hWin);
  aw_setfocus(hWin,nxtbtn);
  byte ondispatch     = 1;
  
  while(ondispatch){
    dword msg=aw_dispatch(hWin);
    switch (aw_gm(msg)){
      case 6:{
        ondispatch = 0;
      }
      break;
      case 5:{
        //-- BACK
        if ((aparse_backpos>0)&&(aparse_backpos>aparse_installpos)) {
          aparse_startpos = aparse_backpos;
          aparse_backpos  = 0;
          aparse_isback   = 1;
          ondispatch      = 0;
        }
      }
      break;
      case 4:{
        //-- EXIT
        func_pos        = -4;
        ondispatch      = 0;
      }
      break;
    }
  }
  
  //-- Collecting Items:
  FILE * fp = fopen(save_prop_name,"wb");
  if (fp!=NULL){
    for (i=0;i<=group_id;i++){
      int selidx   = acopt_getselectedindex(opt1,i);
      if (selidx!=-1){
        int selindex = acopt_getgroupid(opt1,selidx)+1;
        snprintf(save_prop_key,63,"selected.%d=%d\n",i,selindex);
        fwrite(save_prop_key,1,strlen(save_prop_key),fp);
      }
    }
    fclose(fp);
  }
  
  aw_destroy(hWin);
  
  if (aparse_isback) return NULL;
  _AROMA_BACKABLE_FINISH();
  return StringValue(strdup(""));
}


/*************
 *
 *  SHOW MENUBOX UI
 *
 ************/
Value* AROMA_MENUBOX(const char* name, State* state, int argc, Expr* argv[]) {
  _AROMA_BACKABLE_INIT();
  if (argc<7) {
    return ErrorAbort(state, "%s() expects more than 7 args, got %d", name, argc);
  }
  else if ((argc-4)%3!=0){
    return ErrorAbort(state, "%s() expects 4 args + 3 args per items, got %d", name, argc);
  }
  ag_setbusy();
  int i;
  char** args = ReadVarArgs(state, argc, argv);
  if (args==NULL) return NULL;
  
  aui_initwinbg(args[0]);
  char save_prop_name[256];
  char text[256];
  snprintf(save_prop_name,255,"%s/%s",AROMA_TMP,args[3]);
  snprintf(text,255,"%s",args[1]);
  
  int pad         = agdp() * 4;
  int chkW        = agw() - (pad*2);
  int bntH        = agdp() * 20;
  int chkH        = agh() - ( aui_minY + bntH + (pad*4));
  int chkY        = aui_minY + pad;
  int btnY        = chkY + chkH + (pad*2);
  ag_roundgrad_ex(&aui_win_bg,0,btnY-pad,agw(),bntH+(pad*2),acfg()->navbg,acfg()->navbg_g,(acfg()->winroundsz*agdp())-2,0,0,1,1);
    
  //-- Load Icon
  PNGCANVAS ap;
  byte imgE       = 0;
  int  imgA       = 0;
  int  imgW       = 0;
  int  imgH       = 0;
  int  tifX       = pad*2;
  int  imgX       = pad;
  int  tifY       = chkY;
  int  imgY       = chkY;
  if (apng_load(&ap,args[2])){
    imgE  = 1;
    imgW  = min(ap.w,agdp()*30);
    imgH  = min(ap.h,agdp()*30);
    imgA  = imgW;
    tifX += imgA; 
  }
  int txtH        = ag_txtheight(chkW-((pad*2)+imgA),text,0);
  if (imgE){
    if (txtH<imgH){
      tifY+= (imgH-txtH)/2;
      txtH=imgH;
    }
    apng_draw_ex(&aui_win_bg,&ap,imgX,imgY,0,0,imgW,imgH);
    apng_close(&ap);
  }
  
  //-- Draw Text
  ag_textf(&aui_win_bg,chkW-((pad*2)+imgA),tifX+1,tifY+1,text,acfg()->textbg,0);
  ag_text(&aui_win_bg,chkW-((pad*2)+imgA),tifX,tifY,text,acfg()->textfg,0);
  
  //-- Resize Checkbox Size & Pos
  chkY+=txtH+pad;
  chkH-=txtH+pad;
  
  AWINDOWP hWin   = aw(&aui_win_bg);
  
  //-- Check Box
  ACONTROLP menu1  = acmenu(hWin,pad,chkY,chkW,chkH,6);
  
  if ((aparse_backpos>0)&&(aparse_backpos>aparse_installpos)){
    //-- BACK BUTTON
    acbutton(
      hWin,
      pad,btnY,(chkW/2)-(agdp()*2),bntH,acfg()->text_back,0,
      5
    );
  }
  //-- NEXT BUTTON
  /*
  ACONTROLP nxtbtn = acbutton(
    hWin,
    pad+(agdp()*2)+(chkW/2),btnY,(chkW/2)-(agdp()*2),bntH,acfg()->text_next,0,
    6
  );*/
  char save_prop_key[64];
  //-- Populate Checkbox Items
  for (i=4;i<argc;i+=3) {
    if (strcmp(args[i],"")!=0)
      acmenu_add(menu1,args[i],args[i+1],args[i+2]);
  }

  //-- Free Arguments
  for (i=0;i<argc;++i) {
    free(args[i]);
  }

  free(args);
  aw_show(hWin);
  // aw_setfocus(hWin,nxtbtn);
  byte ondispatch     = 1;
  
  while(ondispatch){
    dword msg=aw_dispatch(hWin);
    switch (aw_gm(msg)){
      case 6:{
        ondispatch = 0;
      }
      break;
      case 5:{
        //-- BACK
        if ((aparse_backpos>0)&&(aparse_backpos>aparse_installpos)) {
          aparse_startpos = aparse_backpos;
          aparse_backpos  = 0;
          aparse_isback   = 1;
          ondispatch      = 0;
        }
      }
      break;
      case 4:{
        //-- EXIT
        func_pos        = -4;
        ondispatch      = 0;
      }
      break;
    }
  }
  
  int selindex = acmenu_getselectedindex(menu1)+1;
  snprintf(save_prop_key,63,"selected=%d\n",selindex);
  //printf(save_prop_key);
  
  //-- Collecting Items:
  FILE * fp = fopen(save_prop_name,"wb");
  if (fp!=NULL){
    fwrite(save_prop_key,1,strlen(save_prop_key),fp);
    fclose(fp);
    
  }
  
  aw_destroy(hWin);
  
  if (aparse_isback) return NULL;
  _AROMA_BACKABLE_FINISH();
  return StringValue(strdup(""));
}
Value* AROMA_CALIBTOOL(const char* name, State* state, int argc, Expr* argv[]) {
  int func_pos = argv[0]->start; 
  if (func_pos<aparse_startpos){
    return StringValue(strdup(""));
  }
  //aui_initwinbg("Calibrating Tool");
  //AWINDOWP hWin   = aw(&aui_win_bg);
  aw_calibtools(NULL);
  //aw_destroy(hWin);
  return StringValue(strdup(""));
}
/*************
 *
 *  SHOW ALERT DIALOG
 *
 ************/
Value* AROMA_ALERT(const char* name, State* state, int argc, Expr* argv[]) {
  int func_pos = argv[0]->start; 
  if (func_pos<aparse_startpos){
    return StringValue(strdup(""));
  }
  
  if ((argc<2)||(argc>4)) {
    return ErrorAbort(state, "%s() expects 2-4 args (title, text, [icon, ok text]), got %d", name, argc);
  }
  ag_setbusy();
  int i;
  char** args = ReadVarArgs(state, argc, argv);
  if (args==NULL) return NULL;
  
  aw_alert(
    NULL,
    args[0],
    args[1],
    (argc>2)?args[2]:"",
    (argc>3)?args[3]:NULL
  );
  
  for (i=0;i<argc;++i) {
    free(args[i]);
  }
  free(args);
  return StringValue(strdup(""));
}

/*************
 *
 *  SHOW CONFIRM DIALOG
 *
 ************/
Value* AROMA_CONFIRM(const char* name, State* state, int argc, Expr* argv[]) {
  int func_pos = argv[0]->start; 
  if (func_pos<aparse_startpos){
    return StringValue(strdup(""));
  }
  
  if ((argc<2)||(argc>5)) {
    return ErrorAbort(state, "%s() expects 2-4 args (title, text, [icon, yes text, no text]), got %d", name, argc);
  }
  ag_setbusy();
  int i;
  char** args = ReadVarArgs(state, argc, argv);
  if (args==NULL) return NULL;
  
  byte res = aw_confirm(
    NULL,
    args[0],
    args[1],
    (argc>2)?args[2]:"",
    (argc>3)?args[3]:NULL,
    (argc>4)?args[4]:NULL
  );
  
  for (i=0;i<argc;++i) {
    free(args[i]);
  }
  free(args);
  
  if (res) return StringValue(strdup("yes"));
  return StringValue(strdup("no"));
}

/*************
 *
 *  SHOW TEXT DIALOG
 *
 ************/
Value* AROMA_TEXTDIALOG(const char* name, State* state, int argc, Expr* argv[]) {
  int func_pos = argv[0]->start; 
  if (func_pos<aparse_startpos){
    return StringValue(strdup(""));
  }
  
  if ((argc<2)||(argc>3)) {
    return ErrorAbort(state, "%s() expects 2-3 args (title, text [, ok text]), got %d", name, argc);
  }
  ag_setbusy();
  int i;
  char** args = ReadVarArgs(state, argc, argv);
  if (args==NULL) return NULL;
  
  aw_textdialog(
    NULL,
    args[0],
    args[1],
    (argc>2)?args[2]:NULL
  );
  
  for (i=0;i<argc;++i) {
    free(args[i]);
  }
  free(args);
  return StringValue(strdup(""));
}
/*************
 *
 *  TERMINATE INSTALLER
 *
 ************/
Value* AROMA_EXIT(const char* name, State* state, int argc, Expr* argv[]) {
  fprintf(apipe(),"ui_print\n");
  fprintf(apipe(),"ui_print Exit Installer...\n");
  fprintf(apipe(),"ui_print\n");
  return NULL;
}
/*************
 *
 *  PARTITION INFORMATION Function
 *
 ************/
Value* AROMA_GETPARTSIZE(const char* name, State* state, int argc, Expr* argv[]) {
  byte ispercent=0;
  if (strcmp(name,"getdiskusedpercent")==0){
    if (argc!=1) {
      return ErrorAbort(state, "%s() expects 1 args (mountpoint), got %d", name, argc);
    }
    ispercent=1;
  }
  else{
    if ((argc!=1)&&(argc!=2)) {
      return ErrorAbort(state, "%s() expects 1 or 2 args (mountpoint [, unit(b,k,m)]), got %d", name, argc);
    }
  }
  ag_setbusy();
  char** args = ReadVarArgs(state, argc, argv);
  if (args==NULL) return NULL;
  int  ret = -1;
  int i;
  byte mtd = ismounted(args[0]);
  if (!mtd){
    alib_exec("/sbin/mount",args[0]);
    if (!ismounted(args[0])){
      // printf("Not Mounted...\n");
      goto done;
    }
  }
  if (ispercent)
    ret = alib_diskusage(args[0]);
  else if (strcmp(name,"getdisksize")==0)
    ret = alib_disksize(args[0]);
  else
    ret = alib_diskfree(args[0]);
  
  if ((ispercent==0)&&(argc==2)){
    if (args[1][0]=='k') ret=ret/1024;
    else if (args[1][0]=='m') ret=ret/(1024*1024);
  }
  if (!mtd){
    alib_exec("/sbin/umount",args[0]);
  }
done:
  for (i=0;i<argc;++i) {
    free(args[i]);
  }
  free(args);
  char retstr[64];
  snprintf(retstr,63,"%i",ret);  
  return StringValue(strdup(retstr));
}

/*************
 *
 *  Install UI
 *
 ************/
Value* AROMA_INSTALLUI(const char* name, State* state, int argc, Expr* argv[]) {
  _AROMA_BACKABLE_INIT();
  
  if (argc!=3) {
    return ErrorAbort(state, "%s() expects 3 args (title,desc,ico), got %d", name, argc);
  }
  ag_setbusy();
  int i;
  char** args = ReadVarArgs(state, argc, argv);
  if (args==NULL) return NULL;
  aui_initwinbg(args[0]);
  char text[256];
  snprintf(text,255,"%s",args[1]);
  int pad         = agdp() * 4;
  int chkW        = agw() - (pad*2);
  int bntH        = agdp() * 20;
  int chkH        = agh() - ( aui_minY + bntH + (pad*4));
  int chkY        = aui_minY + pad;
  int btnY        = chkY + chkH + (pad*2);
  // ag_roundgrad_ex(&aui_win_bg,0,btnY-pad,agw(),bntH+(pad*2),acfg()->navbg,acfg()->navbg_g,(acfg()->winroundsz*agdp())-2,0,0,1,1);
    
  //-- Load Icon
  PNGCANVAS ap;
  byte imgE       = 0;
  int  imgA       = 0;
  int  imgW       = 0;
  int  imgH       = 0;
  int  tifX       = pad*2;
  int  imgX       = pad;
  int  tifY       = chkY;
  int  imgY       = chkY;
  if (apng_load(&ap,args[2])){
    imgE  = 1;
    imgW  = min(ap.w,agdp()*30);
    imgH  = min(ap.h,agdp()*30);
    imgA  = imgW;
    tifX += imgA; 
  }
  int txtH        = ag_txtheight(chkW-((pad*2)+imgA),text,0);
  if (imgE){
    if (txtH<imgH){
      tifY+= (imgH-txtH)/2;
      txtH=imgH;
    }
    apng_draw_ex(&aui_win_bg,&ap,imgX,imgY,0,0,imgW,imgH);
    apng_close(&ap);
  }
  
  //-- Draw Text
  ag_textf(&aui_win_bg,chkW-((pad*2)+imgA),tifX+1,tifY+1,text,acfg()->textbg,0);
  ag_text(&aui_win_bg,chkW-((pad*2)+imgA),tifX,tifY,text,acfg()->textfg,0);
  
  //-- Resize Checkbox Size & Pos
  chkY+=txtH+pad;
  chkH-=txtH+pad;
  
  //-- Free Arguments
  for (i=0;i<argc;++i) {
    free(args[i]);
  }
  free(args);
  
  aroma_start_install(
    &aui_win_bg,
    pad,chkY,chkW,chkH,
    pad,btnY,chkW,bntH
  );

  aparse_installpos = func_pos;
  // return NULL;
  return StringValue(strdup(""));
}
/*************
 *
 *  REGISTER FEATURES
 *
 ************/
void RegisterAroma() {
  //-- CONFIG FUNCTIONS
  RegisterFunction("setcolor",      AROMA_SETCOLOR);
  RegisterFunction("ini_set",       AROMA_INI_SET);
  RegisterFunction("calibrate",     AROMA_CALIBRATE);
  RegisterFunction("calibtool",     AROMA_CALIBTOOL);
  
  //-- VARIABLE FUNCTIONS
  RegisterFunction("setvar",        AROMA_SETVAR);
  RegisterFunction("getvar",        AROMA_GETVAR);
  RegisterFunction("appendvar",     AROMA_APPENDVAR);
  RegisterFunction("prependvar",    AROMA_PREPENDVAR);
  
  //-- FILE & ZIP CONTENT FUNCTIONS
  RegisterFunction("file_getprop",  FileGetPropFn);
  RegisterFunction("readfile",      AROMA_GETFILECONTENTS);
  RegisterFunction("readfile_aroma",AROMA_GETFILECONTENTSA);
  RegisterFunction("writetmpfile",  AROMA_WRITETOTMP);

  //-- MAIN UI FUNCTIONS (With Next & Back Buttons)
  RegisterFunction("splash",        AROMA_SPLASH);
  RegisterFunction("checkbox",      AROMA_CHECKBOX);
  RegisterFunction("selectbox",     AROMA_SELECTBOX);
  RegisterFunction("textbox",       AROMA_TEXTUI);
  RegisterFunction("viewbox",       AROMA_VIEWUI);
  RegisterFunction("agreebox",      AROMA_TEXTUI);
  RegisterFunction("menubox",       AROMA_MENUBOX);
  
  //-- INSTALL UI
  RegisterFunction("install",       AROMA_INSTALLUI);
  
  //-- DIALOG UI FUNCTIONS
  RegisterFunction("alert",         AROMA_ALERT);
  RegisterFunction("textdialog",    AROMA_TEXTDIALOG);
  RegisterFunction("confirm",       AROMA_CONFIRM);
  
  //-- DISK INFO FUNCTIONS
  RegisterFunction("getdisksize", AROMA_GETPARTSIZE);
  RegisterFunction("getdiskfree", AROMA_GETPARTSIZE);
  RegisterFunction("getdiskusedpercent", AROMA_GETPARTSIZE);
  
  //-- COMPARISON & MATH
  RegisterFunction("cmp", AROMA_CMP);
  RegisterFunction("cal", AROMA_MATH);  
  RegisterFunction("iif", AROMA_IIF);
  
  //-- ETC
  RegisterFunction("exit",          AROMA_EXIT);
  RegisterFunction("pleasewait",    AROMA_ONPLEASEWAIT);
  
}
/*************
 *
 *  START UI
 *
 ************/
byte aui_start(){
  //-- LOAD CONFIG SCRIPT
  AZMEM script_installer;
  if (!az_readmem(&script_installer,AROMA_CFG,0)) return 0;
  char * script_data = script_installer.data;
  
  //-- EDIFY REGISTRATION:
  RegisterBuiltins();
  RegisterAroma();
  FinishRegistration();
  
  //-- PARSE CONFIG SCRIPT
  Expr* root;
  int error_count = 0;
  yy_scan_string(script_data);
  int error = yyparse(&root, &error_count);
  if (error != 0 || error_count > 0) {
      vibrate(50);
      fprintf(apipe(),"ui_print\n");
      fprintf(apipe(),"ui_print SYNTAX ERROR!!! aroma-config on line %d col %d\n",yyErrLine(),yyErrCol());
      fprintf(apipe(),"ui_print\n");
      usleep(200000);
      vibrate(50);
      return 0;
  }

  //-- EVALUATE CONFIG SCRIPT
  State state;
  state.cookie = NULL;
  state.script = script_data;
  state.errmsg = NULL;
  char* result = NULL;
  
  //-- EVALUATE & TEST FOR BACK ACTIONS
  ag_canvas(&aui_bg,agw(),agh());
  ag_canvas(&aui_win_bg,agw(),agh());
  aparse_installpos = 0;
  do{
    acfg_init();
    aui_isbgredraw = 1;
    if (result!=NULL) free(result);
    aparse_isback   = 0;
    result = Evaluate(&state, root);
  }while(aparse_isback);
  ag_ccanvas(&aui_win_bg);
  ag_ccanvas(&aui_bg);
  
  //-- CLEANUP & ERROR HANDLER
  if (result == NULL) {
    byte res = 0;
    if (state.errmsg == NULL){
        fprintf(apipe(),"ui_print\n");
        fprintf(apipe(),"ui_print AROMA Installer Terminated...\n");
        fprintf(apipe(),"ui_print\n");
        res = 1;
    }
    else{
        vibrate(50);
        fprintf(apipe(),"ui_print\n");
        fprintf(apipe(),"ui_print FUNCTION ERROR!!! aroma-config: %s\n",state.errmsg);
        fprintf(apipe(),"ui_print\n");
        usleep(200000);
        vibrate(50);
    }
    free(script_installer.data);
    free(state.errmsg);
    return res;
  }
  else{
    free(script_installer.data);
    free(result);
  }
  return 1;
}