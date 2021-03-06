This example is used to show how see memory contention via visualizing events.

Make sure to load all the event files to see them all at once in the visualizer, and align all the
times to the 'Barrier' end event (instance == 0) when the visualizer asks how to align the files.

Memory contention can come from different aspects:
  - Improper use of caching:
      This can be alleviated by splitting the processing into smaller segments of memory.
      When viewing the file '1_concurrent_thread.events', notice with the first set of 'Sqrt()' events
      take more time to process than the remaining. This is because the various levels of cache are
      getting the data from main memory.
  - Overuse of threads:
      When comapring all the event files with overall time to complete processing, you'll notice the
      more threads, the more time needed to complete processing. This is OK to a point, but eventually
      they will all be competing to use the same memory.


Linux & Mac:
  Without event instrumentation:
    > make
  With event instrumentation (one of):
    > make INSTRUMENT_APP=Yes CLOCK=gettimeofday
    > make INSTRUMENT_APP=Yes CLOCK=gettime
  Run:
    > ./memory_contention <num_threads> <num_elements>
    > ./memory_contention 4 1000
  View Results:
    View the event file simultaneously with UnikornViewer
  Clean:
    > make clean


Windows:
  Without event instrumentation:
    > nmake -f windows.Makefile
  With event instrumentation (one of):
    > nmake -f windows.Makefile INSTRUMENT_APP=Yes CLOCK=queryperformancecounter
    > nmake -f windows.Makefile INSTRUMENT_APP=Yes CLOCK=ftime
  Run:
    > memory_contention <num_threads> <num_elements>
    > memory_contention 4 1000
  View Results:
    View the event file simultaneously with UnikornViewer
  Clean:
    > nmake -f windows.Makefile clean
