This example is used to determine clock performance for the various
clocks that are supported. Test you own custom clock as needed.

To add your own clock to the common reference, edit ref/event_clocks.c


BUILD & RUN
Linux:
  Choose one of:
    > make CLOCK=gettimeofday
    > make CLOCK=clock_gettime
    > make CLOCK=<your-own-clock>
  > ./test_clock
  > make clean

Windows:
  TBD
