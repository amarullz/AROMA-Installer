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
 * AROMA Languages Handler
 *
 */

#include "../aroma.h"

AARRAYP alang = NULL;

//*
//* Release Loaded Language
//*
void alang_release(){
  if (alang!=NULL){
    aarray_free(alang);
    alang=NULL;
  }
}

//*
//* Get Lang Value
//*
char * alang_get(char * key){
  if (alang==NULL) return NULL;
  return aarray_get(alang,key);
}

//*
//* Parse AMS
//*
char * alang_ams(const char * str){
  char   c = 0;
  char  pc = 0;
  char * r = malloc(1); *r=0;
  int   rl = 0;
  byte  state=0;
  char key[256];
  int   kp = 0;
  byte tag_type = 0;
  
  while ((c=*str++)){
    if (state==0){
      if ((c=='<')&&(pc!='\\')&&((*str=='~')||(*str=='$'))){
        tag_type  = (*str=='~')?0:1;
        state     = 1;
        kp        = 0;
        key[0]    = 0;
      }
      else if ((c=='<')&&(pc=='\\')&&((*str=='~')||(*str=='$'))){
        r[rl-1] = c;
        r[rl]   = 0;
      }
      else{
        r = realloc(r, rl+2);
        r[rl++] = c;
        r[rl]   = 0;
      }
    }
    else if(state==1){
      if ((c!='>')&&(kp<255)){
        key[kp++] = c;
        key[kp]   = 0;
      }
      else if (tag_type==0){
        //-- Lang Tags
        state=0;
        char * lfound = alang_get(key+1);
        if (lfound!=NULL){
          int addsz = strlen(lfound);
          r = realloc(r, rl+addsz+1);
          char * rpos = r+rl;
          memcpy(rpos,lfound,addsz);
          rl+=addsz;
          r[rl] = 0;
        }
        else{
          int addsz = strlen(key+1);
          r = realloc(r, rl+addsz+1);
          char * rpos = r+rl;
          memcpy(rpos,key+1,addsz);
          rl+=addsz;
          r[rl] = 0;
        }
      }
      else{
        //-- Variable Tags
        state=0;
        char * lfound = aui_getvar(key+1);
        if (lfound!=NULL){
          int addsz = strlen(lfound);
          r = realloc(r, rl+addsz+1);
          char * rpos = r+rl;
          memcpy(rpos,lfound,addsz);
          rl+=addsz;
          r[rl] = 0;
          free(lfound);
        }
      }
    }
    pc = c;
  }
  return r;
}

//*
//* Load & Parse Language File
//*
byte alang_load(char * z){
  alang_release();
  alang       = aarray_create();
  char * buf  = aui_readfromzip(z);
  if (buf==NULL) return 0;
  char * vuf  = buf;
  if (strlen(vuf)>3){
    //-- Check UTF-8 File Header
    if ((vuf[0]==0xEF)&&(vuf[1]==0xBB)&&(vuf[2]==0xBF)){
        vuf+=3;
    }
  }
  byte state  = 0;
  byte slash  = 0;
  char c      = 0;
  char pc     = 0;
  char * key  = NULL;
  char * val  = NULL;
  
  while ((c=*vuf)){
    
    if (state==0){
      //-- First State
      if (!isspace(c)){
        key   = vuf;
        state = 2;
      }
      else if (c=='#')
        state = 1;
    }
    else if (state==1){
      //-- Comment
      if (c=='\n') state=0;
    }
    else if (state==2){
      if (isspace(c)||(c=='=')||(c=='\n')){
        *vuf = 0;
        if (c=='=') state=3;
        else if (c=='\n') state=0;
      }
    }
    else if (state==3){
      if (!isspace(c)){
        val = vuf;
        state=4;
        pc  = c;
      }
      else if (c=='\n') state=0;
    }
    else if (state==4){
      if (((c=='\n')&&(pc!='\\'))||(*(vuf+1)==0)){
        if ((c=='\n')&&(pc!='\\')) *vuf = 0;
          
        //-- Cleanup backslashes
        int i;
        int j=0;
        int l=strlen(val);
        for (i=0;i<l;i++){
          if ((val[i]=='\\')&&(val[i+1]=='\n')) continue;
          val[j++]=val[i];
        }
        val[j]=0;
        
        //-- Save Lang Value
        aarray_set(alang,key,val);
        
        //-- End Of String
        state = 0;
      }
      pc=c;
    }
    
    vuf++;
    
  }
  
  free(buf);
  return 1;
}