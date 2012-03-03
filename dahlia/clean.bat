@echo off
echo CLEANUP AROMA INSTALLER
echo =======================
echo.
echo * Creating directories
mkdir obj
mkdir bin
cd obj
echo * Cleanup objects
del /F /Q /S *.*
cd ..
echo * Cleanup binaries

cd bin
del /F /Q update-binary
cd ..\release\META-INF\com\google\android\

del /F /Q update-binary
cd ..\..\..\..\..

echo.
pause