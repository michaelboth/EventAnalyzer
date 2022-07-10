// Copyright 2021 Michael Both
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#define DEFINE_FOLDERS_AND_EVENTS
#include "custom_folders_and_events.h"
#ifdef INSTRUMENT_APP
  #include "event_file_loader.h"
#endif
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#ifdef INSTRUMENT_APP
static void *session = NULL;
#endif

static void doStuff() {
  double a = 4.0;
  EVENTS_START_SQRT(session, a);
  double b = sqrt(a);
  EVENTS_END_SQRT(session, b);
  EVENTS_START_PRINT(session, 0);
  printf("The square root of %f is %f\n", a, b);
  EVENTS_END_PRINT(session, 0);
}

int main(int argc, char **argv) {
  if (argc != 8) {
    printf("Usage Example:  %s record_and_load.events 100 auto_flush=no threaded=yes instance=yes value=yes location=yes\n", argv[0]);
    return 0;
  }

  // Create event session
#ifdef INSTRUMENT_APP
  const char *filename = argv[1];
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
  FileFlushInfo flush_info;
  session = EVENTS_INIT(filename, max_events, flush_when_full, is_threaded, record_instance, record_value, record_location, &flush_info);
#endif

  // Record
  doStuff();
  EVENTS_FOLDER1(session);
  doStuff();
  EVENTS_FOLDER2(session);
  doStuff();
  EVENTS_CLOSE_FOLDER(session);
  EVENTS_CLOSE_FOLDER(session);
  doStuff();
  EVENTS_FOLDER2(session);
  doStuff();
  EVENTS_CLOSE_FOLDER(session);
  doStuff();

  // Save and finalize
  EVENTS_FLUSH(session);
  EVENTS_FINALIZE(session);

  // Load the events
#ifdef INSTRUMENT_APP
  Events *instance = loadEventsFile(filename);
  freeEvents(instance);
  printf("Events were recorded to the file '%s'. Use the Unikorn Viewer to view the results.\n", filename);
#else
  printf("Event recording is not enabled.\n");
#endif
}
