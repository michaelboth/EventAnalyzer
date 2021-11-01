#define DEFINE_FOLDERS_AND_EVENTS
#include "event_instrumenting.h"
#include "events_loader.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

EVENTS_GLOBAL_INSTANCE;

static void doStuff() {
  double a = 4.0;
  EVENTS_START_SQRT();
  double b = sqrt(a);
  EVENTS_END_SQRT();
  EVENTS_START_PRINT();
  printf("The square root of %f is %f\n", a, b);
  EVENTS_END_PRINT();
}

int main(int argc, char **argv) {
  if (argc != 8) {
    printf("Usage Example:  %s record_and_load.events 100 auto_flush=no threaded=yes instance=yes value=yes location=yes\n", argv[0]);
    return 0;
  }
  const char *filename = argv[1];

  // Record and save events
  {
#ifdef INSTRUMENT_EVENTS
    uint32_t max_events = (uint32_t)atoi(argv[2]);
    assert(max_events > 0);
    assert(strncmp("auto_flush=", argv[3], 11)==0);
    assert(strncmp("threaded=",   argv[4], 9)==0);
    assert(strncmp("instance=",   argv[5], 9)==0);
    assert(strncmp("value=",      argv[6], 6)==0);
    assert(strncmp("location=",   argv[7], 9)==0);
    bool flush_when_full = strcmp(argv[3], "auto_flush=yes")==0;
    bool is_threaded = strcmp(argv[4], "threaded=yes")==0;
    bool record_instance = strcmp(argv[5], "instance=yes")==0;
    bool record_value = strcmp(argv[6], "value=yes")==0;
    bool record_location = strcmp(argv[7], "location=yes")==0;
#endif
    // Init
    EVENTS_INIT(filename, max_events, flush_when_full, is_threaded, record_instance, record_value, record_location);
    // Record
    doStuff();
    EVENTS_FOLDER1();
    doStuff();
    EVENTS_FOLDER2();
    doStuff();
    EVENTS_CLOSE_FOLDER();
    EVENTS_CLOSE_FOLDER();
    doStuff();
    EVENTS_FOLDER2();
    doStuff();
    EVENTS_CLOSE_FOLDER();
    doStuff();
    // Save and finalize
    EVENTS_FLUSH();
    EVENTS_FINALIZE();
  }

  // Load the events
#ifdef INSTRUMENT_EVENTS
  /*+ make sure printf are compiled in to show the loaded values */
  Events *instance = loadEventsFile(filename);
  freeEvents(instance);
#endif
}
