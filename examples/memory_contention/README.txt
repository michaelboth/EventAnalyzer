This example is used to show how see memory contention via visualizing events.

Make sure to load all the event files to see them all at once in the visualizer, and align all the
times to zero when the visualizer asks how to align the files.

Memory contention can come from different aspects:
  - Overuse of threads
  - Improper use of caching; may need to process in smaller segments of memory

See two distinctive characteristics of memory contention in the visual results:
  - When viewing the file '1_concurrent_thread.events', notice with the first set of 'Sqrt()' events
    take more time to process than the remaining. This is because the various levels of cache are
    getting the data from main memory.
  - When comapring all the event files with overall time to complete processing, you'll notice the
    more threads, the more time needed to complete processing. This is OK to a point, but eventually
    they will all be competing to use the same memory.


BUILD & RUN
Linux & Mac:
  Without instrumentation:
    > make
  With instrumentation (one of):
    > make INSTRUMENT_APP=Yes CLOCK=gettimeofday
    > make INSTRUMENT_APP=Yes CLOCK=clock_gettime
  Run:
    > ./memory_contention <num_threads> <num_elements>
    > ./memory_contention 8 1000
    > ./memory_contention 8 10000
    > ./memory_contention 8 100000
  View Results:
    View all the event files simultaneously with Unikorn Viewer
  Clean:
    > make clean

Windows:
  Without instrumentation:
    > nmake -f windows.Makefile [THREAD_SAFE=Yes]
  With instrumentation (one of):
    > nmake -f windows.Makefile [THREAD_SAFE=Yes] INSTRUMENT_APP=Yes CLOCK=QueryPerformanceCounter
    > nmake -f windows.Makefile [THREAD_SAFE=Yes] INSTRUMENT_APP=Yes CLOCK=ftime
  Run:
    > memory_contention <num_threads> <num_elements>
    > memory_contention 8 1000
    > memory_contention 8 10000
    > memory_contention 8 100000
  View Results:
    View all the event files simultaneously with Unikorn Viewer
  Clean:
    > nmake -f windows.Makefile clean
