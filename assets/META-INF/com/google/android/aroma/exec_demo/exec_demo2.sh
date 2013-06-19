#!/sbin/sh
# #!/system/bin/sh

echo "<b>Evironment Variable Demo</b>"
echo "UPDATE_PACKAGE: <b>$UPDATE_PACKAGE</b>"
echo "AROMA_TMP: <b>$AROMA_TMP</b>"
echo "AROMA_NAME: <b>$AROMA_NAME</b>"
echo "AROMA_COPY: <b>$AROMA_COPY</b>"
echo "AROMA_VERSION: <b>$AROMA_VERSION</b>"
echo "AROMA_BUILD: <b>$AROMA_BUILD</b>"
echo "AROMA_BUILD_CN: <b>$AROMA_BUILD_CN</b>"
echo "PATH: <b>$PATH</b>"
echo " "

echo "<b>default.prop value</b>"
cat /default.prop


#-- Exit Code
exit 0