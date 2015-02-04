#!/bin/bash
AROMA_LOCAL_PATH=$(dirname $(dirname $0))
AROMA_BINARY="out/aroma_installer"
cd $AROMA_LOCAL_PATH

#
# Copy Binary
#
if [ ! -x $AROMA_BINARY ]
then
  echo "0"
  exit
fi

rm -rf out/aroma.zip
rm -rf assets/META-INF/com/google/android/update-binary

cp $AROMA_BINARY assets/META-INF/com/google/android/update-binary
cd assets
zip -r9q ../out/aroma.zip .
cd ..
echo "1"
