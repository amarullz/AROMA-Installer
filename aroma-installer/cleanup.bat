@echo off

echo Cleanup
echo =======
echo * Obj
cd obj
del /F /Q /S *.*
cd ..

echo * Build
cd build
del /F /Q /S *.*
cd ..

echo * Examples
cd examples\release\META-INF\com\google\android\
del /F /Q update-binary
cd ..\..\..\..\..\..
cd examples\generic\META-INF\com\google\android\
del /F /Q update-binary
cd ..\..\..\..\..\..

echo ========
echo FINISHED
pause