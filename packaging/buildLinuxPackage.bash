#!/bin/bash

if [ "$#" -ne "2" ]; then
    echo "USAGE: buildLinuxPackage.bash <Qt_folder> <unikorn_version>"
    echo "     > ./buildLinuxPackage.bash $HOME/3rdParty/Qt5/5.15.2 1.0.0"
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
mkdir ${output_folder}/visualizer
mkdir ${output_folder}/visualizer/linux
mkdir ${output_folder}/visualizer/linux/platforms

# Copy the relevant files
cp ../LICENSE ${output_folder}
cp ../Unikorn_Introduction.pdf ${output_folder}
cp ../GETTING_STARTED.txt ${output_folder}
cp -r ../examples ${output_folder}
cp -r ../src ${output_folder}
cp -r ../inc ${output_folder}

# Build the unikorn viewer
cd ../visualizer
qmake
make -j8
cp UnikornViewer ../packaging/${output_folder}/visualizer/linux
make distclean
cd ../packaging
cp UnikornViewer.sh ${output_folder}/visualizer/linux
cp ${qt_folder}/plugins/platforms/libqxcb.so ${output_folder}/visualizer/linux/platforms
cp ${qt_folder}/lib/libQt5Core.so.5 ${output_folder}/visualizer/linux
cp ${qt_folder}/lib/libQt5Gui.so.5 ${output_folder}/visualizer/linux
cp ${qt_folder}/lib/libQt5Widgets.so.5 ${output_folder}/visualizer/linux
cp ${qt_folder}/lib/libQt5XcbQpa.so.5 ${output_folder}/visualizer/linux
cp ${qt_folder}/lib/libQt5DBus.so.5 ${output_folder}/visualizer/linux
#cp ${qt_folder}/lib/libicui18n.so.56 ${output_folder}/visualizer/linux
#cp ${qt_folder}/lib/libicuuc.so.56 ${output_folder}/visualizer/linux
#cp ${qt_folder}/lib/libicudata.so.56 ${output_folder}/visualizer/linux
cp /lib/x86_64-linux-gnu/libicui18n.so.70 ${output_folder}/visualizer/linux
cp /lib/x86_64-linux-gnu/libicuuc.so.70 ${output_folder}/visualizer/linux
cp /lib/x86_64-linux-gnu/libicudata.so.70 ${output_folder}/visualizer/linux

# Compress package
tar cvf ${output_folder}-linux-x64.tar ${output_folder}
gzip ${output_folder}-linux-x64.tar
#zip -r ${output_folder}-linux-x64.zip ${output_folder}

echo "Unikorn Linux package created"

exit 0
