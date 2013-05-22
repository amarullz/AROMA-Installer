LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := aroma
LOCAL_SRC_FILES := 	\
  libs/png/png.c \
  libs/png/pngerror.c \
  libs/png/pnggccrd.c \
  libs/png/pngget.c \
  libs/png/pngmem.c \
  libs/png/pngpread.c \
  libs/png/pngread.c \
  libs/png/pngrio.c \
  libs/png/pngrtran.c \
  libs/png/pngrutil.c \
  libs/png/pngset.c \
  libs/png/pngtrans.c \
  libs/png/pngvcrd.c \
  libs/minutf8/minutf8.c \
  libs/minzip/DirUtil.c \
  libs/minzip/Hash.c \
  libs/minzip/Inlines.c \
  libs/minzip/SysUtil.c \
  libs/minzip/Zip.c \
  libs/freetype/autofit/autofit.c \
  libs/freetype/base/basepic.c \
  libs/freetype/base/ftapi.c \
  libs/freetype/base/ftbase.c \
  libs/freetype/base/ftbbox.c \
  libs/freetype/base/ftbitmap.c \
  libs/freetype/base/ftdbgmem.c \
  libs/freetype/base/ftdebug.c \
  libs/freetype/base/ftglyph.c \
  libs/freetype/base/ftinit.c \
  libs/freetype/base/ftpic.c \
  libs/freetype/base/ftstroke.c \
  libs/freetype/base/ftsynth.c \
  libs/freetype/base/ftsystem.c \
  libs/freetype/cff/cff.c \
  libs/freetype/pshinter/pshinter.c \
  libs/freetype/psnames/psnames.c \
  libs/freetype/raster/raster.c \
  libs/freetype/sfnt/sfnt.c \
  libs/freetype/smooth/smooth.c \
  libs/freetype/truetype/truetype.c \
  $(wildcard src/edify/*.c) \
  $(wildcard src/libs/*.c) \
  $(wildcard src/controls/*.c) \
  $(wildcard src/main/*.c)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_LDLIBS := -lz

LOCAL_CFLAGS := -Os -static -fdata-sections \
  -ffunction-sections -fno-short-enums \
  -Wl,--gc-sections \
  -fPIC -DPIC \
  -Wl,-s \
  -D_GLIBCXX_DEBUG_PEDANTIC \
  -D_GLIBCXX_DEBUG \
  -D_AROMA_NODEBUG \
  -DFT2_BUILD_LIBRARY=1 \
  -DDARWIN_NO_CARBON

deploy:
		adb push libs/armeabi/aroma /data/data/


include $(BUILD_EXECUTABLE)


  
