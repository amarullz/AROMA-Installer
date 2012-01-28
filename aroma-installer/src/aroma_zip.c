#include "aroma.h"
#include "libs/Zip.h"

/*****************************[ GLOBAL VARIABLES ]*****************************/
ZipArchive zip;

/*********************************[ FUNCTIONS ]********************************/
//-- AROMA ZIP Init
byte az_init(const char * filename){
  if (mzOpenZipArchive(filename, &zip)!=0) return 0;
  mkdir(AROMA_TMP,755);
  return 1;
}

//-- AROMA ZIP Close
void az_close(){
  mzCloseZipArchive(&zip);
}

//-- Extract To Memory
byte az_readmem(AZMEM * out,const char * zpath, byte bytesafe){
  char z_path[256];
  snprintf(z_path, sizeof(z_path)-1, "%s", zpath);
  const ZipEntry* se = mzFindZipEntry(&zip, z_path);
  if (se == NULL) return 0;
  out->sz   = se->uncompLen+(bytesafe?0:1);
  out->data = malloc(out->sz);
  
  //memset(out->data,0,out->sz);
  if (!mzReadZipEntry(&zip, se, out->data, se->uncompLen)) {
      free(out->data);
      return 0;
  }
  if (!bytesafe) out->data[se->uncompLen] = '\0';
  return 1;
}

//-- Extract To File
byte az_extract(const char * zpath, const char * dest){
  const ZipEntry * zdata = mzFindZipEntry(&zip,zpath);
  if (zdata == NULL)
    return 0;
  
  unlink(dest);
  int fd = creat(dest, 0755);
  
  if (fd < 0) return 0;
  
  byte ok = mzExtractZipEntryToFile(&zip, zdata, fd);
  close(fd);
  return ok;
}