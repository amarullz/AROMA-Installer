@echo off

echo Compiling AROMA Without Rebuilding Libs...
echo ==========================================
echo * Compiling
REM "D:\DevelTools\cndk\bin\arm-none-linux-gnueabi-gcc" = Change to arm-none-linux-gnueabi-gcc PATH
D:\DevelTools\cndk\bin\arm-none-linux-gnueabi-gcc -Os -static -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,-s -Werror src/*.c obj/*.o -o build/aroma -lm -lpthread
echo * Finished...
echo.
echo * Copying Binary Files
del build\update-binary
copy build\aroma build\update-binary
copy build\aroma examples\release\META-INF\com\google\android\update-binary
copy build\aroma examples\generic\META-INF\com\google\android\update-binary
echo.
echo CTRL-C = Exit, Enter = Deploy
pause
deploy.bat