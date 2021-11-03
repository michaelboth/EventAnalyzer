A simple example to encapsulate all the functionality in a single file.
Refer to the 'hello2' example to see a better design.

BUILD & RUN
Linux & Mac:
  > make
  > ./hello
  View 'hello.events' With Unikorn GUI
  > make clean

Windows:
  > nmake -f windows.Makefile [THREAD_SAFE=Yes]
  > hello
  View 'hello.events' With Unikorn GUI
  > nmake -f windows.Makefile clean
