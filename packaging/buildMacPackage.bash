#!/bin/bash

if [ "$#" .ne "2" ]; then
    echo "Usage: buildMacPackage.bash <Qt_folder> <unikorn_version>"
    echo "     > ./buildMacPackage.bash $HOME/Qt/5.15.2 1.0.0"
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
cp -r UnikornViewer.app ../packaging/${output_folder}/bin
make distclean
cd ../packaging/${output_folder}/bin
${qt_folder}/clang_64/bin/macdeployqt UnikornViewer.app
cd ../..

# Compress package
zip -r ${output_folder}-mac.zip ${output_folder}

echo "Packaging created"

exit 0
