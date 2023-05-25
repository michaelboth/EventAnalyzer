// Copyright 2021,2022,2023 Michael Both
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

#define ENABLE_UNIKORN_SESSION_CREATION
#include "unikorn_instrumentation.h"
#ifdef ENABLE_UNIKORN_RECORDING
  #include "unikorn_file_loader.h"
#endif
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef NDEBUG // Don't want assert compiled out
  #undef NDEBUG
#endif
#include <assert.h>

#ifdef ENABLE_UNIKORN_RECORDING
static void *unikorn_session = NULL;
#endif

static void doStuff() {
  double a = 4.0;
  UK_RECORD_EVENT(unikorn_session, SQRT_START_ID, a);
  double b = sqrt(a);
  UK_RECORD_EVENT(unikorn_session, SQRT_END_ID, b);
  UK_RECORD_EVENT(unikorn_session, PRINT_START_ID, 0);
  printf("The square root of %f is %f\n", a, b);
  UK_RECORD_EVENT(unikorn_session, PRINT_END_ID, 0);
}

int main(int argc, char **argv) {
  if (argc != 8) {
    printf("Usage Example:  %s record_and_load.events 100 auto_flush=no threaded=yes instance=yes value=yes location=yes\n", argv[0]);
    return 0;
  }

  // Create event session
#ifdef ENABLE_UNIKORN_RECORDING
  const char *filename = argv[1];
  uint32_t max_events = (uint32_t)atoi(argv[2]);
  assert(max_events > 0);
  assert(strncmp("auto_flush=", argv[3], 11)==0);
  assert(strncmp("threaded=",   argv[4], 9)==0);
  assert(strncmp("instance=",   argv[5], 9)==0);
  assert(strncmp("value=",      argv[6], 6)==0);
  assert(strncmp("location=",   argv[7], 9)==0);
  bool flush_when_full = strcmp(argv[3], "auto_flush=yes")==0;
  bool is_multi_threaded = strcmp(argv[4], "threaded=yes")==0;
  bool record_instance = strcmp(argv[5], "instance=yes")==0;
  bool record_value = strcmp(argv[6], "value=yes")==0;
  bool record_location = strcmp(argv[7], "location=yes")==0;
  UkFileFlushInfo flush_info; // Needs to be persistent for life of session
  unikorn_session = UK_CREATE(filename, max_events, flush_when_full, is_multi_threaded, record_instance, record_value, record_location, &flush_info);
#endif

  // Record
  doStuff();
  UK_OPEN_FOLDER(unikorn_session, FOLDER1_ID);
  doStuff();
  UK_OPEN_FOLDER(unikorn_session, FOLDER2_ID);
  doStuff();
  UK_CLOSE_FOLDER(unikorn_session);
  UK_CLOSE_FOLDER(unikorn_session);
  doStuff();
  UK_OPEN_FOLDER(unikorn_session, FOLDER2_ID);
  doStuff();
  UK_CLOSE_FOLDER(unikorn_session);
  doStuff();

  // Save and finalize
  UK_FLUSH(unikorn_session);
  UK_DESTROY(unikorn_session, &flush_info);

  // Load the events
#ifdef ENABLE_UNIKORN_RECORDING
  UkEvents *instance = ukLoadEventsFile(filename);
  ukFreeEvents(instance);
  printf("Events were recorded to the file '%s'. Use the Unikorn Viewer to view the results.\n", filename);
#else
  printf("Event recording is not enabled.\n");
#endif
}
