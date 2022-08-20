Every software API should have a basic hello world example.
Use this as the starting point for any application.
Modify the 'unikorn_instrumentation.h' file to define your own
custom folders and events


Linux & Mac:
  Without event instrumentation:
    > make
  With event instrumentation (one of):
    > make INSTRUMENT_APP=Yes CLOCK=gettime
    > make INSTRUMENT_APP=Yes CLOCK=gettimeofday
  Run:
    > ./hello
  View Results:
    View 'hello.events' with UnikornViewer
  Clean:
    > make clean


Windows:
  Without event instrumentation:
    > nmake -f windows.Makefile
  With event instrumentation (one of):
    > nmake -f windows.Makefile INSTRUMENT_APP=Yes CLOCK=queryperformancecounter
    > nmake -f windows.Makefile INSTRUMENT_APP=Yes CLOCK=ftime
  Run:
    > hello
  View Results:
    View 'hello.events' with UnikornViewer
  Clean:
    > nmake -f windows.Makefile clean
