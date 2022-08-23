#!/bin/bash

if [[ "$#" -ne "0" && "$#" -ne "1" ]]; then
    echo "USAGE: buildExamples.bash [clean]"
    exit 0
fi

if [ "$#" -eq "1" ]; then
    echo "Arg: '$1'"
    if [ "$1" != "clean" ]; then
        echo "USAGE: buildExamples.bash [clean]"
        echo "The only optional supported arg is 'clean'"
        exit 0
    fi
    echo "Cleaning examples"
    cd ../examples/hello
    make clean
    cd ../multi_thread_and_file
    make clean
    cd ../test_clock
    make clean
    cd ../test_record_and_load
    make clean
    exit 0
fi


echo "Building and running"

cd ../examples/hello
make
./hello
make clean
make INSTRUMENT_APP=Yes CLOCK=gettimeofday
./hello
make clean
make INSTRUMENT_APP=Yes CLOCK=gettime
./hello

cd ../multi_thread_and_file
make
./multi_thread_and_file 4 1000
make clean
make INSTRUMENT_APP=Yes CLOCK=gettimeofday
./multi_thread_and_file 4 1000
make clean
make INSTRUMENT_APP=Yes CLOCK=gettime
./multi_thread_and_file 4 1000

cd ../test_clock
make CLOCK=gettime
./test_clock
make clean
make CLOCK=gettimeofday
./test_clock
make clean

cd ../test_record_and_load
make
./test_record_and_load test_record_and_load.events 100 auto_flush=no threaded=yes instance=yes value=yes location=yes
./test_record_and_load test_record_and_load.events 17 auto_flush=no threaded=yes instance=yes value=yes location=yes
./test_record_and_load test_record_and_load.events 12 auto_flush=yes threaded=yes instance=yes value=yes location=yes
make clean
make INSTRUMENT_APP=Yes CLOCK=gettimeofday
./test_record_and_load test_record_and_load.events 100 auto_flush=no threaded=yes instance=yes value=yes location=yes
./test_record_and_load test_record_and_load.events 17 auto_flush=no threaded=yes instance=yes value=yes location=yes
./test_record_and_load test_record_and_load.events 12 auto_flush=yes threaded=yes instance=yes value=yes location=yes
make clean
make INSTRUMENT_APP=Yes CLOCK=gettime
./test_record_and_load test_record_and_load.events 100 auto_flush=no threaded=yes instance=yes value=yes location=yes
./test_record_and_load test_record_and_load.events 17 auto_flush=no threaded=yes instance=yes value=yes location=yes
./test_record_and_load test_record_and_load.events 12 auto_flush=yes threaded=yes instance=yes value=yes location=yes
