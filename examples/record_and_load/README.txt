This example is used to validate recording and loading.
Not an overly useful example other than to do some sanity checking.


BUILD & RUN
Linux & Mac:
  Without instrumentation:
    > make
  With instrumentation (one of):
    > make INSTRUMENT_APP=Yes CLOCK=gettimeofday
    > make INSTRUMENT_APP=Yes CLOCK=clock_gettime
  Run:
    > ./record_and_load record_and_load.events 100 auto_flush=no threaded=yes instance=yes value=yes location=yes
    > ./record_and_load record_and_load.events 20 auto_flush=no threaded=yes instance=yes value=yes location=yes
    > ./record_and_load record_and_load.events 20 auto_flush=yes threaded=yes instance=yes value=yes location=yes
  View Results:
    View the record_and_load.events file With Unikorn Viewer
  Clean:
    > make clean

Windows:
  Without instrumentation:
    > nmake -f windows.Makefile [THREAD_SAFE=Yes]
  With instrumentation (one of):
    > nmake -f windows.Makefile [THREAD_SAFE=Yes] INSTRUMENT_APP=Yes CLOCK=QueryPerformanceCounter
    > nmake -f windows.Makefile [THREAD_SAFE=Yes] INSTRUMENT_APP=Yes CLOCK=ftime
  Run:
    > record_and_load record_and_load.events 100 auto_flush=no threaded=yes instance=yes value=yes location=yes
    > record_and_load record_and_load.events 20 auto_flush=no threaded=yes instance=yes value=yes location=yes
    > record_and_load record_and_load.events 20 auto_flush=yes threaded=yes instance=yes value=yes location=yes
  View Results:
    View 'record_and_load.events' With Unikorn Viewer
  Clean:
    > nmake -f windows.Makefile clean
