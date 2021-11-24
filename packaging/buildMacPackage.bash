#!/bin/bash

if [ "$#" .ne "2" ]; then
    echo "USAGE: buildMacPackage.bash <Qt_folder> <unikorn_version>"
    echo "     > ./buildMacPackage.bash $HOME/Qt/5.15.2 1.0.0"
    exit 0
fi

export qt_folder=$1
export unikorn_version=$2
export output_folder="Unikorn-${unikorn_version}"

echo "unikorn version = ${unikorn_version}"
echo "output folder = ${output_folder}"
echo "qt_folder = ${qt_folder}"

[ ! -d ${qt_folder} ] && echo "ERROR: The folder '${qt_folder}' does not exist!" && exit 0

# Create needed foldeers
mkdir ${output_folder}
mkdir ${output_folder}/inc
mkdir ${output_folder}/bin

# Copy the relevant files
cp ../README.md ${output_folder}
cp ../LICENSE ${output_folder}
cp -r ../examples ${output_folder}
cp -r ../src ${output_folder}
cp -r ../inc ${output_folder}

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
tar cvf ${output_folder}-linux-x64.tar ${output_folder}
gzip ${output_folder}-linux-x64.tar
#zip -r ${output_folder}-mac.zip ${output_folder}

echo "Packaging created"

exit 0
