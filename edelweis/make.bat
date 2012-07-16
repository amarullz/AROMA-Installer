@echo off
echo COMPILING AROMA INSTALLER
echo =========================
echo.
echo * Creating directories
mkdir obj
mkdir bin
cd obj
echo.
echo * Compiling... This may take a moments...
echo.

D:\DevelTools\cndk\bin\arm-none-linux-gnueabi-gcc ^
  -Os -static -fdata-sections ^
  -ffunction-sections ^
  -Wl,--gc-sections ^
  -fPIC -DPIC ^
  -Wl,-s -Werror ^
  -D_AROMA_NODEBUG ^
  -D_GLIBCXX_DEBUG_PEDANTIC ^
  -D_GLIBCXX_DEBUG ^
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
  -o ../bin/update-binary ^
  -lm -lpthread

echo.
cd ..

echo * Copying binary into release
echo.
copy bin\update-binary release\META-INF\com\google\android\update-binary
echo.

echo =============================
echo CTRL-C = Exit, Enter = Deploy
echo.
pause
echo.
deploy.bat