A simple example to encapsulate all the functionality in a single file.
Refer to the 'hello2' example to see a better design.

BUILD & RUN
Linux & Mac:
  Build:
    > make
  Run:
    > ./hello
  View Results:
    View 'hello.events' With Unikorn Viewer
  Clean:
    > make clean

Windows:
  Build:
    > nmake -f windows.Makefile [THREAD_SAFE=Yes]
  Run:
    > hello
  View Results:
    View 'hello.events' With Unikorn Viewer
  Clean:
    > nmake -f windows.Makefile clean
