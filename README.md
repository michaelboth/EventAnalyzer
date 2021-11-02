# Unikorn Software Event Analyzer (C API and GUI Visualizer)
With the increased complexity of modern hardware and software, optimizing an application for performance is sometimes like trying to find a **unicorn**. Going green is more important than ever. Stop accepting poor performing software or using more hardware as a kludge to fix bad performance. Unikorn is so easy to use, it should be with daily development from the application's inception to distribution.
## Building Unikorn's C Library
Linux:
```
> cd unikorn/lib
> make THREAD_SAFE=Yes
```
Windows:
To be completed. Please stand by...
## Instrumenting Your Application with Events
I'll step through the ```examples/hello1/``` example to get you familiar with event instrumentation.
First, include the Unikorn header file plus some standard header files:
```C
#include "unikorn.h"
#include <stdio.h>
#include <stdlib.h>
```
Unikorn can record folders and events. Folders are just containers for holding groups of events, but are not required. Folders (if any) must be defined first. Each folder is assigned one ID and events are assigned a start ID and an end ID. IDs must start with 1 and each subsequent ID must be +1. Using a C `enum` makes this easy.  For this example, no folders will be defined, and we'll create events for two different sort routines:
```C
enum {
  // Folders
  // Events
  QUICK_SORT_START_ID=1,
  QUICK_SORT_END_ID,
  BUBBLE_SORT_START_ID,
  BUBBLE_SORT_END_ID
};
```
Next, define the folders (none for this example) and events. Folders only need a name, and evends need a name and color:
```C
UkEventInfo events[] = {
  { "Quick Sort",  QUICK_SORT_START_ID,  QUICK_SORT_END_ID,  UK_BLUE},
  { "Bubble Sort", BUBBLE_SORT_START_ID, BUBBLE_SORT_END_ID, UK_RED}
};
```
Next, fill in the attribute structure that defines the properties of the event recording session:
```C
UkAttrs attrs = {
  .max_event_count = 10000,     // Max number of events that can be stored in the circular buffer
  .flush_when_full = false,     // Only record when the app explicitly calls ukFlush();
  .is_threaded = true,          // Record the thread ID; each thread will be displayed as a folder in the GUI
  .record_instance = true,      // Record the couter indicating how many times this event was recorded
  .record_value = true,         // Record a 64 bit double value with the event
  .record_file_location = true, // Record file name, function name, and line number where the event was recorded
  .folder_info_count = 0,
  .folder_info_list = NULL,
  .event_info_count = sizeof(events) / sizeof(UkEventInfo),
  .event_info_list = events,
};
```
Next, define a clock to record the current time. Unikorn does not include a built in clock because there are too many clocks to choose from and each clock has different properties. For this example, we'll just use a standard Unix/Windows clock:
```C
#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <Windows.h>
  #include <sys/timeb.h>
#else
  #include <stddef.h>
  #include <sys/time.h>
#endif

uint64_t getEventTime() {
  static uint64_t base_time;
  static bool got_base_time = 0;
#ifdef _WIN32
  static LARGE_INTEGER frequency = 0;
  if (frequency == 0) {
    bool ok = QueryPerformanceFrequency(&frequency);
    assert(!ok);
  }
  LARGE_INTEGER count;
  bool ok = QueryPerformanceCounter((LARGE_INTEGER *)&count);
  assert(!ok);
  LARGE_INTEGER seconds = count / frequency;
  LARGE_INTEGER nanoseconds = (1000000000 * (count % frequency)) / frequency;
  uint64_t total_nanoseconds = (seconds * 1000000000) + nanoseconds;
#else
  // gettimeofday() only has microsecond precision but is more portable
  struct timeval tp;
  gettimeofday(&tp, NULL);
  uint64_t total_nanoseconds = (uint64_t)tp.tv_sec * 1000000000 + (uint64_t)tp.tv_usec * 1000;
#endif
  if (!got_base_time) {
    base_time = total_nanoseconds;
    got_base_time = 1;
  }
  return (total_nanoseconds - base_time); // Return a time starting from 0 indicating when the app started
}
```
Next, need to create the output stream method. Typical this would be a file or a socket. For this example, we'll use a file:
```C
typedef struct {
  const char *filename;
  FILE *file;
  bool events_saved;
  bool append_subsequent_saves;
} FileFlushInfo;

bool prepareFileFlush(void *user_data) {
  FileFlushInfo *flush_info = (FileFlushInfo *)user_data;
  // Open a file
  const char *save_mode = "wb";
  if (flush_info->events_saved && flush_info->append_subsequent_saves) {
    save_mode = "ab";
  }
  flush_info->file = fopen(flush_info->filename, save_mode);
  if (flush_info->file == NULL) {
    return false;
  }
  return true;
}

bool fileFlush(void *user_data, const void *data, size_t bytes) {
  FileFlushInfo *flush_info = (FileFlushInfo *)user_data;
  if (fwrite(data, bytes, 1, flush_info->file) == 1) return true;
  return false;
}

bool finishFileFlush(void *user_data) {
  FileFlushInfo *flush_info = (FileFlushInfo *)user_data;
  int rc = fclose(flush_info->file);
  return (rc == 0);
}

FileFlushInfo flush_info = {
  .filename = "./hello.events",
  .file = NULL,
  .events_saved = false,
  .append_subsequent_saves = false
};
```
All the components needed for creating the event session are now defined. Now, create the event recording session:
```C
void *session = ukCreate(&attrs, getEventTime, &flush_info, prepareFileFlush, fileFlush, finishFileFlush);
```
Now that the event session is created, you can start using the events throughout your application. To record a section of source code, just record the start and stop events. This can be done around a single line of source code, or around a large block of source code:
```C
// Quick sort
ukRecordEvent(session, QUICK_SORT_START_ID, 0.0, __FILE__, __FUNCTION__, __LINE__);
myQuickSort(quick_sort_list, num_elements);
ukRecordEvent(session, QUICK_SORT_END_ID, 0.0, __FILE__, __FUNCTION__, __LINE__);

// Bubble sort
ukRecordEvent(session, BUBBLE_SORT_START_ID, 0.0, __FILE__, __FUNCTION__, __LINE__);
myBubbleSort(bubble_sort_list, num_elements);
ukRecordEvent(session, BUBBLE_SORT_END_ID, 0.0, __FILE__, __FUNCTION__, __LINE__);
```
This is an overly simple example, you might have dozzens of different types of events, and you might record millions of events over a short period of time. When you're ready to flush the events to file, all you need to do is:
```C
ukFlush(session);
```
When you're done recording events, then you just need to free the event session:
```C
ukDestroy(session);
```
That's it.
You could create a set of conveinece files for instrumentating all your applications. I've already created some reference files to do this in the folder ```ref/```:
```
ref/event_clocks.c           # Some useful clock for various OSes
ref/event_clocks.h           # Header file for event_clocks.c
ref/event_file_flush.c       # Functions to flush events to a file
ref/event_file_flush.h       # Header file for event_file_flush.c
ref/event_instrumenting.c    # Reusable code for defining an event session
```
Then use a custom header file to define the folders and events, like with the example ```examples/hello2/```
## Visualizing Events with the GUI
To be completed, please stand by...
