<img src="visualizer/icons/unikorn_logo.png" alt="Logo" style="width:400px;"/>

# Unikorn Software Event Analyzer
With the increased complexity of modern hardware and software, optimizing an application for performance is sometimes like trying to find a **unicorn**. Going green is more important than ever. Stop accepting poor performing software or using more hardware to fix bad performance. Unikorn is easy to use, and should be part of daily development from the application's inception to distribution.
<br><br>
Unikorn is a C API and graphical visualizer (Windows, Mac, and Linux). These are the simple steps to discovering the behaviour of your application:
1. Instrument your application with custom events (names and colors), a clock, and output stream to flush the events
2. Run your application; make sure the events get flushed
3. View the events in the Unikorn Viewer
<img src="docs/UnikornViewer.png" alt="Logo" style="width:900px;"/>


## Preparing the Environment
### Linux:
You'll need gcc and make<br>
*Tested on Ubuntu 18.04*
### Mac
You'll need xCode<br>
*Tested on OSX 11.6 (Big Sur), using xCode 13.1*
### Windows
You'll need Visual Studio (mingw may work also)<br>
*Tested with Visual Studio 2019 64bit console window*<br>
You may need Posix threads. Unikorn's API can optionally be thread safe, and if so requires Posix threads. Visual Studio does not have native support for Posix threads, so you'll need to download and build it:
1. Get the source code from: https://sourceforge.net/projects/pthreads4w/
2. Unzip, rename to 'pthreads4w' and put in the C:\ folder
3. Start a Visual Studio x64 native shell
```
> cd c:\pthreads4w
> nmake VC VC-debug VC-static VC-static-debug install DESTROOT=.\install
```

## Instrumenting Your Application with Events
The following is the 'hello' example. All of the event recording source code is compiled out if ```INSTRUMENT_APP``` is not defined when compiling.
```c
#define DEFINE_FOLDERS_AND_EVENTS
#include "custom_folders_and_events.h"
#include <stdio.h>

// Define the event session global variable
EVENTS_GLOBAL_INSTANCE;

int main() {
  // Create event session
#ifdef INSTRUMENT_APP
  const char *filename = "./hello.events";
  uint32_t max_events = 10000;
  bool flush_when_full = false;
  bool is_threaded = false;
  bool record_instance = true;
  bool record_value = true;
  bool record_location = true;
#endif
  EVENTS_INIT(filename, max_events, flush_when_full, is_threaded, record_instance, record_value, record_location);

  // Do some processing
  EVENTS_START_FOR_LOOP();
  for (int j=0; j<10; j++) {
    EVENTS_START_PRINTF();
    printf("%d: Hello!\n", j+1);
    EVENTS_END_PRINTF();
  }
  EVENTS_END_FOR_LOOP();

  // Clean up
  EVENTS_FLUSH();
  EVENTS_FINALIZE();
#ifdef INSTRUMENT_APP
  printf("Events were recorded to the file '%s'. Use the Unikorn Viewer to view the results.\n", filename);
#else
  printf("Event recording is not enabled.\n");
#endif

  return 0;
}
```

The custom folders and events are defined in ```custom_folders_and_events.h```. Here is the code snippet that defined the events (names and colors):
```c
static UkEventInfo L_events[] = {
  // Name          Start ID              End ID              Color
  { "For Loop",    FOR_LOOP_START_ID,    FOR_LOOP_END_ID,    UK_BLACK},
  { "printf()",    PRINTF_START_ID,      PRINTF_END_ID,      UK_BLUE}
};
```
Modify this file for use in your application. You could define any number of folders and events.<br>

If ```INSTRUMENT_APP``` is defined then example is built with a few other files that can be common to any application you instrument with event recording:
```
src/unikorn.c                                       # The event recording engine
src/app_event_recording.c                           # A wrapper file to make it easy to compile out event instrumentation
src/event_recorder_clock_gettime.c                  # A clock for Mac/Linux: high precision on most variations
src/event_recorder_clock_gettimeofday.c             # A clock for Mac/Linux: good precision and portable
src/event_recorder_clock_queryperformancecounter.c  # A clock for Windows: high precision
src/event_recorder_clock_ftime.c                    # A clock for Windows: not high precision
src/event_recorder_file_flush.c                     # Flush event data to a file
inc/*.h                                             # Header files
```

## Examples
To help you get started, some examples are provided
Example | Description
--------|------------
hello | Duh
memory_contention | A simple example show how multi-threaded processing can effect memory.
test_clock | Helpful if you need to characterize the overhead and precision of a clock.
test_record_and_load | A simple example used to validate the unikorn API and event loading using ```src/event_file_loader.c```


## Developing a Visualizer or Application to Analyze the Events
If you are creating you own graphical visualizer, or just need to load events into some post-processing application, you can use the supplied source code to load the events:
```
src/event_file_loader.c       # Code for load a .events file
int/event_file_loader.h       # Header file for event_file_loader.c
```

## Visualizing Events with the Unikorn Viewer
The Unikorn Viewer is written in C++ using the Open Source Qt framework (www.qt.io).<br>
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
