#include <stdio.h>
#include <stdlib.h>

#ifndef _AROMA_NODEBUG
#include <sys/statvfs.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

  #define AROMA_MEM_TMPDIR "/tmp/aroma-memory"
  long aroma_isexist_mem(void * x){
    char pn[256];
    snprintf(pn,256,"%s/%i",AROMA_MEM_TMPDIR,(long) x);
    FILE * fp = fopen(pn, "r");
    if (fp){
      long sz=0;
      fread(&sz,1,sizeof(long),fp);
      fclose(fp);
      return sz;
    }
    return 0;
  }
  void aroma_touch_memaddr(void * x,long sz){
    long fz=aroma_isexist_mem(x);
    if (fz==0){
      char pn[256];
      snprintf(pn,256,"%s/%i",AROMA_MEM_TMPDIR,(long) x);
      FILE * fp=fopen(pn,"wb");
      fwrite(&sz,1,sizeof(long),fp);
      if (fp) fclose(fp);
    }
  }
  void aroma_unlink_memaddr(void * x){
    long fz=aroma_isexist_mem(x);
    if (fz){
      char pn[256];
      snprintf(pn,256,"%s/%i",AROMA_MEM_TMPDIR,(long) x);
      unlink(pn);
    }
  }
  void aroma_dump_malloc(){
     const char *path = AROMA_MEM_TMPDIR;
     DIR *d = opendir(path);
     size_t path_len = strlen(path);
     
     printf("\n\n===================================================\n");
     printf(    "|                   LEAK INFO:                    |\n");
     printf(    "===================================================\n\n");
     if (d)
     {
        struct dirent *p;
        while ((p=readdir(d)))
        {
            char *buf;
            size_t len;
            if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
            {
               continue;
            }
            len = path_len + strlen(p->d_name) + 2; 
            buf = malloc(len);
            if (buf)
            {
               struct stat statbuf;
               snprintf(buf, len, "%s/%s", path, p->d_name);
               if (!stat(buf, &statbuf))
               {
                  if (S_ISDIR(statbuf.st_mode)){
                  }
                  else
                  {
                    long memaddr = atoi(p->d_name);
                    long fz      = aroma_isexist_mem((void *) memaddr);
                    char str[10];
                    memset(str,0,10);
                    if (fz>0) snprintf(str,10,(char *) memaddr);
                    printf("   MEMORY LEAK: [0x%x](%i b) = \"%s\"\n",memaddr,fz,str);
                  }
               }
               free(buf);
            }
        }
        closedir(d);
     }
     printf("\n\n===================================================\n\n");
  }
#endif


static int aroma_parent_pid  = 0;
void aroma_memory_parentpid(int parent_pid){
  aroma_parent_pid=parent_pid;
}
void aroma_memory_terminate(const char * message){
  fprintf(stdout,"\n\naroma/FATAL-ERROR: %s\n\n",message);
  if (aroma_parent_pid) kill(aroma_parent_pid,18);
  exit(-1);
}
void *aroma_malloc(size_t size){
    void *ret = malloc(size);
    if (!ret && !size)
        ret = malloc(1);
    if (!ret) {
        if (!ret && !size) ret = malloc(1);
        if (!ret) aroma_memory_terminate("Out Of Memory...\n");
    }
#ifndef _AROMA_NODEBUG
  aroma_touch_memaddr(ret,size);
#endif
    return ret;
}

#ifndef _AROMA_NODEBUG
  void aroma_memory_debug_init(){
    create_directory(AROMA_MEM_TMPDIR,0777);
  }
  void aroma_free(void * x){
    aroma_unlink_memaddr(x);
    free(x);
  }
#endif