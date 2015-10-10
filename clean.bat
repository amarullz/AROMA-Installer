@echo off
echo CLEANUP AROMA INSTALLER
echo =======================
echo.
echo * Creating directories
mkdir obj
mkdir out
cd obj
echo * Cleanup objects
del /F /Q /S *.*
cd ..
echo * Cleanup binaries

cd out
del /F /Q *
cd ..\assets\META-INF\com\google\android\
del /F /Q update-binary
cd ..\..\..\..\..

echo.
pause