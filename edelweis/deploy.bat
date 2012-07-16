@echo off
echo Deploying...
REM
REM "D:\DevelTools\AndroidSDK16\" change to android sdk path
REM

D:\DevelTools\AndroidSDK16\platform-tools\adb push bin/update-binary /tmp/
D:\DevelTools\AndroidSDK16\platform-tools\adb shell chmod 777 /tmp/update-binary

echo STARTING
echo --------------------------------------------------------
echo.
D:\DevelTools\AndroidSDK16\platform-tools\adb shell /tmp/update-binary 1 0 /sdcard/aroma.zip
echo.
echo --------------------------------------------------------
echo CTRL-C = Save, Enter = Clean
pause
D:\DevelTools\AndroidSDK16\platform-tools\adb shell rm /tmp/update-binary