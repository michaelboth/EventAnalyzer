This example is used to validate recording and loading.
Not an overly useful example other than to do some sanity checking.


Linux & Mac:
  Without event instrumentation:
    > make
  With event instrumentation (one of):
    > make INSTRUMENT_APP=Yes CLOCK=gettimeofday
    > make INSTRUMENT_APP=Yes CLOCK=gettime
  Run:
    > ./test_record_and_load test_record_and_load.events 100 auto_flush=no threaded=yes instance=yes value=yes location=yes
    > ./test_record_and_load test_record_and_load.events 17 auto_flush=no threaded=yes instance=yes value=yes location=yes
    > ./test_record_and_load test_record_and_load.events 12 auto_flush=yes threaded=yes instance=yes value=yes location=yes
  View Results:
    View 'test_record_and_load.events' with UnikornViewer
  Clean:
    > make clean


Windows:
  Without event instrumentation:
    > nmake -f windows.Makefile
  With event instrumentation (one of):
    > nmake -f windows.Makefile INSTRUMENT_APP=Yes CLOCK=queryperformancecounter
    > nmake -f windows.Makefile INSTRUMENT_APP=Yes CLOCK=ftime
  Run:
    > test_record_and_load test_record_and_load.events 100 auto_flush=no threaded=yes instance=yes value=yes location=yes
    > test_record_and_load test_record_and_load.events 17 auto_flush=no threaded=yes instance=yes value=yes location=yes
    > test_record_and_load test_record_and_load.events 12 auto_flush=yes threaded=yes instance=yes value=yes location=yes
  View Results:
    View 'test_record_and_load.events' with UnikornViewer
  Clean:
    > nmake -f windows.Makefile clean
