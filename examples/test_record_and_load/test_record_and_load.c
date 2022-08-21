// Copyright 2021,2022 Michael Both
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
#include <assert.h>

/*+ valgrind */

#ifdef ENABLE_UNIKORN_RECORDING
static void *unikorn_session = NULL;
#endif

static void doStuff() {
  double a = 4.0;
  UNIKORN_START_SQRT(unikorn_session, a);
  double b = sqrt(a);
  UNIKORN_END_SQRT(unikorn_session, b);
  UNIKORN_START_PRINT(unikorn_session, 0);
  printf("The square root of %f is %f\n", a, b);
  UNIKORN_END_PRINT(unikorn_session, 0);
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
  UkFileFlushInfo flush_info; // Needs to be persistant for life of session
  unikorn_session = UNIKORN_INIT(filename, max_events, flush_when_full, is_multi_threaded, record_instance, record_value, record_location, &flush_info);
#endif

  // Record
  doStuff();
  UNIKORN_OPEN_FOLDER1(unikorn_session);
  doStuff();
  UNIKORN_OPEN_FOLDER2(unikorn_session);
  doStuff();
  UNIKORN_CLOSE_FOLDER(unikorn_session);
  UNIKORN_CLOSE_FOLDER(unikorn_session);
  doStuff();
  UNIKORN_OPEN_FOLDER2(unikorn_session);
  doStuff();
  UNIKORN_CLOSE_FOLDER(unikorn_session);
  doStuff();

  // Save and finalize
  UNIKORN_FLUSH(unikorn_session);
  UNIKORN_FINALIZE(unikorn_session);

  // Load the events
#ifdef ENABLE_UNIKORN_RECORDING
  UkEvents *instance = ukLoadEventsFile(filename);
  ukFreeEvents(instance);
  printf("Events were recorded to the file '%s'. Use the Unikorn Viewer to view the results.\n", filename);
#else
  printf("Event recording is not enabled.\n");
#endif
}
