#!/sbin/sh
# #!/system/bin/sh

echo "<b>Listing Root Directory:</b>"
ls /
echo " "

echo "<b>Running ls -l /</b>"
ls -l /
echo " "

echo "<b>Mounting /sdcard</b>"
mount -t auto /sdcard
echo " "

echo "<b>Listing /sdcard</b>"
ls /sdcard
echo " "

echo "<b>Mounting /system</b>"
mount -t auto /system
echo " "

echo "<b>Listing /system</b>"
ls /system
echo " "

echo "<b>Mounting /data</b>"
mount -t auto /data
echo " "

echo "<b>Listing /data</b>"
ls /data
echo " "

echo "<b>Mounting /sd-ext</b>"
mount -t auto /sd-ext
echo " "

echo "<b>Listing /sd-ext</b>"
ls /sd-ext
echo " "

echo "<b>Running df</b>"
df
echo " "

echo "<b>Running ps</b>"
ps
echo " "
echo " "

echo "<u>Shell Finished</u>"

#-- Exit Code
exit 10