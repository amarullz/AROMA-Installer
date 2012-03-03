@echo off
echo CLEANUP AROMA INSTALLER (DEBUG)
echo ===============================
echo.
echo * Creating directories
mkdir obj-debug
mkdir bin-debug
cd obj-debug
echo * Cleanup objects
del /F /Q /S *.*
cd ..
echo * Cleanup binaries

cd bin-debug
del /F /Q update-binary

echo.
pause