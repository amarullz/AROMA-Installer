@echo off
echo REBUILDING AROMA INSTALLER (DEBUG)
echo ==================================
echo.
echo * Creating directories
mkdir obj-debug
mkdir bin-debug
cd obj-debug
echo * Cleanup objects
del /F /Q /S *.*
echo.
echo * Compiling... This may take a moments...
echo.

D:\DevelTools\cndk\bin\arm-none-linux-gnueabi-gcc -g ^
  -save-temps ^
  -static ^
  -Wl,-s -Werror ^
  -D_GLIBCXX_DEBUG_PEDANTIC ^
  -D_GLIBCXX_DEBUG ^
  -DFT2_BUILD_LIBRARY=1 ^
  -DDARWIN_NO_CARBON ^
    ^
      ../libs/zlib/adler32.c ^
      ../libs/zlib/adler32_arm.c ^
      ../libs/zlib/crc32.c ^
      ../libs/zlib/infback.c ^
      ../libs/zlib/inffast.c ^
      ../libs/zlib/inflate.c ^
      ../libs/zlib/inftrees.c ^
      ../libs/zlib/zutil.c ^
      ../libs/png/png.c ^
      ../libs/png/pngerror.c ^
      ../libs/png/pnggccrd.c ^
      ../libs/png/pngget.c ^
      ../libs/png/pngmem.c ^
      ../libs/png/pngpread.c ^
      ../libs/png/pngread.c ^
      ../libs/png/pngrio.c ^
      ../libs/png/pngrtran.c ^
      ../libs/png/pngrutil.c ^
      ../libs/png/pngset.c ^
      ../libs/png/pngtrans.c ^
      ../libs/png/pngvcrd.c ^
      ../libs/minutf8/minutf8.c ^
      ../libs/minzip/DirUtil.c ^
      ../libs/minzip/Hash.c ^
      ../libs/minzip/Inlines.c ^
      ../libs/minzip/SysUtil.c ^
      ../libs/minzip/Zip.c ^
      ../libs/freetype/autofit/autofit.c ^
      ../libs/freetype/base/basepic.c ^
      ../libs/freetype/base/ftapi.c ^
      ../libs/freetype/base/ftbase.c ^
      ../libs/freetype/base/ftbbox.c ^
      ../libs/freetype/base/ftbitmap.c ^
      ../libs/freetype/base/ftdbgmem.c ^
      ../libs/freetype/base/ftdebug.c ^
      ../libs/freetype/base/ftglyph.c ^
      ../libs/freetype/base/ftinit.c ^
      ../libs/freetype/base/ftpic.c ^
      ../libs/freetype/base/ftstroke.c ^
      ../libs/freetype/base/ftsynth.c ^
      ../libs/freetype/base/ftsystem.c ^
      ../libs/freetype/cff/cff.c ^
      ../libs/freetype/pshinter/pshinter.c ^
      ../libs/freetype/psnames/psnames.c ^
      ../libs/freetype/raster/raster.c ^
      ../libs/freetype/sfnt/sfnt.c ^
      ../libs/freetype/smooth/smooth.c ^
      ../libs/freetype/truetype/truetype.c ^
    ^
  	  ../src/edify/*.c ^
  	  ../src/libs/*.c ^
  	  ../src/controls/*.c ^
  	  ../src/main/*.c ^
  	^
  -I../include ^
  -I../src ^
  -o ../bin-debug/update-binary ^
  -lm -lpthread

echo.
echo * Cleanup AROMA Installer objects
echo.
del aroma*

echo.
cd ..

echo =============================
echo CTRL-C = Exit, Enter = Deploy
echo.
pause
echo.
debug.bat