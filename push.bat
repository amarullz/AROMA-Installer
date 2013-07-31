@echo off
echo Pushing...
D:\DevelTools\sdk\platform-tools\adb shell mount -a
D:\DevelTools\sdk\platform-tools\adb shell mkdir -p /sdcard/0/
echo Copiying zip file
D:\DevelTools\sdk\platform-tools\adb push out/aroma.zip /sdcard/0/aroma.zip
pause