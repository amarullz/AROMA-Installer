#include "aroma.h"
#include "libs/png.h"

/*********************************[ STRUCTRES ]********************************/
//-- READER STRUCTURE
typedef struct  {
	byte * data;
	int pos;
	int len;
} APNG_DATA;

/*********************************[ FUNCTIONS ]********************************/

//-- READER FUNCTION
void apng_readfn(png_structp pngPtr, png_bytep data, png_size_t length) {
  png_voidp a       = png_get_io_ptr(pngPtr);
  APNG_DATA * cpng  = (APNG_DATA *) a;
  if (cpng->pos+length>=cpng->len) length=cpng->len-cpng->pos;
  if (length){
    memcpy(data, cpng->data+cpng->pos, length);
    cpng->pos += length;
  }
}

//-- CLOSE
void apng_close(PNGCANVAS * pngcanvas){
  if (pngcanvas->r) free(pngcanvas->r);
  if (pngcanvas->g) free(pngcanvas->g);
  if (pngcanvas->b) free(pngcanvas->b);
  if (pngcanvas->a) free(pngcanvas->a);
}

//-- LOAD PNG FROM ZIP
byte apng_load(PNGCANVAS * pngcanvas,const char* imgname) {
  memset(pngcanvas,0,sizeof(PNGCANVAS));
  
  png_structp png_ptr   = NULL;
  png_infop info_ptr    = NULL;
  byte result           = 0;
  byte header[8];
  
  //-- LOAD DATA FROM ZIP
  char zpath[256];
  snprintf(zpath, sizeof(zpath)-1, "%s/%s.png",AROMA_DIR,imgname);
  AZMEM data_png;
  if (!az_readmem(&data_png,zpath,1)) return 0;
  
  //-- CREATE PNG ARGUMENT
  APNG_DATA          apng_data;
  apng_data.data   = data_png.data;
  apng_data.pos    = 0;
  apng_data.len    = data_png.sz;
  
  //-- HEADER
  memcpy(header,apng_data.data,sizeof(header));
  apng_data.pos+=sizeof(header);
  
  //-- COMPARE
  if (png_sig_cmp(header, 0, sizeof(header))) {
      goto exit;
  }
  
  //-- CREATE READ STRUCTURE
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) {
      goto exit;
  }
  
  //-- CREATE INFO STRUCTURE
  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
      goto exit;
  }

  //-- JMP
  if (setjmp(png_jmpbuf(png_ptr))) {
      goto exit;
  }
  
  //-- SET FUNCTION
  png_set_read_fn(png_ptr, &apng_data, apng_readfn);
  png_set_sig_bytes(png_ptr, sizeof(header));
  png_read_info(png_ptr, info_ptr);
  
  //-- Check Color Mode
  if (!(info_ptr->bit_depth == 8 &&
        ((info_ptr->channels == 3 && info_ptr->color_type == PNG_COLOR_TYPE_RGB) ||
         (info_ptr->channels == 4 && info_ptr->color_type == PNG_COLOR_TYPE_RGBA)))) {
      LOGE("PNG(%s): Not Supported. Only 8 Bit Depth with 3/4 Channel.\n",zpath);
      goto exit;
  }
  LOGI("PNG(%s): %ix%ix%i\n",zpath,info_ptr->width,info_ptr->height,info_ptr->channels);
  
  //-- Initializing Canvas
  pngcanvas->w    = info_ptr->width;
  pngcanvas->h    = info_ptr->height;
  pngcanvas->c    = info_ptr->channels;
  pngcanvas->s    = pngcanvas->w*pngcanvas->h;
  pngcanvas->r    = malloc(pngcanvas->s);
  pngcanvas->g    = malloc(pngcanvas->s);
  pngcanvas->b    = malloc(pngcanvas->s);
  if (pngcanvas->c==4)
    pngcanvas->a=malloc(pngcanvas->s);
  else
    pngcanvas->a= NULL;
  
  //-- READ ROWS
  int row_sz          = (int) png_get_rowbytes(png_ptr, info_ptr);
  png_bytep row_data  = (png_bytep) malloc(row_sz);
  
  int y;
  for (y=0; y<pngcanvas->h; ++y) {
    int x;
    png_read_row(png_ptr, row_data, NULL);
    for(x=pngcanvas->w-1;x>=0;x--) {
      //-- Get Row Data
      int sx = x * pngcanvas->c;
      int dx = y * pngcanvas->w + x;
      
      //-- SAVE RGB CHANNELS
      pngcanvas->r[dx] = row_data[sx];
      pngcanvas->g[dx] = row_data[sx+1];
      pngcanvas->b[dx] = row_data[sx+2];
      
      //-- SAVE ALPHA CHANNEL
      if (pngcanvas->c==4)
        pngcanvas->a[dx]=row_data[sx+3];
    }
  }
  free(row_data);
  result = 1;
exit:
  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);    
  free(data_png.data);
  return result;
}

//-- DRAW PNG INTO CANVAS
byte apng_draw(CANVAS * _b, PNGCANVAS * p, int xpos, int ypos){
  if (p==NULL) return 0;
  return apng_draw_ex(_b,p,xpos,ypos,0,0,p->w,p->h);
}
byte apng_draw_ex(CANVAS * _b, PNGCANVAS * p, int xpos, int ypos, int sxpos, int sypos,int sw, int sh){
  if (_b==NULL) _b=agc();
  if (p==NULL) return 0;
  if (p->s==0) return 0;
  
  //-- Quantizer Error Dithering Data Termporary
  int    qz  = p->s * 3;  
  byte * qe  = malloc(qz);
  memset(qe,0,qz);  
  
  //-- Drawing
  int x,y;
  for (y=sypos;(y<sh) && (y<p->h) && (y+ypos<_b->h);y++){
    for (x=sxpos;(x<sw) && (x<p->w) && (x+xpos<_b->w);x++){
      int sx = y * p->w + x;
      int qx = sx * 3;
      byte dr,dg,db;
      
      //-- Get Destination Color
      if (p->c==3){
        //-- NO ALPHA CHANNEL
        dr = p->r[sx];
        dg = p->g[sx];
        db = p->b[sx];
      }
      else{
        //-- Leave The Transparent
        if (p->a[sx]==0) continue;
        if (p->a[sx]==255){
          dr = p->r[sx];
          dg = p->g[sx];
          db = p->b[sx];
        }
        else{        
          //-- WITH ALPHA CHANNEL
          color * dstp = agxy(_b, x+xpos, y+ypos);
          if (dstp==NULL) continue;
          
          color dcolor = dstp[0]; //-- Destination Color
          byte  ralpha = 255 - p->a[sx];
          dr = (byte) (((((int) ag_r(dcolor)) * ralpha) + (((int) p->r[sx]) * p->a[sx])) >> 8);
          dg = (byte) (((((int) ag_g(dcolor)) * ralpha) + (((int) p->g[sx]) * p->a[sx])) >> 8);
          db = (byte) (((((int) ag_b(dcolor)) * ralpha) + (((int) p->b[sx]) * p->a[sx])) >> 8);
        }
      }
      
      //-- Dithering
      byte old_r = (byte) min(((int) dr) + ((int) qe[qx]),  255);
      byte old_g = (byte) min(((int) dg) + ((int) qe[qx+1]),255);
      byte old_b = (byte) min(((int) db) + ((int) qe[qx+2]),255);
      byte new_r = ag_close_r(old_r);
      byte new_g = ag_close_g(old_g);
      byte new_b = ag_close_b(old_b);
      byte err_r = old_r - new_r;
      byte err_g = old_g - new_g;
      byte err_b = old_b - new_b;
      
      // Save Green QE
      if (x<p->w-1) qe[qx+4] += err_g;
      if (y<p->h-1){
        qx = ((y+1) * p->w + x) * 3;
        
        // Save Red QE
        qe[qx] += err_r; 
        
        // Save Blue QE
        if (x<p->w-1) qe[qx+5] += err_b; 
      }
      ag_setpixel(_b,x+xpos,y+ypos, ag_rgb(new_r,new_g,new_b));      
    }
  }
  
  //LOGI("PNGDRAW: %ix%i on %ix%i\n",p->w,p->h,xpos,ypos);
  free(qe);
  return 1;
}

//-- LOAD PNG FONTS FROM ZIP
byte apng_loadfont(PNGFONTS * pngfont,const char* imgname) {
  png_structp png_ptr   = NULL;
  png_infop info_ptr    = NULL;
  byte result           = 0;
  byte header[8];
  
  //-- LOAD DATA FROM ZIP
  char zpath[256];
  snprintf(zpath, sizeof(zpath)-1, "%s/fonts/%s.png",AROMA_DIR,imgname);
  AZMEM data_png;
  if (!az_readmem(&data_png,zpath,1)) return 0;
  
  //-- CREATE PNG ARGUMENT
  APNG_DATA          apng_data;
  apng_data.data   = data_png.data;
  apng_data.pos    = 0;
  apng_data.len    = data_png.sz;
  
  //-- HEADER
  memcpy(header,apng_data.data,sizeof(header));
  apng_data.pos+=sizeof(header);
  
  //-- COMPARE
  if (png_sig_cmp(header, 0, sizeof(header))) {
      goto exit;
  }
  
  //-- CREATE READ STRUCTURE
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) {
      goto exit;
  }
  
  //-- CREATE INFO STRUCTURE
  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
      goto exit;
  }

  //-- JMP
  if (setjmp(png_jmpbuf(png_ptr))) {
      goto exit;
  }
  
  //-- SET FUNCTION
  png_set_read_fn(png_ptr, &apng_data, apng_readfn);
  png_set_sig_bytes(png_ptr, sizeof(header));
  png_read_info(png_ptr, info_ptr);
  
  //-- Check Color Mode
  if (!(info_ptr->bit_depth == 8 && (info_ptr->channels == 4 && info_ptr->color_type == PNG_COLOR_TYPE_RGBA))) {
      LOGE("PNG FONT(%s): Not Supported. Only 8 Bit Depth with 4 Channel.\n",zpath);
      goto exit;
  }
  LOGI("PNG(%s): %ix%ix%i\n",zpath,info_ptr->width,info_ptr->height,info_ptr->channels);
  
  //-- Initializing Canvas
  pngfont->w    = info_ptr->width;
  pngfont->h    = info_ptr->height;
  pngfont->c    = info_ptr->channels;
  pngfont->fh   = pngfont->h-1;
  pngfont->s    = pngfont->w*pngfont->fh;
  pngfont->d    = malloc(pngfont->s);
  
  //-- READ ROWS
  int row_sz          = (int) png_get_rowbytes(png_ptr, info_ptr);
  png_bytep row_data  = (png_bytep) malloc(row_sz);
  
  int y;
  int f_x   = 0;
  int f_w   = 0;
  int f_p   = 0;
  for (y=0; y<pngfont->h; ++y) {
    int x;
    png_read_row(png_ptr, row_data, NULL);
    for(x=0;x<pngfont->w;x++) {
      //-- Get Row Data
      int sx        = x * pngfont->c;
      byte alphaval = row_data[sx+3];
      if (y==0){
        if (alphaval==255){
          if (f_p<96){
            
              pngfont->fx[f_p] = x;
              pngfont->fw[f_p] = min(pngfont->w - x,255);
              if (f_p>0){
                pngfont->fw[f_p-1] = min(x - pngfont->fx[f_p-1],255);
                //LOGI("Font(%i): x = %i - w = %i\n",f_p-1,pngfont->fx[f_p-1],pngfont->fw[f_p-1]);
              }            
            f_p++;
          }
        }
      }
      else{
        int dx        = (y-1) * pngfont->w + x;
        pngfont->d[dx]= alphaval;
      }
    }
  }
  free(row_data);
  result = 1;
  pngfont->loaded=1;
exit:
  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);    
  free(data_png.data);
  return result;
}
//-- CLOSE
void apng_closefont(PNGFONTS * p){
  if (p->d) free(p->d);
  p->loaded=0;
}

//-- DRAW PNG FONT INTO CANVAS
byte apng_drawfont(CANVAS * _b, PNGFONTS * p, byte fpos, int xpos, int ypos, color cl,byte underline){
  if (_b==NULL) _b=agc();
  if (p==NULL) return 0;
  if (p->loaded==0) return 0;
  if (fpos>95) return 0;
  
  //-- Quantizer Error Dithering Data Termporary
  byte   fw  = p->fw[fpos];
  int    fx  = p->fx[fpos];
  int    fh  = p->fh;
  int    qz  = (fh * fw) * 3;  
  byte * qe  = malloc(qz);
  memset(qe,0,qz);  
  
  //-- Drawing
  int x,y;
  for (y=0;(y<p->fh) && (y+ypos<_b->h);y++){
    for (x=0;(x<fw) && (x+xpos<_b->w);x++){
      int sx = y * p->w + x + fx;
      int qx = (y * fw + x) * 3;
      byte a = p->d[sx];
      
      //-- Save Colors
      byte dr = ag_r(cl);
      byte dg = ag_g(cl);
      byte db = ag_b(cl);
  
      //-- Get Font Alpha
      if (a==0){
        //continue;
        color * dstp = agxy(_b, x+xpos, y+ypos);
        if (dstp==NULL) continue;
        color dcolor = dstp[0]; //-- Destination Color
        dr = ag_r(dcolor);
        dg = ag_g(dcolor);
        db = ag_b(dcolor);
      }
      else if (a!=255){
        //-- WITH ALPHA CHANNEL
        color * dstp = agxy(_b, x+xpos, y+ypos);
        if (dstp==NULL) continue;
        color dcolor = dstp[0]; //-- Destination Color
        byte  ralpha = 255 - a;
        dr = (byte) (((((int) ag_r(dcolor)) * ralpha) + (((int) dr) * a)) >> 8);
        dg = (byte) (((((int) ag_g(dcolor)) * ralpha) + (((int) dg) * a)) >> 8);
        db = (byte) (((((int) ag_b(dcolor)) * ralpha) + (((int) db) * a)) >> 8);
      }
      
      //-- Dithering
      byte old_r = (byte) min(((int) dr) + ((int) qe[qx]),  255);
      byte old_g = (byte) min(((int) dg) + ((int) qe[qx+1]),255);
      byte old_b = (byte) min(((int) db) + ((int) qe[qx+2]),255);
      byte new_r = ag_close_r(old_r);
      byte new_g = ag_close_g(old_g);
      byte new_b = ag_close_b(old_b);
      byte err_r = old_r - new_r;
      byte err_g = old_g - new_g;
      byte err_b = old_b - new_b;
      
      // Save Green QE
      if (x<fw-1) qe[qx+4] += err_g;
      if (y<fh-1){
        qx = ((y+1) * fw + x) * 3;
        // Save Red QE
        qe[qx] += err_r; 
        // Save Blue QE
        if (x<fw-1) qe[qx+5] += err_b; 
      }
      ag_setpixel(_b,x+xpos,y+ypos,ag_rgb(new_r,new_g,new_b));  
      if (underline){
        if (y==(p->fh-1)){
          ag_setpixel(_b,x+xpos,y+ypos,cl); 
        }
      }    
    }
  }
  free(qe);
  return 1;
}