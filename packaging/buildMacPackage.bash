#!/bin/bash

if [ "$#" -ne "2" ]; then
    echo "USAGE: buildMacPackage.bash <Qt_folder> <unikorn_version>"
    echo "     > ./buildMacPackage.bash $HOME/3rdParty/Qt5/5.15.2/clang_64 1.0.0"
    exit 0
fi

export qt_folder=$1
export unikorn_version=$2
export output_folder="Unikorn-${unikorn_version}"

echo "unikorn version = ${unikorn_version}"
echo "output folder = ${output_folder}"
echo "qt_folder = ${qt_folder}"

[ ! -d ${qt_folder} ] && echo "ERROR: The folder '${qt_folder}' does not exist!" && exit 0

# Create needed folders
mkdir ${output_folder}

# Copy the relevant files
cp ../LICENSE ${output_folder}
cp ../Unikorn_Introduction.pdf ${output_folder}
cp ../GETTING_STARTED.txt ${output_folder}
cp -r ../examples ${output_folder}
cp -r ../src ${output_folder}
cp -r ../inc ${output_folder}
cp -r ../visualizer ${output_folder}

# Create folder for prebuild mac GUI
mkdir ${output_folder}/visualizer/mac

# Build the unikorn viewer
cd ../visualizer
qmake
make -j8
cp -r UnikornViewer.app ../packaging/${output_folder}/visualizer/mac
make distclean
rm .qmake.stash
cd ../packaging/${output_folder}/visualizer/mac
${qt_folder}/bin/macdeployqt UnikornViewer.app
cd ../../..

# Compress package
tar cvf ${output_folder}-osx.tar ${output_folder}
gzip ${output_folder}-osx.tar
#zip -r ${output_folder}-mac.zip ${output_folder}

echo "Unikorn Mac package created"

exit 0
