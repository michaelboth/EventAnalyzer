This example is used to determine clock performance for the various
clocks that are supported. Test your own custom clock as needed.

Many clocks are already implemented in src/unikorn_clock_*.c
but you can implement your own clock if needed.


Linux & Mac:
  Choose one of:
    > make CLOCK=gettime
    > make CLOCK=gettimeofday
  Run:
    > ./test_clock
  Clean:
    > make clean

Windows:
  Choose one of:
    > nmake -f windows.Makefile CLOCK=ftime
    > nmake -f windows.Makefile CLOCK=queryperformancecounter
  Run:
    > test_clock
  Clean:
    > nmake -f windows.Makefile clean
