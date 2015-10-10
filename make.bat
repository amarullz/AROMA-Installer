@echo off
echo COMPILING AROMA INSTALLER
echo =========================
echo.
echo * Creating directories
mkdir obj
mkdir out
cd obj
echo.
echo * Compiling... This may take a moments...
echo.

D:\DevelTools\cndk\arm\bin\arm-none-linux-gnueabi-gcc ^
  -O2 -static -fdata-sections ^
  -ffunction-sections ^
  -Wl,--gc-sections ^
  -fPIC -DPIC ^
  -Wl,-s -Werror ^
  -D_GLIBCXX_DEBUG_PEDANTIC ^
  -D_GLIBCXX_DEBUG ^
  -D_AROMA_NODEBUG ^
  -DFT2_BUILD_LIBRARY=1 ^
  -DDARWIN_NO_CARBON ^
  -mfloat-abi=softfp -mfpu=neon ^
  -D__ARM_HAVE_NEON ^
    ^
      *.o ^
    ^
  	  ../src/libs/*.c ^
  	  ../src/controls/*.c ^
  	  ../src/main/*.c ^
  	^
  -I../include ^
  -I../src ^
  -o ../out/aroma_installer ^
  -lm -lpthread

echo.
cd ..

echo * Copying binary into release
echo.
copy out\aroma_installer assets\META-INF\com\google\android\update-binary
echo.

echo Press Enter to Build Release ZIP
pause
echo * Creating Release Zip
cd assets
..\tools\7z a -mx9 -tzip ..\out/aroma.zip .
cd ..


echo =============================
echo CTRL-C = Exit, Enter = Deploy
echo.
pause
echo.
deploy.bat