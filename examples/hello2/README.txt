This is a more realistic example compared to 'examples/hello1'

It's better to encapsulate the setup of event instrumentation in separate files in order to
allow multiple applications to share the same setup code. Only the 'event_instrumenting.h'
header file needs to be customized per application.

Also, instrumentation can easily be compiled out once it's no longer needed.


BUILD & RUN
Linux:
  Without instrumentation:
    > make
  With instrumentation:
    > make INSTRUMENT_APP=Yes CLOCK=gettimeofday
  > ./hello
  View 'hello.events' With Unikorn GUI
  > make clean

Windows:
  TBD
