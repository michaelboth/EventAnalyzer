#!/bin/bash

if [ "$#" -ne "1" ]; then
    echo "USAGE: buildLinuxPackage.bash <unikorn_version>"
    echo "     > ./buildLinuxPackage.bash 1.0.0"
    exit 0
fi

export unikorn_version=$1
export output_folder="Unikorn-${unikorn_version}"

echo "unikorn version = ${unikorn_version}"
echo "output folder = ${output_folder}"

# Create needed foldeers
mkdir ${output_folder}
mkdir ${output_folder}/inc
mkdir ${output_folder}/visualizer
mkdir ${output_folder}/visualizer/linux

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

# Compress package
tar cvf ${output_folder}-linux-x64.tar ${output_folder}
gzip ${output_folder}-linux-x64.tar
#zip -r ${output_folder}-linux-x64.zip ${output_folder}

echo "Unikorn Linux package created"

exit 0
