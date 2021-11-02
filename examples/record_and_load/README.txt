This example is used to validate recording and loading.
Not an overly useful example other than to do some sanity checking.


BUILD & RUN
Linux:
  Without instrumentation:
    > make
  With instrumentation (one of):
    > make INSTRUMENT_APP=Yes CLOCK=gettimeofday
    > make INSTRUMENT_APP=Yes CLOCK=clock_gettime
  > ./record_and_load record_and_load.events 100 auto_flush=no threaded=yes instance=yes value=yes location=yes
  View the record_and_load.events file With Unikorn GUI
  > make clean

Windows:
  TBD
