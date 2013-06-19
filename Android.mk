LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
  AROMA_INSTALLER_LOCALPATH := $(LOCAL_PATH)
  LOCAL_SRC_FILES := 	\
    libs/zlib/adler32.c \
    libs/zlib/crc32.c \
    libs/zlib/infback.c \
    libs/zlib/inffast.c \
    libs/zlib/inflate.c \
    libs/zlib/inftrees.c \
    libs/zlib/zutil.c \
    libs/zlib/inflate_fast_copy_neon.s \
  \
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
    libs/png/png_read_filter_row_neon.s \
  \
    libs/minutf8/minutf8.c \
    libs/minzip/DirUtil.c \
    libs/minzip/Hash.c \
    libs/minzip/Inlines.c \
    libs/minzip/SysUtil.c \
    libs/minzip/Zip.c \
  \
    libs/freetype/autofit/autofit.c \
    libs/freetype/base/basepic.c \
    libs/freetype/base/ftapi.c \
    libs/freetype/base/ftbase.c \
    libs/freetype/base/ftbbox.c \
    libs/freetype/base/ftbitmap.c \
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
    libs/freetype/base/ftlcdfil.c \
    \
    src/edify/expr.c \
    src/edify/lex.yy.c \
    src/edify/parser.c \
    \
    src/controls/aroma_controls.c \
    src/controls/aroma_control_button.c \
    src/controls/aroma_control_check.c \
    src/controls/aroma_control_checkbox.c \
    src/controls/aroma_control_menubox.c \
    src/controls/aroma_control_optbox.c \
    src/controls/aroma_control_textbox.c \
    src/controls/aroma_control_threads.c \
    src/controls/aroma_control_imgbutton.c \
    \
    src/libs/aroma_array.c \
    src/libs/aroma_freetype.c \
    src/libs/aroma_graph.c \
    src/libs/aroma_input.c \
    src/libs/aroma_languages.c \
    src/libs/aroma_libs.c \
    src/libs/aroma_memory.c \
    src/libs/aroma_png.c \
    src/libs/aroma_zip.c \
    \
    src/main/aroma_ui.c \
    src/main/aroma_installer.c \
    src/main/aroma.c
  
  LOCAL_MODULE                  := aroma_installer
  LOCAL_MODULE_TAGS             := eng
  LOCAL_FORCE_STATIC_EXECUTABLE := true
  
  LOCAL_C_INCLUDES              := $(AROMA_INSTALLER_LOCALPATH)/include
  LOCAL_MODULE_PATH             := $(AROMA_INSTALLER_LOCALPATH)/out
  LOCAL_STATIC_LIBRARIES        := libm libc
  
  LOCAL_CFLAGS                  := -O2 
  LOCAL_CFLAGS                  += -DFT2_BUILD_LIBRARY=1 -DDARWIN_NO_CARBON 
  LOCAL_CFLAGS                  += -fdata-sections -ffunction-sections
  LOCAL_CFLAGS                  += -Wl,--gc-sections -fPIC -DPIC
  LOCAL_CFLAGS                  += -D_AROMA_NODEBUG
  
  #
  # Comment It, If You Don't Want To Use NEON
  #
  LOCAL_CFLAGS                  += -mfloat-abi=softfp -mfpu=neon -D__ARM_HAVE_NEON

include $(BUILD_EXECUTABLE)
    
    
include $(CLEAR_VARS)
LOCAL_MODULE        := aroma_installer.zip
LOCAL_MODULE_TAGS   := eng
ifeq ($(MAKECMDGOALS),aroma_installer.zip)
  $(info ==========================================================================)
  $(info )
  $(info MAKING AROMA Installer ZIP)
  OUTPUT_SH := $(shell $(AROMA_INSTALLER_LOCALPATH)/tools/android_building.sh)
  ifeq ($(OUTPUT_SH),0)
    $(info Please Compile AROMA Installer First, by running: make -j4 aroma_installer)
  else
    $(info AROMA ZIP is On $(AROMA_INSTALLER_LOCALPATH)/out/aroma.zip)
  endif
  $(info )
  $(info ==========================================================================)
endif

