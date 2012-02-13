#!/bin/bash

echo -en "\033]2;[Aroma Compiling]\007"

#path to the aroma installer directory
cd ~/ICSSGSinstaller/InstallerSource

if ! [ -d build ] ; then
	mkdir -p build
fi

if ! [ -d out ] ; then
	mkdir -p out
fi

cd build

#replace "/opt/toolchain/arm-2009q3/bin/arm-none-linux-gnueabi-gcc" with the correct path to "arm-none-linux-gnueabi-gcc"
/opt/toolchain/arm-2009q3/bin/arm-none-linux-gnueabi-gcc -save-temps -Os -static -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,-s -Werror ../src/*.c ../src/libs/*.c ../src/edify/*.c -o ../out/aroma -lm -lpthread

cd ..

if [ -e out/update-binary ] ; then
	rm out/update-binary
fi

mv out/aroma out/update-binary
cp out/update-binary examples/release/META-INF/com/google/android/update-binary
cp out/update-binary examples/generic/META-INF/com/google/android/update-binary

rm -rf build
