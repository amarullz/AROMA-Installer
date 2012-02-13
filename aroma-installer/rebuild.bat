@echo off

echo Rebuilding AROMA With Libs...
echo =============================
echo * Clean Up Compiled Object
cd obj
del /F /Q /S *.*
echo * Compiling
REM "D:\DevelTools\cndk\bin\arm-none-linux-gnueabi-gcc" = Change to arm-none-linux-gnueabi-gcc PATH
D:\DevelTools\cndk\bin\arm-none-linux-gnueabi-gcc -save-temps -Os -static -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,-s -Werror ../src/*.c ../src/libs/*.c ../src/edify/*.c -o ../build/aroma -lm -lpthread
echo * Clean Up AROMA
del aroma*
echo * Finished...

echo.
echo * Copying Binary Files...
cd ..
del build\update-binary
copy build\aroma build\update-binary
copy build\aroma examples\release\META-INF\com\google\android\update-binary
copy build\aroma examples\generic\META-INF\com\google\android\update-binary
echo.
echo =============================
echo CTRL-C = Exit, Enter = Deploy
pause
deploy.bat