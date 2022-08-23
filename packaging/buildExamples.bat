@echo off

rem Keep variables local to this script
setlocal EnableDelayedExpansion

rem count number of args
set argC=0
for %%i in (%*) do set /A argC+=1

rem + if "%argC%" neq "2" (
if "%argC%" equ "1" (
  if "%1" neq "clean" (
    echo "USAGE: buildExamples.bat [clean]"
    echo "The only optional supported arg is 'clean'"
    GOTO:done
  )
  echo "Cleaning examples"
  cd ..\examples\hello
  nmake -f windows.Makefile clean
  cd ..\multi_thread_and_file
  nmake -f windows.Makefile clean
  cd ..\test_clock
  nmake -f windows.Makefile clean
  cd ..\test_record_and_load
  nmake -f windows.Makefile clean
  GOTO:done
)

echo "Building and running"

cd ..\lib
nmake -f windows.Makefile RELEASE=Yes ATOMIC_RECORDING=No
nmake -f windows.Makefile clean
nmake -f windows.Makefile RELEASE=Yes ATOMIC_RECORDING=Yes
nmake -f windows.Makefile clean

cd ..\examples\hello
nmake -f windows.Makefile
hello
nmake -f windows.Makefile clean
nmake -f windows.Makefile INSTRUMENT_APP=Yes CLOCK=ftime
hello
nmake -f windows.Makefile clean
nmake -f windows.Makefile INSTRUMENT_APP=Yes CLOCK=queryperformancecounter
hello

cd ..\multi_thread_and_file
nmake -f windows.Makefile
multi_thread_and_file 4 1000
nmake -f windows.Makefile clean
nmake -f windows.Makefile INSTRUMENT_APP=Yes CLOCK=ftime
multi_thread_and_file 4 1000
nmake -f windows.Makefile clean
nmake -f windows.Makefile INSTRUMENT_APP=Yes CLOCK=queryperformancecounter
multi_thread_and_file 4 1000

cd ..\test_clock
nmake -f windows.Makefile CLOCK=ftime
test_clock
nmake -f windows.Makefile clean
nmake -f windows.Makefile CLOCK=queryperformancecounter
test_clock
nmake -f windows.Makefile clean

cd ..\test_record_and_load
nmake -f windows.Makefile
test_record_and_load test_record_and_load.events 100 auto_flush=no threaded=yes instance=yes value=yes location=yes
test_record_and_load test_record_and_load.events 17 auto_flush=no threaded=yes instance=yes value=yes location=yes
test_record_and_load test_record_and_load.events 12 auto_flush=yes threaded=yes instance=yes value=yes location=yes
nmake -f windows.Makefile clean
nmake -f windows.Makefile INSTRUMENT_APP=Yes CLOCK=ftime
test_record_and_load test_record_and_load.events 100 auto_flush=no threaded=yes instance=yes value=yes location=yes
test_record_and_load test_record_and_load.events 17 auto_flush=no threaded=yes instance=yes value=yes location=yes
test_record_and_load test_record_and_load.events 12 auto_flush=yes threaded=yes instance=yes value=yes location=yes
nmake -f windows.Makefile clean
nmake -f windows.Makefile INSTRUMENT_APP=Yes CLOCK=queryperformancecounter
test_record_and_load test_record_and_load.events 100 auto_flush=no threaded=yes instance=yes value=yes location=yes
test_record_and_load test_record_and_load.events 17 auto_flush=no threaded=yes instance=yes value=yes location=yes
test_record_and_load test_record_and_load.events 12 auto_flush=yes threaded=yes instance=yes value=yes location=yes

GOTO:done

:done
endlocal
GOTO:EOF
