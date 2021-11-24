#!/bin/bash

if [ "$#" .ne "2" ]; then
    echo "Usage: buildLinuxPackage.bash <Qt_folder> <unikorn_version>"
    echo "     > ./buildLinuxPackage.bash $HOME/Qt-5.12.2/5.15.2 1.0.0"
    exit 0
fi

export qt_folder=$1
export unikorn_version=$2
export output_folder="Unikorn-${unikorn_version}"

echo "unikorn version = ${unikorn_version}"
echo "output folder = ${output_folder}"
echo "qt_folder = ${qt_folder}"

# Create needed foldeers
mkdir ${output_folder}
mkdir ${output_folder}/inc
mkdir ${output_folder}/lib
mkdir ${output_folder}/bin
mkdir ${output_folder}/bin/platforms

# Copy the relevant files
cp -r ../examples ${output_folder}
cp -r ../ref ${output_folder}
cp ../inc/unikorn.h ${output_folder}/inc

# Build the unikorn library
cd ../lib
make clean
make RELEASE=Yes ALLOW_THREADS=Yes
cp libunikorn.a ../packaging/${output_folder}/lib
make clean
cd ../packaging

# Build the unikorn viewer
cd ../visualizer
qmake
make -j8
cp UnikornViewer ../packaging/${output_folder}/bin
make distclean
cd ../packaging
cp ${qt_folder}/gcc_64/plugins/platforms/libqxcb.so ${output_folder}/bin/platforms
cp ${qt_folder}/gcc_64/lib/libQt5Core.so.5 ${output_folder}/bin
cp ${qt_folder}/gcc_64/lib/libQt5Gui.so.5 ${output_folder}/bin
cp ${qt_folder}/gcc_64/lib/libQt5Widgets.so.5 ${output_folder}/bin

# Compress package
zip -r ${output_folder}-linux-x64.zip ${output_folder}

echo "Packaging created"

exit 0
