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
#include <stdio.h>

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
  FileFlushInfo flush_info;
  void *session = EVENTS_INIT(filename, max_events, flush_when_full, is_threaded, record_instance, record_value, record_location, &flush_info);
#endif

  // Do some processing
  EVENTS_START_FOR_LOOP(session, 0);
  for (int j=0; j<10; j++) {
    EVENTS_START_PRINTF(session, j);
    printf("%d: Hello!\n", j+1);
    EVENTS_END_PRINTF(session, j);
  }
  EVENTS_END_FOR_LOOP(session, 0);

  // Clean up
  EVENTS_FLUSH(session);
  EVENTS_FINALIZE(session);
#ifdef INSTRUMENT_APP
  printf("Events were recorded to the file '%s'. Use the Unikorn Viewer to view the results.\n", filename);
#else
  printf("Event recording is not enabled.\n");
#endif

  return 0;
}
