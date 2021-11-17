This example is used to show how see memory contention via visualizing events

Memory contention can come from different angles:
  Cache levels
  Main memory


BUILD & RUN
Linux & Mac:
  Build with one of:
    > make CLOCK=gettimeofday
    > make CLOCK=clock_gettime
  Run
    > ./memory_contention <num_threads> <num_elements>
    > ./memory_contention 1 1000
    > ./memory_contention 1 10000
    > ./memory_contention 1 100000
    > ./memory_contention 8 1000
    > ./memory_contention 8 10000
    > ./memory_contention 8 100000
  View Results:
    View 'memory_contention.events' With Unikorn Viewer
  Clean:
    > make clean

Windows:
  Build with one of:
    > nmake -f windows.Makefile THREAD_SAFE=Yes CLOCK=QueryPerformanceCounter
    > nmake -f windows.Makefile THREAD_SAFE=Yes CLOCK=ftime
  Run
    > memory_contention <num_threads> <num_elements>
    > memory_contention 1 1000
    > memory_contention 1 10000
    > memory_contention 1 100000
    > memory_contention 8 1000
    > memory_contention 8 10000
    > memory_contention 8 100000
  View Results:
    View 'memory_contention.events' With Unikorn Viewer
  Clean:
    > nmake -f windows.Makefile clean
