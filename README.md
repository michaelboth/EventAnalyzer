<img src="visualizer/icons/unikorn_logo.png" alt="Logo" style="width:400px;"/>

# Unikorn Software Event Analyzer
Unikorn is C API (source code, not a library) and graphical visualizer (Windows, Mac, and Linux) used to easily and quickly improve the design, performance, and reliability of complex software.

Even seasoned experts will have difficulty fixing, tuning and improving software during development... like looking for a **unicorn**.

Just instrument your source code with meaningful event; you defined the names and colors. This instrumentation is optional at compile time. Run your application, then view the results in the Unikorn graphical viewer.
<img src="docs/UnikornViewer.png" alt="Logo" style="width:900px;"/>

Unikorn helps with:
- Revealing sub-microsecond timing of key events, functions, code segments
- Validating complex dataflow interactions between CPU threads, processes, GPU streams, communication, IO
- Validating determinism even with long runs (e.g. hours, days)
- Implicitly seeing memory contention, thread starvation, context switches, and many other events
- Finding causality bugs: e.g. knowing what happing just before the application started failing.

Unikorn is easy to use, and should be part of daily development from the application's inception to distribution.
<br>

## Downloading Unikorn
In the 'Releases' section (right panel of the GitHub webpage, near the top), click on 'Latest' to get the latest release. 

## Preparing the OS Environment
To instrument and build your application, the following is needed:
### Linux:
gcc and make
### Mac
xCode
### Windows
Visual Studio<br>
Unikorn's API can optionally be thread safe, which requires Posix threads, which is not supported in Visual Studio. To download and build it:
1. Get the source code from: https://sourceforge.net/projects/pthreads4w/
2. Unzip, rename to 'pthreads4w' and put in the C:\ folder
3. Start a Visual Studio x64 native shell
```
> cd c:\pthreads4w
> nmake VC VC-debug VC-static VC-static-debug install DESTROOT=.\install
```

## Instrumenting Your Application with Events
The following is the 'hello' example. All of the event recording source code is compiled out if ```ENABLE_UNIKORN_RECORDING``` is not defined when compiling.
```c
#define ENABLE_UNIKORN_SESSION_CREATION
#include "unikorn_instrumentation.h"
#include <stdio.h>

int main() {
  // Create event session
#ifdef ENABLE_UNIKORN_RECORDING
  UkFileFlushInfo flush_info; // Needs to be persistant for life of session
  // Arguments: filename, max_events, flush_when_full, is_threaded, record_instance, record_value, record_location, &flush_info
  void *unikorn_session = UNIKORN_INIT("./hello.events", 10000, false, false, true, true, true, &flush_info);
#endif

  // Do some simple timing
  UNIKORN_START_FOR_LOOP(unikorn_session, 0);
  for (int j=0; j<10; j++) {
    UNIKORN_START_PRINTF(unikorn_session, j);
    printf("%d: Hello!\n", j+1);
    UNIKORN_END_PRINTF(unikorn_session, j);
  }
  UNIKORN_END_FOR_LOOP(unikorn_session, 0);

  // Clean up
  UNIKORN_FLUSH(unikorn_session);
  UNIKORN_FINALIZE(unikorn_session);
#ifdef ENABLE_UNIKORN_RECORDING
  printf("Events were recorded. Use UnikornViewer to view the .events file.\n");
#else
  printf("Event recording is not enabled.\n");
#endif

  return 0;
}
```

The instrumentation is defined in ```unikorn_instrumentation.h```. Here is the code snippet that defines the events (names and colors):
```c
static UkEventInfo L_events[] = {
  // Name        Color      Start ID            End ID           Start Value Name  End Value Name
  { "For Loop",  UK_BLACK,  FOR_LOOP_START_ID,  FOR_LOOP_END_ID, "",               ""},
  { "printf()",  UK_BLUE,   PRINTF_START_ID,    PRINTF_END_ID,   "Loop Index",     "Loop Index"},
};
```
Modify this file for use in your application. Define any number of events.<br>

If ```ENABLE_UNIKORN_RECORDING``` is defined then your application also needs to compile in a few more unikorn files:
```
src/unikorn.c                                # The event recording engine
src/unikorn_clock_gettime.c                  # A clock for Mac/Linux: high precision on most variations
src/unikorn_clock_gettimeofday.c             # A clock for Mac/Linux: good precision and portable
src/unikorn_clock_queryperformancecounter.c  # A clock for Windows: high precision
src/unikorn_clock_ftime.c                    # A clock for Windows: not high precision
src/unikorn_file_flush.c                     # Flush event data to a file
inc/*.h                                      # Header files
```

## Examples
To help you get started, some examples are provided
Example | Description
--------|------------
hello | Duh
memory_contention | A simple example to show how multi-threaded processing can effect memory.
test_clock | Helpful if you need to characterize the overhead and precision of a clock.
test_record_and_load | A simple and full featured (including folders) example used to validate the unikorn API and event loading using ```src/unikorn_file_loader.c```


## Developing a Visualizer or Application to Analyze the Events
If you are creating you own graphical visualizer, or just need to load events into some post-processing application, you can use the supplied source code to load the events:
```
src/unikorn_file_loader.c       # Code for load a .events file
int/unikorn_file_loader.h       # Header file for unikorn_file_loader.c
```

## Visualizing Events with the Unikorn Viewer
The Unikorn Viewer is written in C++ using the Open Source Qt framework (www.qt.io).<br>

Get the pre-built viewers from the 'Releases' section (right panel of the GitHub webpage, near the top), click on 'Latest' to get the latest release.<br>

To manualy build it yourself, do the following:<br>
*Tested with Qt Qt 5.15.2*<br>
### Linux and Mac
```
> cd unikorn/visualizer
> qmake
> make -j8
> ./UnikornViewer
```
### Windows
```
> cd unikorn\visualizer
> qmake
> nmake
> release\UnikornViewer.exe
```
