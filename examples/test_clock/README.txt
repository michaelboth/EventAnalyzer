This example is used to determine clock performance for the various
clocks that are supported. Test you own custom clock as needed.

Many clocks were already implemented in src/event_recorder_clock_*.c
but you can implement you own clock if needed.


Linux & Mac:
  Choose one of:
    > make CLOCK=gettimeofday
    > make CLOCK=gettime
  Run:
    > ./test_clock
  Clean:
    > make clean

Windows:
  Choose one of:
    > nmake -f windows.Makefile CLOCK=ftime
    > nmake -f windows.Makefile CLOCK=QueryPerformanceCounter
  Run:
    > test_clock
  Clean:
    > nmake -f windows.Makefile clean
