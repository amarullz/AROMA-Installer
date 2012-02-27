@echo off
echo Deploying...
REM "D:\DevelTools\AndroidSDK16\" change to android sdk path
D:\DevelTools\AndroidSDK16\platform-tools\adb push build/update-binary /data/
D:\DevelTools\AndroidSDK16\platform-tools\adb shell chmod 777 /data/update-binary

echo STARTING
echo --------------------------------------------------------
echo.
D:\DevelTools\AndroidSDK16\platform-tools\adb shell /data/update-binary 1 0 /sdcard/aroma.zip
echo.
echo --------------------------------------------------------
echo CTRL-C = Save, Enter = Clean
pause
D:\DevelTools\AndroidSDK16\platform-tools\adb shell rm /data/update-binary