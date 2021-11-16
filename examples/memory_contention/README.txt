This is a more realistic example compared to 'examples/hello1'

It's better to encapsulate the setup of event instrumentation in separate files in order to
allow multiple applications to share the same setup code. Only the 'event_instrumenting.h'
header file needs to be customized per application.

Also, instrumentation can easily be compiled out once it's no longer needed.


BUILD & RUN
Linux & Mac:
  Without instrumentation:
    > make
  With instrumentation (one of):
    > make INSTRUMENT_APP=Yes CLOCK=gettimeofday
    > make INSTRUMENT_APP=Yes CLOCK=clock_gettime
  > ./hello
  View 'hello.events' With Unikorn GUI
  > make clean

Windows:
  Without instrumentation:
    > nmake -f windows.Makefile [THREAD_SAFE=Yes]
  With instrumentation (one of):
    > nmake -f windows.Makefile [THREAD_SAFE=Yes] INSTRUMENT_APP=Yes CLOCK=QueryPerformanceCounter
    > nmake -f windows.Makefile [THREAD_SAFE=Yes] INSTRUMENT_APP=Yes CLOCK=ftime
  > hello
  View 'hello.events' With Unikorn GUI
  > nmake -f windows.Makefile clean
