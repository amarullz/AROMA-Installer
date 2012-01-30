@echo off
echo Deploying...
REM "D:\DevelTools\AndroidSDK16\" change to android sdk path
D:\DevelTools\AndroidSDK16\platform-tools\adb push build/aroma /data/
D:\DevelTools\AndroidSDK16\platform-tools\adb shell chmod 777 /data/aroma

echo STARTING
echo --------------------------------------------------------
echo.
D:\DevelTools\AndroidSDK16\platform-tools\adb shell /data/aroma 1 0 /data/aroma.zip
echo.
echo --------------------------------------------------------
echo CTRL-C = Save, Enter = Clean
pause
D:\DevelTools\AndroidSDK16\platform-tools\adb shell rm /data/aroma