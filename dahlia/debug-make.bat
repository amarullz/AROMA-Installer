@echo off
echo COMPILING AROMA INSTALLER (DEBUG)
echo =================================
echo.
echo * Creating directories
mkdir obj-debug
mkdir bin-debug
cd obj-debug
echo.
echo * Compiling... This may take a moments...
echo.

D:\DevelTools\cndk\bin\arm-none-linux-gnueabi-gcc -g ^
  -static ^
  -Wl,-s -Werror ^
  -DFT2_BUILD_LIBRARY=1 ^
  -DDARWIN_NO_CARBON ^
    ^
      *.o ^
    ^
  	  ../src/libs/*.c ^
  	  ../src/controls/*.c ^
  	  ../src/main/*.c ^
  	^
  -I../include ^
  -I../src ^
  -o ../bin-debug/update-binary ^
  -lm -lpthread

echo.
cd ..

echo =============================
echo CTRL-C = Exit, Enter = Deploy
echo.
pause
echo.
debug.bat