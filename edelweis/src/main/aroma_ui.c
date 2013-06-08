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
 * Source code for parsing and processing edify script (aroma-config)
 *
 */

#include <sys/stat.h>       //-- Filesystem Stats
#include "../edify/expr.h"  //-- Edify Parser
#include "../aroma.h"

#define APARSE_MAXHISTORY 256

//* 
//* GLOBAL UI VARIABLES
//* 
static  int     aparse_installpos = 0;  //-- Is already run install function
static  int     aparse_backpos    = 0;  //-- Back Position in edify
static  int     aparse_startpos   = 0;  //-- Start Position of current expression
static  byte    aparse_isback     = 0;  //-- Is NULL return was Back Message
static  byte    aui_isbgredraw    = 0;  //-- Is Background Need Redrawed
static  int     aui_minY          = 0;  //-- Most Top Allowable UI Draw Position (Y)
static  CANVAS  aui_bg;                 //-- Saved CANVAS for background
static  CANVAS  aui_win_bg;             //-- Current drawed CANVAS for windows background

//-- Back History
static  int     aparse_history[APARSE_MAXHISTORY];
static  int     aparse_history_pos= 0;

//* 
//* MACROS
//* 
#define _INITBACK() \
        int func_pos = argv[0]->start; \
        if (aparse_history_pos<255) { \
          aparse_history[aparse_history_pos++]=func_pos; \
        } \
        if ((func_pos<aparse_installpos)||(func_pos<aparse_startpos)){ \
          aparse_backpos = func_pos; \
          return StringValue(strdup("")); \
        }

#define _FINISHBACK() \
        if (func_pos==-4){ \
          return NULL; \
        } \
        aparse_backpos = func_pos;

#define _INITARGS() \
          char** args = ReadVarArgs(state, argc, argv); \
          if (args==NULL) return NULL;

#define _FREEARGS() \
          int freearg_i; \
          for (freearg_i=0;freearg_i<argc;++freearg_i) free(args[freearg_i]); \
          free(args);

#define MAX_FILE_GETPROP_SIZE    65536

/************************************[ AROMA INSTALLER UI - LIBRARIES ]************************************/

//* 
//* Redraw Window Background
//* 
void aui_redraw(){
  if (!aui_isbgredraw) return;
  ag_blank(&aui_bg);
  int elmP  = agdp()*4;
  int capH  = ag_fontheight(1) + (elmP*2);
  aui_minY  = capH;
  
  ag_rect(&aui_bg,0,0,agw(),agh(),0x0000);
  
  //-- Background
  if (!atheme_id_draw(0, &aui_bg, 0, 0, agw(),agh())){
    ag_roundgrad(&aui_bg,0,0,agw(),agh(),acfg()->winbg,acfg()->winbg_g,acfg()->winroundsz*agdp()+2);
  }
  
  //-- Titlebar
  if (!atheme_id_draw(1, &aui_bg, 0, 0, agw(),capH)){
    ag_roundgrad_ex(&aui_bg,0,0,agw(),capH,acfg()->titlebg,acfg()->titlebg_g,(acfg()->winroundsz*agdp())-2,1,1,0,0);
  }
  
  aui_isbgredraw = 0;
}

//* 
//* Init Window Background With New Title
//* 
void aui_setbg(char * titlev){
  char title[64];
  snprintf(title,64,"%s",titlev);
  aui_redraw();
  int elmP  = agdp()*4;
  int titW  = ag_txtwidth(title,1);
  ag_draw(&aui_win_bg,&aui_bg,0,0);
  ag_textf(&aui_win_bg,titW,((agw()/2)-(titW/2))+1,elmP+1,title,acfg()->titlebg_g,1);
  ag_text(&aui_win_bg,titW,(agw()/2)-(titW/2),elmP,title,acfg()->titlefg,1);
}

//* 
//* Draw Navigation Bar
//*
void aui_drawnav(CANVAS * bg,int x, int y, int w, int h){
  if (!atheme_id_draw(2, bg, x, y, w, h)){
    ag_roundgrad_ex(
      bg,x,y,w,h,
      acfg()->navbg,
      acfg()->navbg_g,
      (acfg()->winroundsz*agdp())-2,0,0,1,1
    );
  }
}

//* 
//* Read Strings From filesystem
//* 
char * aui_readfromfs(char * name){
  char* buffer = NULL;
  struct stat st;
  if (stat(name,&st) < 0) return NULL;
  if (st.st_size>MAX_FILE_GETPROP_SIZE) return NULL;
  buffer = malloc(st.st_size+1);
  if (buffer == NULL) goto done;
  FILE* f = fopen(name, "rb");
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

//* 
//* Write Strings into file
//* 
void aui_writetofs(char * name, char * value){
  FILE * fp = fopen(name,"wb");
  if (fp!=NULL){
    fwrite(value,1,strlen(value),fp);
    fclose(fp);
  }
}

//* 
//* Read Strings From Temporary File
//*
char * aui_readfromtmp(char * name){
  char path[256];
  snprintf(path,256,"%s/%s",AROMA_TMP,name);
  aui_readfromfs(path);
  return NULL;
}

//* 
//* Write Strings From Temporary File
//*
void aui_writetotmp(char * name, char * value){
  char path[256];
  snprintf(path,256,"%s/%s",AROMA_TMP,name);
  aui_writetofs(path,value);
}

//* 
//* Read Strings From ZIP
//* 
char * aui_readfromzip(char * name){
  AZMEM filedata;
  if (!az_readmem(&filedata,name,0)) return NULL;
  return filedata.data;
}

//* 
//* Parse PROP String
//* 
char * aui_parsepropstring(char * bf,char *key){
  char* result = NULL;  
  if (bf==NULL) return result;
  char* buffer=strdup(bf);
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
  free(buffer);
done:
  
  return result;
}

//* 
//* Parse PROP Files
//* 
char * aui_parseprop(char * filename,char *key){
  char * buffer = aui_readfromfs(filename);
  char * result = aui_parsepropstring(buffer,key);
  free(buffer);
  return result;
}

//* 
//* Parse PROP from ZIP
//* 
char * aui_parsepropzip(char * filename,char *key){
  char * buffer = aui_readfromzip(filename);
  char * result = aui_parsepropstring(buffer,key);
  free(buffer);
  return result;
}

//* 
//* Read Variable
//* 
char * aui_getvar(char * name){
  char path[256];
  snprintf(path,256,"%s/.__%s.var",AROMA_TMP,name);
  return aui_readfromfs(path);
}

//* 
//* Set Variable
//* 
void aui_setvar(char * name, char * value){
  char path[256];
  snprintf(path,256,"%s/.__%s.var",AROMA_TMP,name);
  aui_writetofs(path,value);
}

//* 
//* Append Variable
//* 
void aui_appendvar(char * name, char * value){
  char path[256];
  snprintf(path,256,"%s/.__%s.var",AROMA_TMP,name);
  FILE * fp = fopen(path,"ab");
  if (fp!=NULL){
    fwrite(value,1,strlen(value),fp);
    fclose(fp);
  }
}

//* 
//* Delete Variable
//* 
void aui_delvar(char * name){
  char path[256];
  snprintf(path,256,"%s/.__%s.var",AROMA_TMP,name);
  unlink(path);
}

//* 
//* Prepend Variable
//* 
void aui_prependvar(char * name, char * value){
  char path[256];
  snprintf(path,256,"%s/.__%s.var",AROMA_TMP,name);
  char * buf = aui_getvar(name);
  FILE * fp = fopen(path,"wb");
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

//* 
//* Set Colorset From Prop String
//* 
void aui_setthemecolor(char * prop, char * key, color * cl){
  char * val = aui_parsepropstring(prop,key);
  if (val!=NULL){
    cl[0] = strtocolor(val);
    free(val);
  }
}
//* 
//* Set Drawing Config From Prop String
//* 
void aui_setthemeconfig(char * prop, char * key, byte * b){
  char * val = aui_parsepropstring(prop,key);
  if (val!=NULL){
    b[0] = (byte) min(atoi(val),255);
    free(val);
  }
}

/************************************[ AROMA EDIFY HANDLERS ]************************************/
//* 
//* loadtruefont
//*
Value* AROMA_FONT(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc!=3) {
    return ErrorAbort(state, "%s() expects 3 args (fonttype, fontpath, size), got %d", name, argc);
  }
  
  //-- This is Busy Function
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  char zpath[256];
  if (strcmp(name,"fontresload")==0)
    snprintf(zpath,256,"%s/",AROMA_DIR);
  else
    snprintf(zpath,256,"");
  
  int size = atoi(args[2]);
  if (args[0][0]=='0'){
    if (!ag_loadsmallfont(args[1], size, zpath))
      ag_loadsmallfont("fonts/small",0,NULL);
  }
  else{
    if (!ag_loadbigfont(args[1], size, zpath))
      ag_loadbigfont("fonts/big",0,NULL);
  }
  
  //-- Release Arguments
  _FREEARGS();
  
  //-- Return
  return StringValue(strdup(""));
  
}
//* 
//* set_theme
//* 
Value* AROMA_THEME(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc!=1) {
    return ErrorAbort(state, "%s() expects 1 args (themename), got %d", name, argc);
  }
  
  //-- This is Busy Function
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  acfg_init_ex(1);
  
  if ((strcmp(args[0],"")==0)||(strcmp(args[0],"generic")==0)){
    //-- Background Should Be Redrawed
    aui_isbgredraw = 1;
    
    //-- Release Arguments
    _FREEARGS();
    
    //-- Return
    return StringValue(strdup(""));
  }

  //-- Parse The Prop
  char themename[256];
  snprintf(themename,256,"%s/themes/%s/theme.prop",AROMA_DIR,args[0]);
  snprintf(acfg()->themename,64,"%s",args[0]);
  char * propstr = aui_readfromzip(themename);
  if (propstr){
    int i=0;
    for (i=0;i<AROMA_THEME_CNT;i++){
      char * key = atheme_key(i);
      char * val = aui_parsepropstring(propstr,key);
      if (val!=NULL){
        if (strcmp(val,"")!=0){
          snprintf(themename,256,"themes/%s/%s",args[0],val);
          atheme_create(key,themename);
        }
        free(val);
      }
    }
    //printf("PASS THEME\n");
    aui_setthemecolor(propstr,  "color.winbg",              &acfg()->winbg);
    aui_setthemecolor(propstr,  "color.winbg_g",            &acfg()->winbg_g);
    aui_setthemecolor(propstr,  "color.winfg",              &acfg()->winfg);
    aui_setthemecolor(propstr,  "color.winfg_gray",         &acfg()->winfg_gray);
    aui_setthemecolor(propstr,  "color.dialogbg",           &acfg()->dialogbg);
    aui_setthemecolor(propstr,  "color.dialogbg_g",         &acfg()->dialogbg_g);
    aui_setthemecolor(propstr,  "color.dialogfg",           &acfg()->dialogfg);
    aui_setthemecolor(propstr,  "color.textbg",             &acfg()->textbg);
    aui_setthemecolor(propstr,  "color.textfg",             &acfg()->textfg);
    aui_setthemecolor(propstr,  "color.textfg_gray",        &acfg()->textfg_gray);
    aui_setthemecolor(propstr,  "color.controlbg",          &acfg()->controlbg);
    aui_setthemecolor(propstr,  "color.controlbg_g",        &acfg()->controlbg_g);
    aui_setthemecolor(propstr,  "color.controlfg",          &acfg()->controlfg);
    aui_setthemecolor(propstr,  "color.selectbg",           &acfg()->selectbg);
    aui_setthemecolor(propstr,  "color.selectbg_g",         &acfg()->selectbg_g);
    aui_setthemecolor(propstr,  "color.selectfg",           &acfg()->selectfg);
    aui_setthemecolor(propstr,  "color.titlebg",            &acfg()->titlebg);
    aui_setthemecolor(propstr,  "color.titlebg_g",          &acfg()->titlebg_g);
    aui_setthemecolor(propstr,  "color.titlefg",            &acfg()->titlefg);
    aui_setthemecolor(propstr,  "color.dlgtitlebg",         &acfg()->dlgtitlebg);
    aui_setthemecolor(propstr,  "color.dlgtitlebg_g",       &acfg()->dlgtitlebg_g);
    aui_setthemecolor(propstr,  "color.dlgtitlefg",         &acfg()->dlgtitlefg);
    aui_setthemecolor(propstr,  "color.scrollbar",          &acfg()->scrollbar);
    aui_setthemecolor(propstr,  "color.navbg",              &acfg()->navbg);
    aui_setthemecolor(propstr,  "color.navbg_g",            &acfg()->navbg_g);
    aui_setthemecolor(propstr,  "color.border",             &acfg()->border);
    aui_setthemecolor(propstr,  "color.border_g",           &acfg()->border_g);
    aui_setthemecolor(propstr,  "color.progressglow",       &acfg()->progressglow);
    
    
    
    aui_setthemeconfig(propstr, "config.roundsize",         &acfg()->roundsz);
    aui_setthemeconfig(propstr, "config.button_roundsize",  &acfg()->btnroundsz);
    aui_setthemeconfig(propstr, "config.window_roundsize",  &acfg()->winroundsz);
    aui_setthemeconfig(propstr, "config.transition_frame",  &acfg()->fadeframes);
    
    //printf("PASS THEME V\n");
    
    //-- LOAD SMALL FONT
    char * font = aui_parsepropstring(propstr,"font.small");
    if (font!=NULL){
      if (!ag_isfreetype(0)){
        snprintf(themename,256,"themes/%s/%s",args[0],font);
        if (!ag_loadsmallfont(themename,0,NULL))
          ag_loadsmallfont("fonts/small",0,NULL);
      }
      free(font);
    }
    
    //-- LOAD BIG FONT
    font = aui_parsepropstring(propstr,"font.big");
    if (font!=NULL){
      if (!ag_isfreetype(0)){
        snprintf(themename,256,"themes/%s/%s",args[0],font);
        if (!ag_loadbigfont(themename,0,NULL))
          ag_loadbigfont("fonts/big",0,NULL);
      }
      free(font);
    }

    free(propstr);
  }
  else{
    snprintf(acfg()->themename,64,"");
  }

  //-- Background Should Be Redrawed
  aui_isbgredraw = 1;
  
  //-- Release Arguments
  _FREEARGS();
  
  //-- Return
  return StringValue(strdup(""));
}

//* 
//* package_extract
//* 
Value* AROMA_EXTRACT(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc!=2) {
    return ErrorAbort(state, "%s() expects 2 args (zip_path, destination), got %d", name, argc);
  }
  
  //-- This is Busy Function
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  byte res=0;
  char dpath[256];
  snprintf(dpath,256,"%s/%s",AROMA_TMP,args[1]);
  if (strcmp("ziptotmp",name)==0){
    res=az_extract(args[0], dpath);
  }
  else if (strcmp("restotmp",name)==0){
    char zpath[256];
    snprintf(zpath,256,"%s/%s",AROMA_DIR,args[0]);
    res=az_extract(zpath, dpath);
  }
  
  //-- Release Arguments
  _FREEARGS();
  
  //-- Return
  if (res) return StringValue(strdup("1"));
  return StringValue(strdup(""));
}

//* 
//* file_getprop, prop
//* 
Value* AROMA_FILEGETPROP(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc!=2) {
    return ErrorAbort(state, "%s() expects 2 args (path, key), got %d", name, argc);
  }
  
  //-- This is Busy Function
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();

  //-- Parse The Prop
  char* result;
  if (strcmp(name,"file_getprop")==0)
    result = aui_parseprop(args[0],args[1]);
  else if (strcmp(name,"prop")==0){
    char path[256];
    snprintf(path,256,"%s/%s",AROMA_TMP,args[0]);
    result = aui_parseprop(path,args[1]);
  }
  else if (strcmp(name,"zipprop")==0){
    result = aui_parsepropzip(args[0],args[1]);
  }
  else if (strcmp(name,"resprop")==0){
    char path[256];
    snprintf(path,256,"%s/%s",AROMA_DIR,args[0]);
    result = aui_parsepropzip(path,args[1]);
  }
  
  if (result == NULL) result = strdup("");

  //-- Release Arguments
  _FREEARGS();
  
  //-- Return
  return StringValue(result);
}

//* 
//* sysprop, property_get
//* 
Value* AROMA_RECOVERYPROP(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc!=1) {
    return ErrorAbort(state, "%s() expects 1 args (key), got %d", name, argc);
  }
  
  //-- This is Busy Function
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();

  //-- Parse The Prop
  char* result = aui_parseprop("/default.prop",args[0]);
  if (result == NULL) result = strdup("");

  //-- Release Arguments
  _FREEARGS();
  
  //-- Return
  return StringValue(result);
}

//* 
//* zipread, readfile
//* 
Value* AROMA_ZIPREAD(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc!=1) {
    return ErrorAbort(state, "%s() expects 1 args (zip entry path), got %d", name, argc);
  }
  
  //-- This is Busy Function
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  //-- Read From Zip
  char * buf = aui_readfromzip(args[0]);
  
  //-- Release Arguments
  _FREEARGS();
  
  //-- Return
  if (buf!=NULL) return StringValue(buf);
  return StringValue(strdup(""));
}

//* 
//* resread, readfile_aroma
//* 
Value* AROMA_RESREAD(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc!=1) {
    return ErrorAbort(state, "%s() expects 1 args (zip entry path in aroma dir), got %d", name, argc);
  }
  
  //-- This is Busy Function
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
    
  //-- Create Path Into Resource Dir
  char path[256];
  snprintf(path,256,"%s/%s",AROMA_DIR,args[0]);
  
  //-- Read From Zip
  char * buf = aui_readfromzip(path);
  
  //-- Release Arguments
  _FREEARGS();
  
  //-- Return
  if (buf!=NULL) return StringValue(buf);
  return StringValue(strdup(""));
}


//* 
//* pleasewait
//* 
Value* AROMA_PLEASEWAIT(const char* name, State* state, int argc, Expr* argv[]) {
  int func_pos = argv[0]->start; 
  if (func_pos<aparse_startpos)
    return StringValue(strdup(""));
  
  if (argc!=1) {
    return ErrorAbort(state, "%s() expects 1 args (wait text), got %d", name, argc);
  }
  
  //-- Get Arguments
  _INITARGS();
  
  //-- Set Busy Text
  char txt[32];
  snprintf(txt,32,"%s",args[0]);
  ag_setbusy_withtext(txt);
  
  //-- Release Arguments
  _FREEARGS();
  
  //-- Return
  return StringValue(strdup(""));
}

//* 
//* writetmpfile, write
//* 
Value* AROMA_WRITEFILE(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc!=2) {
    return ErrorAbort(state, "%s() expects 2 args (filename, value), got %d", name, argc);
  }
  
  //-- This is Busy Function
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  if (strcmp(name,"writetmpfile")==0){
    //-- Write String Into TMP File
    aui_writetotmp(args[0],args[1]);
  }
  else if (strcmp(name,"write")==0){
    //-- Write String Into Filesystem
    aui_writetofs(args[0],args[1]);
  }
  
  //-- Release Arguments
  _FREEARGS();
  
  //-- Return
  return StringValue(strdup(""));
}

//* 
//* readtmpfile, read
//* 
Value* AROMA_GETFILE(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc!=1) {
    return ErrorAbort(state, "%s() expects 1 args (filename), got %d", name, argc);
  }
  
  //-- This is Busy Function
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  char * result = NULL;
  if (strcmp(name,"readtmpfile")==0){
    //-- Read String from TMP File
    result = aui_readfromtmp(args[0]);
  }
  else if (strcmp(name,"read")==0){
    //-- Read String from Filesystem
    result = aui_readfromfs(args[0]);
  }
  
  //-- Release Arguments
  _FREEARGS();
  
  //-- Return
  if (result!=NULL) return StringValue(result);
  return StringValue(strdup(""));
}

//* 
//* getvar
//*
Value* AROMA_GETVAR(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc!=1) {
    return ErrorAbort(state, "%s() expects 1 args (variable name), got %d", name, argc);
  }
  
  //-- This is Busy Function
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
    
  //-- Get Result
  char * result = aui_getvar(args[0]);
  
  //-- Release Arguments
  _FREEARGS();
  
  //-- Return
  if (result!=NULL) return StringValue(result);
  return StringValue(strdup(""));
}

//* 
//* setvar, appendvar, prependvar
//*
Value* AROMA_SAVEVAR(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc!=2) {
    return ErrorAbort(state, "%s() expects 2 args (variable name, value), got %d", name, argc);
  }
  
  //-- This is Busy Function
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  //-- Save Variable
  if (strcmp(name,"setvar")==0){
    //-- setvar
    aui_setvar(args[0],args[1]);
  }
  else if (strcmp(name,"appendvar")==0){
    //-- appendvar
    aui_appendvar(args[0],args[1]);
  }
  else if (strcmp(name,"prependvar")==0){
    //-- prependvar
    aui_prependvar(args[0],args[1]);
  }
  
  //-- Release Arguments
  _FREEARGS();
  
  //-- Return
  return StringValue(strdup(""));
}

//* 
//* cmp
//*
Value* AROMA_CMP(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc!=3) {
    return ErrorAbort(state, "%s() expects 3 args (val1, logic, val2), got %d", name, argc);
  }
  
  //-- This is Busy Function
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  //-- Convert Arguments
  byte ret = 0;
  long val1 = atol(args[0]);
  long val2 = atol(args[2]);
  
  //-- Compare
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
  
  //-- Release Arguments
  _FREEARGS();
  
  //-- Return
  if (ret) return StringValue(strdup("1"));
  return StringValue(strdup(""));
}

//* 
//* cal
//*
Value* AROMA_CAL(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc!=3) {
    return ErrorAbort(state, "%s() expects 3 args (val1, operator, val2), got %d", name, argc);
  }
  
  //-- This is Busy Function
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  //-- Convert Arguments
  long ret = 0;
  long val1 = atol(args[0]);
  long val2 = atol(args[2]);
  
  //-- Calculating
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
  
  //-- Release Arguments
  _FREEARGS();
  
  //-- Return
  char retstr[64];
  snprintf(retstr,64,"%ld",ret);
 // StringValue(strdup(retstr));
 return StringValue(strdup(retstr));
}

//* 
//* iif
//*
Value* AROMA_IIF(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc!=3) {
    return ErrorAbort(state, "%s() expects 3 args (logic, trueval, falseval), got %d", name, argc);
  }
  
  //-- This is Busy Function
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  //-- Compare
  char * ret = NULL;
  if (args[0][0]=='\0')
    ret = strdup(args[2]);
  else
    ret = strdup(args[1]);
  
  //-- Release Arguments
  _FREEARGS();

  //-- Return
  return StringValue(ret);
}

//* 
//* calibrate
//*
Value* AROMA_CALIBRATE(const char* name, State* state, int argc, Expr* argv[]) {
/*
  if ((argc != 4)&&(argc != 5)) {
    return ErrorAbort(state, "%s() expects 4 or 5 args (div-x, add-x, div-y, add-y, usehack), got %d", name, argc);
  }
  
  //-- This is Busy Function
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  //-- Use Touch Screen Hack, for device without touch-up event
  if (argc==5){
    if (strcmp(args[4],"yes")==0)
      atouch_sethack(1);
    else
      atouch_sethack(0);
  }else
    atouch_sethack(0);
  
  //-- Set Calibration Data
  atouch_set_calibrate((float) strtof(args[0],NULL),atoi(args[1]),(float) strtof(args[2],NULL),atoi(args[3]));
  
  //-- Release Arguments
  _FREEARGS();
*/
  //-- Return
  return StringValue(strdup(""));
}

//* 
//* calibrate_matrix
//*
Value* AROMA_CALIBRATE_MATRIX(const char* name, State* state, int argc, Expr* argv[]) {
/*
  if (argc != 8) {
    return ErrorAbort(state, "%s() expects 8 args, got %d", name, argc);
  }
  
  //-- This is Busy Function
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  AW_CALIBMATRIX matrix;
	matrix.An = (float)strtof(args[0], NULL);
	matrix.Bn = (float)strtof(args[1], NULL);
	matrix.Cn = (float)strtof(args[2], NULL);
	matrix.Dn = (float)strtof(args[3], NULL);
	matrix.En = (float)strtof(args[4], NULL);
	matrix.Fn = (float)strtof(args[5], NULL);
	matrix.Divider = (float)strtof(args[6], NULL);
	byte usealt = (byte) atoi(args[7]);
	atouch_matrix_calibrate(&matrix);
	atouch_sethack(usealt);

  //-- Release Arguments
  _FREEARGS();
*/
  //-- Return
  return StringValue(strdup(""));
}

//* 
//* setcolor
//*
Value* AROMA_SETCOLOR(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc != 2) {
    return ErrorAbort(state, "%s() expects 2 args (color type, hexcolor in string), got %d", name, argc);
  }
  
  //-- This is Busy Function
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  //-- Convert String into Color
  color cl = strtocolor(args[1]);
  
  //-- Set Color Property
  if      (strcmp(args[0],"winbg") == 0)          acfg()->winbg=cl;
  else if (strcmp(args[0],"winbg_g") == 0)        acfg()->winbg_g=cl;
  else if (strcmp(args[0],"winfg") == 0)          acfg()->winfg=cl;
  else if (strcmp(args[0],"winfg_gray") == 0)     acfg()->winfg_gray=cl;
  else if (strcmp(args[0],"dialogbg") == 0)       acfg()->dialogbg=cl;
  else if (strcmp(args[0],"dialogbg_g") == 0)     acfg()->dialogbg_g=cl;
  else if (strcmp(args[0],"dialogfg") == 0)       acfg()->dialogfg=cl;
  else if (strcmp(args[0],"textbg") == 0)         acfg()->textbg=cl;
  else if (strcmp(args[0],"textfg") == 0)         acfg()->textfg=cl;
  else if (strcmp(args[0],"textfg_gray") == 0)    acfg()->textfg_gray=cl;
  else if (strcmp(args[0],"controlbg") == 0)      acfg()->controlbg=cl;
  else if (strcmp(args[0],"controlbg_g") == 0)    acfg()->controlbg_g=cl;
  else if (strcmp(args[0],"controlfg") == 0)      acfg()->controlfg=cl;
  else if (strcmp(args[0],"selectbg") == 0)       acfg()->selectbg=cl;
  else if (strcmp(args[0],"selectbg_g") == 0)     acfg()->selectbg_g=cl;
  else if (strcmp(args[0],"selectfg") == 0)       acfg()->selectfg=cl;
  else if (strcmp(args[0],"titlebg") == 0)        acfg()->titlebg=cl;
  else if (strcmp(args[0],"titlebg_g") == 0)      acfg()->titlebg_g=cl;
  else if (strcmp(args[0],"titlefg") == 0)        acfg()->titlefg=cl;
  else if (strcmp(args[0],"dlgtitlebg") == 0)     acfg()->dlgtitlebg=cl;
  else if (strcmp(args[0],"dlgtitlebg_g") == 0)   acfg()->dlgtitlebg_g=cl;
  else if (strcmp(args[0],"dlgtitlefg") == 0)     acfg()->dlgtitlefg=cl;
  else if (strcmp(args[0],"scrollbar") == 0)      acfg()->scrollbar=cl;
  else if (strcmp(args[0],"navbg") == 0)          acfg()->navbg=cl;
  else if (strcmp(args[0],"navbg_g") == 0)        acfg()->navbg_g=cl;
  else if (strcmp(args[0],"border") == 0)         acfg()->border=cl;
  else if (strcmp(args[0],"border_g") == 0)       acfg()->border_g=cl;
  else if (strcmp(args[0],"progressglow") == 0)   acfg()->progressglow=cl;
  
  //-- Background Should Be Redrawed
  aui_isbgredraw = 1;
  
  //-- Release Arguments
  _FREEARGS();

  //-- Return
  return StringValue(strdup(""));
}


//* 
//* ini_get
//*
Value* AROMA_INI_GET(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc != 1) {
    return ErrorAbort(state, "%s() expects 1 args (config name), got %d", name, argc);
  }
  
  //-- This is Busy Function
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  //-- Convert Arguments
  char retval[128];
  memset(retval,0,128);
  
  //-- Set Property
  if      (strcmp(args[0],"roundsize") == 0)          snprintf(retval,128,"%i",acfg()->roundsz);
  else if (strcmp(args[0],"button_roundsize") == 0)   snprintf(retval,128,"%i",acfg()->btnroundsz);
  else if (strcmp(args[0],"window_roundsize") == 0)   snprintf(retval,128,"%i",acfg()->winroundsz);
  else if (strcmp(args[0],"transition_frame") == 0)   snprintf(retval,128,"%i",acfg()->fadeframes);

  else if (strcmp(args[0],"text_ok") == 0)            snprintf(retval,128,"%s",acfg()->text_ok);
  else if (strcmp(args[0],"text_next") == 0)          snprintf(retval,128,"%s",acfg()->text_next);
  else if (strcmp(args[0],"text_back") == 0)          snprintf(retval,128,"%s",acfg()->text_back);

  else if (strcmp(args[0],"text_yes") == 0)           snprintf(retval,128,"%s",acfg()->text_yes);
  else if (strcmp(args[0],"text_no") == 0)            snprintf(retval,128,"%s",acfg()->text_no);
  else if (strcmp(args[0],"text_about") == 0)         snprintf(retval,128,"%s",acfg()->text_about);
  else if (strcmp(args[0],"text_calibrating") == 0)   snprintf(retval,128,"%s",acfg()->text_calibrating);
  else if (strcmp(args[0],"text_quit") == 0)          snprintf(retval,128,"%s",acfg()->text_quit);
  else if (strcmp(args[0],"text_quit_msg") == 0)      snprintf(retval,128,"%s",acfg()->text_quit_msg);
    
  else if (strcmp(args[0],"rom_name") == 0)           snprintf(retval,128,"%s",acfg()->rom_name);
  else if (strcmp(args[0],"rom_version") == 0)        snprintf(retval,128,"%s",acfg()->rom_version);
  else if (strcmp(args[0],"rom_author") == 0)         snprintf(retval,128,"%s",acfg()->rom_author);
  else if (strcmp(args[0],"rom_device") == 0)         snprintf(retval,128,"%s",acfg()->rom_device);
  else if (strcmp(args[0],"rom_date") == 0)           snprintf(retval,128,"%s",acfg()->rom_date);
  
  else if (strcmp(args[0],"customkeycode_up")==0)     snprintf(retval,128,"%i",acfg()->ckey_up);
  else if (strcmp(args[0],"customkeycode_down")==0)   snprintf(retval,128,"%i",acfg()->ckey_down);
  else if (strcmp(args[0],"customkeycode_select")==0) snprintf(retval,128,"%i",acfg()->ckey_select);
  else if (strcmp(args[0],"customkeycode_back") == 0) snprintf(retval,128,"%i",acfg()->ckey_back);
  else if (strcmp(args[0],"customkeycode_menu") == 0) snprintf(retval,128,"%i",acfg()->ckey_menu);
  else if (strcmp(args[0],"dp") == 0) snprintf(retval,128,"%i",agdp());
  
  //-- Release Arguments
  _FREEARGS();

  //-- Return
  return StringValue(strdup(retval));
}

//* 
//* ini_set
//*
Value* AROMA_INI_SET(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc != 2) {
    return ErrorAbort(state, "%s() expects 2 args (config name, config value in string), got %d", name, argc);
  }
  
  //-- This is Busy Function
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  //-- Convert Arguments
  byte valint = (byte) min(atoi(args[1]),255);
  int  valkey = (int) atoi(args[1]);
  
  //-- Set Property
  if      (strcmp(args[0],"roundsize") == 0)          acfg()->roundsz=valint;
  else if (strcmp(args[0],"button_roundsize") == 0)   acfg()->btnroundsz=valint;
  else if (strcmp(args[0],"window_roundsize") == 0)   acfg()->winroundsz=valint;
  else if (strcmp(args[0],"transition_frame") == 0)   acfg()->fadeframes=valint;

  else if (strcmp(args[0],"text_ok") == 0)            snprintf(acfg()->text_ok,64,"%s",args[1]);
  else if (strcmp(args[0],"text_next") == 0)          snprintf(acfg()->text_next,64,"%s",args[1]);
  else if (strcmp(args[0],"text_back") == 0)          snprintf(acfg()->text_back,64,"%s",args[1]);

  else if (strcmp(args[0],"text_yes") == 0)           snprintf(acfg()->text_yes,64,"%s",args[1]);
  else if (strcmp(args[0],"text_no") == 0)            snprintf(acfg()->text_no,64,"%s",args[1]);
  else if (strcmp(args[0],"text_about") == 0)         snprintf(acfg()->text_about,64,"%s",args[1]);
  else if (strcmp(args[0],"text_calibrating") == 0)   snprintf(acfg()->text_calibrating,64,"%s",args[1]);
  else if (strcmp(args[0],"text_quit") == 0)          snprintf(acfg()->text_quit,64,"%s",args[1]);
  else if (strcmp(args[0],"text_quit_msg") == 0)      snprintf(acfg()->text_quit_msg,128,"%s",args[1]);
    
  else if (strcmp(args[0],"rom_name") == 0)           snprintf(acfg()->rom_name,128,"%s",args[1]);
  else if (strcmp(args[0],"rom_version") == 0)        snprintf(acfg()->rom_version,128,"%s",args[1]);
  else if (strcmp(args[0],"rom_author") == 0)         snprintf(acfg()->rom_author,128,"%s",args[1]);
  else if (strcmp(args[0],"rom_device") == 0)         snprintf(acfg()->rom_device,128,"%s",args[1]);
  else if (strcmp(args[0],"rom_date") == 0)           snprintf(acfg()->rom_date,128,"%s",args[1]);
  
  
  else if (strcmp(args[0],"customkeycode_up")==0)     acfg()->ckey_up=valkey;
  else if (strcmp(args[0],"customkeycode_down")==0)   acfg()->ckey_down=valkey;
  else if (strcmp(args[0],"customkeycode_select")==0) acfg()->ckey_select=valkey;
  else if (strcmp(args[0],"customkeycode_back") == 0) acfg()->ckey_back=valkey;
  else if (strcmp(args[0],"customkeycode_menu") == 0) acfg()->ckey_menu=valkey;
    
  //-- Force Color Space  
  else if (strcmp(args[0],"force_colorspace") == 0){
    if (strcasecmp(args[1],"rgba")==0){
      ag_changecolorspace(0,8,16,24);
    }
    else if(strcasecmp(args[1],"abgr")==0){
      ag_changecolorspace(24,16,8,0);
    }
    else if(strcasecmp(args[1],"argb")==0){
      ag_changecolorspace(8,16,24,0);
    }
    else if(strcasecmp(args[1],"bgra")==0){
      ag_changecolorspace(16,8,0,24);
    }
  }
  else if (strcmp(args[0],"dp") == 0){
    set_agdp(valint);
  }
  
  
  //-- Background Should Be Redrawed
  aui_isbgredraw = 1;
  
  //-- Release Arguments
  _FREEARGS();

  //-- Return
  return StringValue(strdup(""));
}

//* 
//* anisplash
//*
Value* AROMA_ANISPLASH(const char* name, State* state, int argc, Expr* argv[]) {
  int func_pos = argv[0]->start; 
  if (func_pos<aparse_startpos) return StringValue(strdup(""));
  
  if (argc < 3) {
    return ErrorAbort(state, "%s() expects at least 2 args (loop count, [image name, duration]), got %d", name, argc);
  }
  else if (((argc-1)%2)!=0){
    return ErrorAbort(state, "%s() expects (1 + 2 * numframes) args (loop count, [image name, duration]), got %d", name, argc);
  }
  int frame_n= (argc-1)/2;
  if (frame_n>32){
    return ErrorAbort(state, "%s() Number of max frame was 32, got %s frames", name, frame_n);
  }
  
  //-- Set Busy before everythings ready
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  //-- Convert Arguments
  int loop_n = atoi(args[0]);
  
  //-- Set Temporary Background
  CANVAS tmpbg;
  ag_canvas(&tmpbg,agw(),agh());
  ag_draw(&tmpbg,agc(),0,0);
  
  //-- Create Splash BG
  CANVAS splashbg;
  ag_canvas(&splashbg,agw(),agh());
  ag_blur(&splashbg,agc(),agdp()*2);
    
  PNGCANVAS * ap = malloc(sizeof(PNGCANVAS) * frame_n);
  int       * ad = malloc(sizeof(int) * frame_n);
  byte      * au = malloc(sizeof(byte) * frame_n);
  
  //-- Load PNG
  int frame;
  for (frame=0;frame<frame_n;frame++){
    ad[frame] = atoi(args[(frame*2)+2]);
    if (apng_load(&(ap[frame]),args[(frame*2)+1]))
      au[frame] = 1;
    else
      au[frame] = 0;
  }
  
  byte firstime = 1;
  while (loop_n-->0){
    //-- Load PNG
    for (frame=0;frame<frame_n;frame++){
      //-- Draw BG
      ag_draw(NULL,&splashbg,0,0);
      
      //-- Draw Frame
      if (au[frame]){
        PNGCANVAS * p = &ap[frame];
        apng_draw(NULL,p,(agw()/2)-(p->w/2),(agh()/2)-(p->h/2));
      }
      
      //-- Wait The Fade Transition
      if (firstime){
        ag_sync_fade(acfg()->fadeframes);
        firstime=0;
      }
      else
        ag_sync();

      usleep(1000*ad[frame]);
    }
  }
  
  //-- Release Arguments
  _FREEARGS();
  
  //-- Release
  for (frame=0;frame<frame_n;frame++){
    if (au[frame]) apng_close(&ap[frame]);
  }
  free(ap); free(ad); free(au);
  
  //-- Redraw Previous Display
  ag_draw(NULL,&tmpbg,0,0);
  ag_sync_fade_wait(acfg()->fadeframes);
  
  //-- Cleanup
  ag_ccanvas(&splashbg);
  ag_ccanvas(&tmpbg);
  
  //-- Return
  return StringValue(strdup(""));
}

//* 
//* splash
//*
Value* AROMA_SPLASH(const char* name, State* state, int argc, Expr* argv[]) {
  int func_pos = argv[0]->start; 
  if (func_pos<aparse_startpos) return StringValue(strdup(""));
  
  if (argc != 2) {
    return ErrorAbort(state, "%s() expects 2 args (delay in milisecond, image name), got %d", name, argc);
  }
  
  //-- Set Busy before everythings ready
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  //-- Convert Arguments
  int delayint = atoi(args[0]);
  
  //-- Set Temporary Background
  CANVAS tmpbg;
  ag_canvas(&tmpbg,agw(),agh());
  ag_draw(&tmpbg,agc(),0,0);
  
  //-- Create Splash BG
  CANVAS splashbg;
  ag_canvas(&splashbg,agw(),agh());
  ag_blur(&splashbg,agc(),agdp()*2);
  
  //-- Load PNG
  PNGCANVAS ap;
  if (apng_load(&ap,args[1])){
    apng_draw(&splashbg,&ap,(agw()/2)-(ap.w/2),(agh()/2)-(ap.h/2));
    apng_close(&ap);
  }
  ag_draw(NULL,&splashbg,0,0);
  
  //-- Release Arguments
  _FREEARGS();
  
  //-- Wait The Fade Transition
  ag_sync_fade(acfg()->fadeframes);
  
  //-- Wait the splash timeout
  usleep(1000*delayint);
  
  //-- Redraw Previous Display
  ag_draw(NULL,&tmpbg,0,0);
  ag_sync_fade_wait(acfg()->fadeframes);
  
  //-- Cleanup
  ag_ccanvas(&splashbg);
  ag_ccanvas(&tmpbg);
  
  //-- Return
  return StringValue(strdup(""));
}

//* 
//* viewbox
//*
Value* AROMA_VIEWBOX(const char* name, State* state, int argc, Expr* argv[]) {
  _INITBACK();

  //-- is plain textbox or agreement
  byte isplain = (strcmp(name,"viewbox")==0)?1:0;
  if (isplain){
    if (argc!=3) return ErrorAbort(state, "%s() expects 3 args (title,desc,ico), got %d", name, argc);
  }
  else{
    if ((argc!=6)&&(argc!=5)&&(argc!=4)) return ErrorAbort(state, "%s() expects 4, 5 or 6 args (title,desc,ico,check_text [,initial_check,variablename]), got %d", name, argc);
  }
  
  //-- Set Busy before everythings ready
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  //-- Init Background
  aui_setbg(args[0]);
  char text[1024];
  snprintf(text,1024,"%s",args[1]);
  
  //-- Init Drawing Data
  int pad         = agdp() * 4;
  int chkW        = agw() - (pad*2);
  int bntH        = agdp() * 20;
  int chkH        = agh() - ( aui_minY + bntH + (pad*4));
  int chkY        = aui_minY + pad;
  int btnY        = chkY + chkH + (pad*2);
  
  //-- Draw Navigation Bar
  aui_drawnav(&aui_win_bg,0,btnY-pad,agw(),bntH+(pad*2));
    
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
  ag_textf(&aui_win_bg,chkW-((pad*2)+imgA),tifX+1,tifY+1,text,acfg()->winbg,0);
  ag_text(&aui_win_bg,chkW-((pad*2)+imgA),tifX,tifY,text,acfg()->winfg,0);
  
  //-- Draw Separator
  if (!isplain){
    color sepcl = ag_calculatealpha(acfg()->winbg,0x0000,80);
    color sepcb = ag_calculatealpha(acfg()->winbg,0xffff,127);
    ag_rect(&aui_win_bg,tifX,tifY+pad+txtH,chkW-((pad*2)+imgA),1,sepcl);
    ag_rect(&aui_win_bg,tifX,tifY+pad+txtH+1,chkW-((pad*2)+imgA),1,sepcb);
  }
  
  //-- Resize Checkbox Size & Pos
  chkY+=txtH+pad;
  chkH-=txtH+pad;
  
  //-- Create Window
  AWINDOWP hWin   = aw(&aui_win_bg);
  ACONTROLP txtcb = NULL;
  if (!isplain){
    byte initial_chk = 0;
    if (argc>4){
      if (atoi(args[4])!=0) initial_chk=1;
    }
    
    //-- Check Box
    int chkaH = agdp()*20;
    txtcb     = accb(hWin,tifX,tifY+(pad*2)+txtH,chkW-((pad*2)+imgA),chkaH+pad,args[3],initial_chk);
  }
  
  //-- BACK BUTTON
  if ((aparse_backpos>0)&&(aparse_backpos>aparse_installpos)){
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

  char save_var_name[256];
  if (argc==6){
    //-- Save Variable Name
    snprintf(save_var_name,256,"%s",args[5]);
  }
            
  //-- Release Arguments
  _FREEARGS();
  
  //-- Dispatch Message
  aw_show(hWin);
  aw_setfocus(hWin,nxtbtn);
  byte ondispatch     = 1;
  byte is_checked     = 0;
      
  while(ondispatch){
    dword msg=aw_dispatch(hWin);
    switch (aw_gm(msg)){
      case 6:{
        //-- NEXT Button
        if (!isplain){
          if (accb_ischecked(txtcb)){
            is_checked = 1;
            if (argc==6){
              //-- Save Into Variable
              aui_setvar(save_var_name,"1");
            }
          }
          else{
            is_checked = 0;
            if (argc==6){
              //-- Save Into Variable
              aui_setvar(save_var_name,"");
            }
          }
        }
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
  
  //-- Return
  if (aparse_isback) return NULL;
  _FINISHBACK();
  
  //-- Return Value
  if (is_checked) return StringValue(strdup("1"));
  return StringValue(strdup(""));
}

//* 
//* textbox, agreebox
//*
Value* AROMA_TEXTBOX(const char* name, State* state, int argc, Expr* argv[]) {
  _INITBACK();
  
  //-- is plain textbox or agreement
  byte isplain = (strcmp(name,"textbox")==0)?1:0;
  if ((isplain)&&(argc!=4)) {
    return ErrorAbort(state, "%s() expects 4 args (title,desc,ico,text), got %d", name, argc);
  }
  else if ((!isplain)&&(argc!=6)) {
    return ErrorAbort(state, "%s() expects 5 args (title,desc,ico,text,agreetext,unchkmessage), got %d", name, argc);
  }
  
  //-- Set Busy before everythings ready
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  //-- Init Background
  aui_setbg(args[0]);
  char text[256];
  snprintf(text,256,"%s",args[1]);
  
  //-- Unchecked Alert Message
  char unchkmsg[256];
  if (!isplain) snprintf(unchkmsg,256,"%s",args[5]);
    
  //-- Title Text
  char titletxt[64];
  snprintf(titletxt,64,"%s",args[0]);
  
  //-- Drawing Data
  int pad         = agdp() * 4;
  int chkW        = agw() - (pad*2);
  int bntH        = agdp() * 20;
  int chkH        = agh() - ( aui_minY + bntH + (pad*4));
  int chkY        = aui_minY + pad;
  int btnY        = chkY + chkH + (pad*2);
  
  //-- Draw Navigation Bar
  aui_drawnav(&aui_win_bg,0,btnY-pad,agw(),bntH+(pad*2));
    
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
  ag_textf(&aui_win_bg,chkW-((pad*2)+imgA),tifX+1,tifY+1,text,acfg()->winbg,0);
  ag_text(&aui_win_bg,chkW-((pad*2)+imgA),tifX,tifY,text,acfg()->winfg,0);
  
  //-- Resize Checkbox Size & Pos
  chkY+=txtH+pad;
  chkH-=txtH+pad;
  
  //-- Create Window
  AWINDOWP hWin   = aw(&aui_win_bg);
  
  //-- Create Controls
  ACONTROLP txtbox;
  ACONTROLP agreecb;  
  if (isplain){
    txtbox = actext(hWin,pad,chkY,chkW,chkH,args[3],0);
  }
  else{
    //-- Check Box
    int chkaH         = agdp()*20;
    int textBoxH      = chkH-(chkaH);
    txtbox            = actext(hWin,pad,chkY,chkW,textBoxH,args[3],0);
    agreecb           = accb(hWin,pad,chkY+textBoxH,chkW,chkaH+pad,args[4],0);
  }

  //-- BACK BUTTON
  if ((aparse_backpos>0)&&(aparse_backpos>aparse_installpos)){
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

  //-- Release Arguments
  _FREEARGS();
  
  //-- Dispatch Message
  aw_show(hWin);
  aw_setfocus(hWin,nxtbtn);
  byte ondispatch     = 1;
  while(ondispatch){
    dword msg=aw_dispatch(hWin);
    switch (aw_gm(msg)){
      case 6:{
        if (!isplain){
          if (!accb_ischecked(agreecb)){
            aw_alert(hWin,titletxt,unchkmsg,"@alert",acfg()->text_ok);
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
  
  //-- Destroy Window
  aw_destroy(hWin);
  
  //-- Finish
  if (aparse_isback) return NULL;
  _FINISHBACK();
  return StringValue(strdup(""));
}

//* 
//* checkbox
//*
Value* AROMA_CHECKBOX(const char* name, State* state, int argc, Expr* argv[]) {
  _INITBACK();
  if (argc<7){
    return ErrorAbort(state, "%s() expects more than 7 args, got %d", name, argc);
  }
  else if ((argc-4)%3!=0){
    return ErrorAbort(state, "%s() expects 4 args + 3 args per items, got %d", name, argc);
  }
  
  //-- Set Busy before everythings ready
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  //-- Variable Def
  int i;
  
  //-- Init Background
  aui_setbg(args[0]);
  
  //-- Init Strings
  char path[256];
  char text[256];
  snprintf(path,256,"%s/%s",AROMA_TMP,args[3]);
  snprintf(text,256,"%s",args[1]);
  
  //-- Drawing Data
  int pad         = agdp() * 4;
  int chkW        = agw() - (pad*2);
  int bntH        = agdp() * 20;
  int chkH        = agh() - ( aui_minY + bntH + (pad*4));
  int chkY        = aui_minY + pad;
  int btnY        = chkY + chkH + (pad*2);
  
  //-- Draw Navigation Bar
  aui_drawnav(&aui_win_bg,0,btnY-pad,agw(),bntH+(pad*2));
    
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
  ag_textf(&aui_win_bg,chkW-((pad*2)+imgA),tifX+1,tifY+1,text,acfg()->winbg,0);
  ag_text(&aui_win_bg,chkW-((pad*2)+imgA),tifX,tifY,text,acfg()->winfg,0);
  
  //-- Resize Checkbox Size & Pos
  chkY+=txtH+pad;
  chkH-=txtH+pad;
  
  //-- Create Window
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
  char propkey[64];
  int idx = 0;
  int group_id = 0;
  for (i=4;i<argc;i+=3) {
    byte defchk = (byte) atoi(args[i+2]);
    if (defchk==2){
      if (accheck_addgroup(chk1,args[i],args[i+1])){
        group_id++;
        idx=0;
      }
    }
    else if (defchk!=3){
      idx++;
      snprintf(propkey,64,"item.%d.%d",group_id,idx);
      char * res = aui_parseprop(path,propkey);
      if (res!=NULL){
        defchk = (strcmp(res,"1")==0)?1:0;
        free(res);
      }
      accheck_add(chk1,args[i],args[i+1],defchk);
    }
  }
  
  //-- Release Arguments
  _FREEARGS();
  
  //-- Dispatch Message
  aw_show(hWin);
  aw_setfocus(hWin,nxtbtn);
  byte ondispatch = 1;
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
  FILE * fp = fopen(path,"wb");
  if (fp!=NULL){
    int itemcnt = accheck_itemcount(chk1);
    for (i=0;i<itemcnt;i++) {
      if (!accheck_isgroup(chk1,i)){
        byte state = accheck_ischecked(chk1,i);
        snprintf(propkey,64,"item.%d.%d=%d\n",accheck_getgroup(chk1,i),accheck_getgroupid(chk1,i)+1,state);
        fwrite(propkey,1,strlen(propkey),fp);
      }
    }
    fclose(fp);
  }
  
  //-- Destroy Window
  aw_destroy(hWin);
  
  //-- Finish
  if (aparse_isback) return NULL;
  _FINISHBACK();
  return StringValue(strdup(""));
}

//* 
//* selectbox
//*
Value* AROMA_SELECTBOX(const char* name, State* state, int argc, Expr* argv[]) {
  _INITBACK();
  if (argc<7) {
    return ErrorAbort(state, "%s() expects more than 7 args, got %d", name, argc);
  }
  else if ((argc-4)%3!=0){
    return ErrorAbort(state, "%s() expects 4 args + 3 args per items, got %d", name, argc);
  }
  
  //-- Set Busy before everythings ready
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  //-- Variable Def
  int i;
  
  //-- Init Background
  aui_setbg(args[0]);
  
  //-- Init Strings
  char path[256];
  char text[256];
  snprintf(path,256,"%s/%s",AROMA_TMP,args[3]);
  snprintf(text,256,"%s",args[1]);
  
  //-- Drawing Data
  int pad         = agdp() * 4;
  int chkW        = agw() - (pad*2);
  int bntH        = agdp() * 20;
  int chkH        = agh() - ( aui_minY + bntH + (pad*4));
  int chkY        = aui_minY + pad;
  int btnY        = chkY + chkH + (pad*2);
  
  //-- Draw Navigation Bar
  aui_drawnav(&aui_win_bg,0,btnY-pad,agw(),bntH+(pad*2));
    
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
  ag_textf(&aui_win_bg,chkW-((pad*2)+imgA),tifX+1,tifY+1,text,acfg()->winbg,0);
  ag_text(&aui_win_bg,chkW-((pad*2)+imgA),tifX,tifY,text,acfg()->winfg,0);
  
  //-- Resize Checkbox Size & Pos
  chkY+=txtH+pad;
  chkH-=txtH+pad;
  
  //-- Create Window
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
  
  char propkey[64];
  
  //-- Populate Checkbox Items
  int group_id = 0;
  int idx      = 0;
  for (i=4;i<argc;i+=3) {
    byte defchk = (byte) atoi(args[i+2]);
    if (defchk==2){
      if (acopt_addgroup(opt1,args[i],args[i+1])){
        group_id++;
        idx      = 0;
      }
    }
    else if (defchk!=3){
      idx++;
      snprintf(propkey,64,"selected.%d",group_id);
      char * savedsel = aui_parseprop(path,propkey);
      
      snprintf(propkey,64,"%d",idx);
      if (savedsel!=NULL){
        defchk = (strcmp(savedsel,propkey)==0)?1:0;
        free(savedsel);
      }
      acopt_add(opt1,args[i],args[i+1],defchk);
    }
  }
  
  //-- Release Arguments
  _FREEARGS();
  
  //-- Dispatch Message
  aw_show(hWin);
  aw_setfocus(hWin,nxtbtn);
  byte ondispatch = 1;  
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
  FILE * fp = fopen(path,"wb");
  if (fp!=NULL){
    for (i=0;i<=group_id;i++){
      int selidx   = acopt_getselectedindex(opt1,i);
      if (selidx!=-1){
        int selindex = acopt_getgroupid(opt1,selidx)+1;
        snprintf(propkey,64,"selected.%d=%d\n",i,selindex);
        fwrite(propkey,1,strlen(propkey),fp);
      }
    }
    fclose(fp);
  }
  
  //-- Destroy Window
  aw_destroy(hWin);
  
  //-- Finish
  if (aparse_isback) return NULL;
  _FINISHBACK();
  return StringValue(strdup(""));
}

//* 
//* menubox
//*
Value* AROMA_MENUBOX(const char* name, State* state, int argc, Expr* argv[]) {
  _INITBACK();
  if (argc<7) {
    return ErrorAbort(state, "%s() expects more than 7 args, got %d", name, argc);
  }
  else if ((argc-4)%3!=0){
    return ErrorAbort(state, "%s() expects 4 args + 3 args per items, got %d", name, argc);
  }
  
  //-- Set Busy before everythings ready
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  //-- Variable Def
  int i;
  
  //-- Init Background
  aui_setbg(args[0]);
  
  //-- Init Strings
  char path[256];
  char text[256];
  snprintf(path,256,"%s/%s",AROMA_TMP,args[3]);
  snprintf(text,256,"%s",args[1]);
  
  //-- Drawing Data
  int pad         = agdp() * 4;
  int chkW        = agw() - (pad*2);
  int bntH        = agdp() * 20;
  int chkH        = agh() - ( aui_minY + bntH + (pad*4));
  int chkY        = aui_minY + pad;
  int btnY        = chkY + chkH + (pad*2);
  
  //-- Draw Navigation Bar
  aui_drawnav(&aui_win_bg,0,btnY-pad,agw(),bntH+(pad*2));
    
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
  ag_textf(&aui_win_bg,chkW-((pad*2)+imgA),tifX+1,tifY+1,text,acfg()->winbg,0);
  ag_text(&aui_win_bg,chkW-((pad*2)+imgA),tifX,tifY,text,acfg()->winfg,0);
  
  //-- Resize Checkbox Size & Pos
  chkY+=txtH+pad;
  chkH-=txtH+pad;
  
  //-- Create Window
  AWINDOWP hWin   = aw(&aui_win_bg);
  
  //-- Check Box
  ACONTROLP menu1  = acmenu(hWin,pad,chkY,chkW,chkH,6);
  ACONTROLP backbtn= NULL;
  if ((aparse_backpos>0)&&(aparse_backpos>aparse_installpos)){
    //-- BACK BUTTON
    backbtn=acbutton(
      hWin,
      pad,btnY,(chkW/2)-(agdp()*2),bntH,acfg()->text_back,0,
      5
    );
  }

  char propkey[64];
  //-- Populate Checkbox Items
  for (i=4;i<argc;i+=3) {
    if (strcmp(args[i],"")!=0)
      acmenu_add(menu1,args[i],args[i+1],args[i+2]);
  }

  //-- Release Arguments
  _FREEARGS();
  
  //-- Dispatch Message
  aw_show(hWin);
  if (backbtn!=NULL) aw_setfocus(hWin,backbtn);
  byte ondispatch = 1;  
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
  snprintf(propkey,64,"selected=%d\n",selindex);
  
  //-- Collecting Items:
  FILE * fp = fopen(path,"wb");
  if (fp!=NULL){
    fwrite(propkey,1,strlen(propkey),fp);
    fclose(fp);
    
  }
  
  //-- Destroy Window
  aw_destroy(hWin);
  
  //-- Finish
  if (aparse_isback) return NULL;
  _FINISHBACK();
  return StringValue(strdup(""));
}

//* 
//* install
//*
Value* AROMA_INSTALL(const char* name, State* state, int argc, Expr* argv[]) {
  _INITBACK();
  if ((argc!=3)&&(argc!=4)) {
    return ErrorAbort(state, "%s() expects 3 or 4 args (title,desc,ico,[finish_info]), got %d", name, argc);
  }
  
  //-- Set Busy before everythings ready
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  //-- Init Background
  aui_setbg(args[0]);
  
  //-- Init Strings
  char text[256];                   //-- Text When Installing
  char finish_text[256];            //-- Text After Installing
  snprintf(text,256,"%s",args[1]);
  if (argc==4)
    snprintf(finish_text,256,"%s",args[3]);
  else
    snprintf(finish_text,256,"%s",args[1]);
  
  //-- Drawing Data
  int pad         = agdp() * 4;
  int chkW        = agw() - (pad*2);
  int bntH        = agdp() * 20;
  int chkH        = agh() - ( aui_minY + bntH + (pad*4));
  int chkY        = aui_minY + pad;
  int btnY        = chkY + chkH + (pad*2);

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
  
  int txtFH       = ag_txtheight(chkW-((pad*2)+imgA),finish_text,0);
  int tifFY       = tifY;
  
  if (imgE){
    if (txtH<imgH){
      tifY+= (imgH-txtH)/2;
      txtH=imgH;
    }
    if (txtFH<imgH){
      tifFY+= (imgH-txtFH)/2;
      txtFH = imgH;
    }
    apng_draw_ex(&aui_win_bg,&ap,imgX,imgY,0,0,imgW,imgH);
    apng_close(&ap);
  }
  
  //-- Finished Text Canvas
  CANVAS cvf;
  ag_canvas(&cvf,agw(),((txtFH>txtH)?txtFH:txtH));
  ag_draw_ex(&cvf,&aui_win_bg,0,0,0,imgY,agw(),cvf.h);
  
  //-- Draw Finished Text
  ag_textf(&cvf, chkW-((pad*2)+imgA), tifX+1, tifFY+1-imgY, finish_text,    acfg()->winbg,0);
  ag_text (&cvf, chkW-((pad*2)+imgA), tifX,   tifFY-imgY,   finish_text,    acfg()->winfg,0);
  
  //-- Draw Text
  ag_textf(&aui_win_bg,chkW-((pad*2)+imgA),tifX+1,tifY+1,text,acfg()->winbg,0);
  ag_text(&aui_win_bg, chkW-((pad*2)+imgA),tifX,tifY,text,    acfg()->winfg,0);
  
  //-- Resize Checkbox Size & Pos
  int chkFY  = chkY + (txtFH+pad);
  int chkFH  = chkH - (txtFH+pad);
  
  chkY      += txtH+pad;
  chkH      -= txtH+pad;
  
  //-- Release Arguments
  _FREEARGS();
  
  //-- Start Installer Proccess
  int ret_status = aroma_start_install(
    &aui_win_bg,
    pad,chkY,chkW,chkH,
    pad,btnY,chkW,bntH,
    &cvf, imgY, chkFY, chkFH
  );
  
  //-- Release Finished Canvas
  ag_ccanvas(&cvf);

  //-- Set Installer already Runned
  aparse_installpos = func_pos;
  
  //-- Installer OK
  snprintf(text,256,"%i",ret_status);
  
  //-- Installer Not Return OK
  return StringValue(strdup(text));
}

//* 
//* calibtool
//*
Value* AROMA_CALIBTOOL(const char* name, State* state, int argc, Expr* argv[]) {
  /*
  int func_pos = argv[0]->start; 
  if (func_pos<aparse_startpos){
    return StringValue(strdup(""));
  }
  aw_calibtools(NULL);
  */
  return StringValue(strdup(""));
}

//* 
//* alert
//*
Value* AROMA_ALERT(const char* name, State* state, int argc, Expr* argv[]) {
  int func_pos = argv[0]->start; 
  if (func_pos<aparse_startpos){
    return StringValue(strdup(""));
  }
  if ((argc<2)||(argc>4)) {
    return ErrorAbort(state, "%s() expects 2-4 args (title, text, [icon, ok text]), got %d", name, argc);
  }
  
  //-- Set Busy before everythings ready
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  //-- Show Alert
  aw_alert(
    NULL,
    args[0],
    args[1],
    (argc>2)?args[2]:"",
    (argc>3)?args[3]:NULL
  );
  
  //-- Release Arguments
  _FREEARGS();
  
  //-- Return
  return StringValue(strdup(""));
}

//* 
//* confirm
//*
Value* AROMA_CONFIRM(const char* name, State* state, int argc, Expr* argv[]) {
  int func_pos = argv[0]->start; 
  if (func_pos<aparse_startpos){
    return StringValue(strdup(""));
  }
  if ((argc<2)||(argc>5)) {
    return ErrorAbort(state, "%s() expects 2-4 args (title, text, [icon, yes text, no text]), got %d", name, argc);
  }
  
  //-- Set Busy before everythings ready
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  //-- Show Confirm
  byte res = aw_confirm(
    NULL,
    args[0],
    args[1],
    (argc>2)?args[2]:"",
    (argc>3)?args[3]:NULL,
    (argc>4)?args[4]:NULL
  );
  
  //-- Release Arguments
  _FREEARGS();
  
  //-- Return
  if (res) return StringValue(strdup("yes"));
  return StringValue(strdup("no"));
}

//* 
//* textdialog
//*
Value* AROMA_TEXTDIALOG(const char* name, State* state, int argc, Expr* argv[]) {
  int func_pos = argv[0]->start; 
  if (func_pos<aparse_startpos){
    return StringValue(strdup(""));
  }
  if ((argc<2)||(argc>3)) {
    return ErrorAbort(state, "%s() expects 2-3 args (title, text [, ok text]), got %d", name, argc);
  }
  
  //-- Set Busy before everythings ready
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  //-- Show Text Dialog
  aw_textdialog(
    NULL,
    args[0],
    args[1],
    (argc>2)?args[2]:NULL
  );
  
  //-- Release Arguments
  _FREEARGS();
  
  //-- Return
  return StringValue(strdup(""));
}

//* 
//* exit
//*
Value* AROMA_EXIT(const char* name, State* state, int argc, Expr* argv[]) {
  fprintf(apipe(),"ui_print\n");
  fprintf(apipe(),"ui_print Exit Installer...\n");
  fprintf(apipe(),"ui_print\n");
  return NULL;
}

//* 
//* exit
//*
Value* AROMA_REBOOT(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc != 1) {
    return ErrorAbort(state, "%s() expects 1 arg", name);
  }  
  //-- Set Busy before everythings ready
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  //-- SET REBOOT
  if (strcmp(args[0],"now")==0){
    a_reboot(1);
    _FREEARGS();
    return NULL; //-- Terminate Immediately
  }
  else if (strcmp(args[0],"onfinish")==0){
    a_reboot(1);
  }
  else{
    a_reboot(0);
  }
  
  //-- Release Arguments
  _FREEARGS();
  
  return StringValue(strdup(""));
}

//* 
//* back
//*
Value* AROMA_BACK(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc != 1) {
    return ErrorAbort(state, "%s() expects 1 arg (number_of_back)", name);
  }
  //-- Set Busy before everythings ready
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  if ((aparse_backpos>0)&&(aparse_backpos>aparse_installpos)){
    int backsize = (byte) max(min(atoi(args[0]),255),1);
    int backpos  = aparse_history_pos - backsize;
    if (backpos<0) backpos = 0;
    int topos = aparse_history[backpos];
    
    //-- Not Allow Back before Installation Pos
    if (topos<=aparse_installpos){
      _FREEARGS();
      return StringValue(strdup(""));
    }
    
    //-- Set Back Position
    aparse_startpos = topos;
    aparse_backpos  = 0;
    aparse_isback   = 1;
    
    //-- Release Arguments
    _FREEARGS();
  }
  else{
    //-- Release Arguments
    _FREEARGS();
    return StringValue(strdup(""));
  }
  return NULL;
}

//* 
//* getdisksize, getdiskfree, getdiskusedpercent
//*
Value* AROMA_GETPART(const char* name, State* state, int argc, Expr* argv[]) {
  byte ispercent=0;
  if (strcmp(name,"getdiskusedpercent")==0){
    if (argc!=1)
      return ErrorAbort(state, "%s() expects 1 args (mountpoint), got %d", name, argc);
    ispercent=1;
  }
  else if ((argc!=1)&&(argc!=2)){
      return ErrorAbort(state, "%s() expects 1 or 2 args (mountpoint [, unit(b,k,m)]), got %d", name, argc);
  }
  char retstr[64];
  
  //-- Set Busy before everythings ready
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  //-- Get & Set mounted
  unsigned long ret = 0;
  byte valid=0;
  byte mtd = ismounted(args[0]);
  if (!mtd){
    alib_exec("/sbin/mount",args[0]);
    if (!ismounted(args[0])){
      _FREEARGS();
      goto done;
    }
  }
  
  int division = 1024*1024;
  //-- Set UNIT
  if ((ispercent==0)&&(argc==2)){
    if (args[1][0]=='k') division=1024;
    else if (args[1][0]=='m') division=1024*1024;
    else if (args[1][0]=='b') division=1;
  }
  
  //-- Calculating
  if (ispercent){
    int pret = alib_diskusage(args[0]);
    if (pret>=0){
      valid   = 1;
      ret     = pret;
    }
  }
  else if (strcmp(name,"getdisksize")==0){
    if (alib_disksize(args[0],&ret,division)) valid=1;
  }
  else{
    if (alib_diskfree(args[0],&ret,division)) valid=1;
  }
  
  
  
  //-- Unmount if previous was unmounted
  if (!mtd){
    alib_exec("/sbin/umount",args[0]);
  }

  //-- Release Arguments
  _FREEARGS();
  
done:
  //-- Finish
  if (valid){
    snprintf(retstr,64,"%lu",ret);  
  }
  else{
    snprintf(retstr,64,"-1");
  }
  return StringValue(strdup(retstr));
}

//* 
//* exit
//*
Value* AROMA_EXEC(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc < 1) {
    return ErrorAbort(state, "%s() expects at least 1 arg", name);
  }  
  //-- Set Busy before everythings ready
  ag_setbusy();
  
  int exec_status=-1;
  char status_str[16];
  snprintf(status_str,16,"-1");
  
  //-- Get Arguments
  _INITARGS();

  //-- Init Executable
  char path[256];
  byte isremoveexec = 0;
  if (strcmp(name,"zipexec")==0){
    isremoveexec    = 1;
    snprintf(path,256,"%s/exec_tmp",AROMA_TMP);
    int res = az_extract(args[0],path);
    if (res==0){
      _FREEARGS();
      goto done;
    }
  }
  else if (strcmp(name,"resexec")==0){
    char zpath[256];
    isremoveexec    = 1;
    snprintf(path, 256,"%s/exec_tmp",AROMA_TMP);
    snprintf(zpath,256,"%s/%s",AROMA_DIR,args[0]);
    int res = az_extract(zpath,path);
    if (res==0){
      _FREEARGS();
      goto done;
    }
  }
  else{
    snprintf(path, 256,"%s",args[0]);
  }
  
  //-- Init Exec CMD & Arguments
  int i         = 0;
  char** args2  = malloc(sizeof(char*) * (argc+1));
  args2[0]      = path;
  for (i=1;i<argc;i++) args2[i]=args[i];
  args2[argc]   = NULL;
  
  //-- Init PIPE
  int pipefd[2];
  pipe(pipefd);
  
  //-- FORK & RUN
  pid_t pid = fork();
  if (pid == 0) {
    setenv("UPDATE_PACKAGE", getArgv(1), 1);
    setenv("AROMA_TMP", AROMA_TMP, 1);
    setenv("AROMA_VERSION", AROMA_VERSION, 1);
    setenv("AROMA_BUILD", AROMA_BUILD, 1);
    setenv("AROMA_BUILD_CN", AROMA_BUILD_CN, 1);
    setenv("AROMA_NAME", AROMA_NAME, 1);
    setenv("AROMA_COPY", AROMA_COPY, 1);
    
    dup2(pipefd[1],STDOUT_FILENO);
    dup2(pipefd[1],STDERR_FILENO);
      
    close(pipefd[0]);
    execv(args2[0], args2);
    _exit(-1);
  }
  close(pipefd[1]);
  
  //-- BUFFER INTO VAR
  aui_setvar("exec_buffer","");
  char  buf[1024];
  FILE* fc = fdopen(pipefd[0], "r");
  while (fgets(buf,sizeof(buf),fc)!=NULL){
    aui_appendvar("exec_buffer",buf);
  }
  fclose(fc);
  
  //-- Get Return Status
  waitpid(pid, &exec_status, 0);
  snprintf(status_str,16,"%i",WEXITSTATUS(exec_status));
  free(args2);

  if (isremoveexec){
    unlink(path);
  }
  
  //-- Release Arguments
  _FREEARGS();
  
done:
  //-- Return
  return StringValue(strdup(status_str));
}


//* 
//* loadlang
//*
void aui_langloadsave(char * dest, int max, char * key){
  char * val = alang_get(key);
  if (val!=NULL) snprintf(dest,max,"%s",val);
}
Value* AROMA_LOADLANG(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc != 1) {
    return ErrorAbort(state, "%s() expects 1 arg (language_file)", name);
  }
  //-- Set Busy before everythings ready
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  //-- Load Language Data
  char path[256];
  snprintf(path,256,"%s/%s",AROMA_DIR,args[0]);
  byte res = alang_load(path);
  
  //-- Replace Text
  if (res){
    acfg_reset_text();
    aui_langloadsave(acfg()->text_ok, 64, "text_ok");
    aui_langloadsave(acfg()->text_next, 64, "text_next");
    aui_langloadsave(acfg()->text_back, 64, "text_back");
    aui_langloadsave(acfg()->text_yes, 64, "text_yes");
    aui_langloadsave(acfg()->text_no, 64, "text_no");
    aui_langloadsave(acfg()->text_about, 64, "text_about");
    aui_langloadsave(acfg()->text_calibrating, 64, "text_calibrating");
    aui_langloadsave(acfg()->text_quit, 64, "text_quit");
    aui_langloadsave(acfg()->text_quit_msg, 128, "text_quit_msg");
  }
  
  _FREEARGS();
  
  return StringValue(strdup(res?"1":""));
}

//* 
//* lang
//*
Value* AROMA_LANG(const char* name, State* state, int argc, Expr* argv[]) {
  if (argc != 1) {
    return ErrorAbort(state, "%s() expects 1 arg (words_key)", name);
  }
  
  //-- Set Busy before everythings ready
  ag_setbusy();
  
  //-- Get Arguments
  _INITARGS();
  
  char * out = alang_get(args[0]);
  
  _FREEARGS();
  
  return StringValue(strdup((out==NULL)?"":out));
}

/************************************[ AROMA EDIFY REGISTER ]************************************/

//* 
//* Register AROMA edify functions
//*
void RegisterAroma() {
  //-- CONFIG FUNCTIONS
  RegisterFunction("setcolor",      AROMA_SETCOLOR);      //-- SET AROMA COLORSET
  RegisterFunction("ini_set",       AROMA_INI_SET);       //-- SET INI CONFIGURATION
  RegisterFunction("ini_get",       AROMA_INI_GET);       //-- SET INI CONFIGURATION
  RegisterFunction("calibrate",     AROMA_CALIBRATE);     //-- SET CALIBRATION DATA
  RegisterFunction("calibrate_matrix",     AROMA_CALIBRATE_MATRIX); //-- SET CALIBRATION MATRIX
  RegisterFunction("calibtool",     AROMA_CALIBTOOL);     //-- SHOW CALIBRATING TOOL
  
  //-- SET THEME
  RegisterFunction("theme",         AROMA_THEME);         //-- SHOW CALIBRATING TOOL
  RegisterFunction("fontload",      AROMA_FONT);          //-- SHOW CALIBRATING TOOL
  RegisterFunction("fontresload",   AROMA_FONT);          //-- SHOW CALIBRATING TOOL
  
  //-- LANGUAGE FUNCTIONS
  RegisterFunction("loadlang",      AROMA_LOADLANG);       //-- Load Language File
  RegisterFunction("lang",          AROMA_LANG);           //-- Get Language Words
  
  //-- VARIABLE FUNCTIONS
  RegisterFunction("getvar",        AROMA_GETVAR);        //-- GET VARIABLE
  RegisterFunction("setvar",        AROMA_SAVEVAR);       //-- SET VARIABLE
  RegisterFunction("appendvar",     AROMA_SAVEVAR);       //-- APPEND STRING INTO VARIABLE
  RegisterFunction("prependvar",    AROMA_SAVEVAR);       //-- PREPEND STRING INTO VARIABLE
  
  //-- PROP FUNCTIONS
  RegisterFunction("file_getprop",  AROMA_FILEGETPROP);   //-- GET PROP
  RegisterFunction("prop",          AROMA_FILEGETPROP);   //-- GET PROP FROM AROMA TMP
  RegisterFunction("zipprop",       AROMA_FILEGETPROP);   //-- GET PROP FROM ZIP
  RegisterFunction("resprop",       AROMA_FILEGETPROP);   //-- GET PROP FROM AROMA RESOURCE ZIP
  RegisterFunction("sysprop",       AROMA_RECOVERYPROP);  //-- GET RECOVERY PROP
  RegisterFunction("property_get",  AROMA_RECOVERYPROP);  //-- GET RECOVERY PROP
  
  //-- FILE FUNCTIONS
  RegisterFunction("writetmpfile",  AROMA_WRITEFILE);     //-- WRITE STRING INTO TEMPORARY FILE
  RegisterFunction("write",         AROMA_WRITEFILE);     //-- WRITE STRING INTO FILESYSTEM
  RegisterFunction("readtmpfile",   AROMA_GETFILE);       //-- READ TEMPORARY FILE AS STRING
  RegisterFunction("read",          AROMA_GETFILE);       //-- READ FILESYSTEM AS STRING
  
  //-- ZIP HANDLING
  RegisterFunction("ziptotmp",      AROMA_EXTRACT);       //-- EXTRACT ZIP CONTENT INTO TMP
  RegisterFunction("restotmp",      AROMA_EXTRACT);       //-- EXTRACT RES CONTENT INTO TMP
  
  //-- ZIP CONTENT FUNCTIONS
  RegisterFunction("readfile",      AROMA_ZIPREAD);       //-- [Deprecated] - Renamed to zipread
  RegisterFunction("readfile_aroma",AROMA_RESREAD);       //-- [Deprecated] - Renamed to resread
  RegisterFunction("zipread",       AROMA_ZIPREAD);       //-- Read String From Zip
  RegisterFunction("resread",       AROMA_RESREAD);       //-- Read Strinf From Resource
  
  //-- EXEC
  RegisterFunction("zipexec",       AROMA_EXEC);          //-- Exec Program From Zip
  RegisterFunction("resexec",       AROMA_EXEC);          //-- Exec Program From Resource
  RegisterFunction("run_program",   AROMA_EXEC);          //-- Run Program/Exec
  RegisterFunction("exec",          AROMA_EXEC);          //-- Run Prohram/Exec

  //-- MAIN UI FUNCTIONS (With Next & Back Buttons)
  RegisterFunction("anisplash",     AROMA_ANISPLASH);     //-- SPLASH SCREEN
  RegisterFunction("splash",        AROMA_SPLASH);        //-- SPLASH SCREEN
  RegisterFunction("checkbox",      AROMA_CHECKBOX);      //-- CHECKBOX
  RegisterFunction("selectbox",     AROMA_SELECTBOX);     //-- SELECTBOX
  RegisterFunction("textbox",       AROMA_TEXTBOX);       //-- TEXTBOX
  RegisterFunction("viewbox",       AROMA_VIEWBOX);       //-- VIEWBOX
  RegisterFunction("checkviewbox",  AROMA_VIEWBOX);       //-- VIEWBOX
  RegisterFunction("agreebox",      AROMA_TEXTBOX);       //-- AGREEBOX
  RegisterFunction("menubox",       AROMA_MENUBOX);       //-- MENUBOX
  
  //-- INSTALL UI
  RegisterFunction("install",       AROMA_INSTALL);       //-- START INSTALLATION PROCCESS
  
  //-- DIALOG UI FUNCTIONS
  RegisterFunction("alert",         AROMA_ALERT);         //-- ALERT DIALOG
  RegisterFunction("textdialog",    AROMA_TEXTDIALOG);    //-- TEXT DIALOG
  RegisterFunction("confirm",       AROMA_CONFIRM);       //-- CONFIRM DIALOG
  
  //-- DISK INFO FUNCTIONS
  RegisterFunction("getdisksize",         AROMA_GETPART); //-- GET DISK SIZE
  RegisterFunction("getdiskfree",         AROMA_GETPART); //-- GET DISK FREE
  RegisterFunction("getdiskusedpercent",  AROMA_GETPART); //-- GET DISKUSAGE AS PERCENTAGE
  
  //-- COMPARISON & MATH
  RegisterFunction("cmp", AROMA_CMP);                     //-- COMPARE INTEGER
  RegisterFunction("cal", AROMA_CAL);                     //-- CALCULATE INTEGER
  RegisterFunction("iif", AROMA_IIF);                     //-- INLINE IF
  
  //-- ETC
  RegisterFunction("exit",          AROMA_EXIT);          //-- TERMINATE PROCCESS
  RegisterFunction("back",          AROMA_BACK);          //-- BACK TO PREVIOUS WIZARD
  RegisterFunction("pleasewait",    AROMA_PLEASEWAIT);    //-- SHOW WAIT SCREEN
  RegisterFunction("reboot",        AROMA_REBOOT);        //-- REBOOT DEVICE
  
}

/************************************[ START AND PARSE SCRIPT ]************************************/

//* 
//* AROMA PARSING & PROCCESSING SCRIPT
//*
byte aui_start(){
  //-- LOAD CONFIG SCRIPT
  AZMEM script_installer;
  if (!az_readmem(&script_installer,AROMA_CFG,0)) return 0;
  
  char * script_data = script_installer.data;
  if (script_installer.sz>3){
    //-- Check UTF-8 File Header
    if ((script_data[0]==0xEF)&&
        (script_data[1]==0xBB)&&
        (script_data[2]==0xBF)){
        script_data+=3;
        LOGS("aroma-config was UTF-8\n");
    }
  }
  
  //-- CLEANUP THEME:
  int i=0;
  for (i=0;i<AROMA_THEME_CNT;i++){
    acfg()->theme[i]=NULL;
    acfg()->theme_9p[i]=0;
  }
  
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
    //-- Init Config and Fonts
    acfg_init();
    ag_loadsmallfont("fonts/small",0,NULL);
    ag_loadbigfont("fonts/big",0,NULL);
    alang_release();
    
    aui_isbgredraw = 1;
    if (result!=NULL) free(result);
    aparse_history_pos  = 0;
    aparse_isback       = 0;
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
    
    alang_release();
    atheme_releaseall();
    return res;
  }
  else{
    free(script_installer.data);
    free(result);
  }
  
  alang_release();
  atheme_releaseall();
  return 1;
}
