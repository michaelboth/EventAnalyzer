This example is used to determine clock performance for the various
clocks that are supported. Test you own custom clock as needed.

To add your own clock to the common reference, edit ref/event_clocks.c


BUILD & RUN
Linux & Mac:
  Choose one of:
    > make CLOCK=gettimeofday
    > make CLOCK=clock_gettime
    > make CLOCK=<your-own-clock>
  Run:
    > ./test_clock
  Clean:
    > make clean

Windows:
  Choose one of:
    > nmake -f windows.Makefile CLOCK=ftime
    > nmake -f windows.Makefile CLOCK=QueryPerformanceCounter
    > nmake -f windows.Makefile CLOCK=<your-own-clock>
  Run:
    > test_clock
  Clean:
    > nmake -f windows.Makefile clean
