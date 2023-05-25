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
#include <stdio.h>

int main() {
  // Create event session
#ifdef ENABLE_UNIKORN_RECORDING
  UkFileFlushInfo flush_info; // Needs to be persistent for life of session
  // Arguments: filename, max_events, flush_when_full, is_multi_threaded, record_instance, record_value, record_location, &flush_info
  void *unikorn_session = UK_CREATE("./hello.events", 10000, false, false, true, true, true, &flush_info);
#endif

  // Print without recording
  printf("Hello!\n");

  // Record print
  UK_RECORD_EVENT(unikorn_session, PRINT_START_ID, 0);
  printf("Hello!\n");
  UK_RECORD_EVENT(unikorn_session, PRINT_END_ID, 42);

  // Folders and loops
  UK_OPEN_FOLDER(unikorn_session, FOLDER_SOLAR_SYSTEM_ID);
  UK_RECORD_EVENT(unikorn_session, FOR_LOOP_START_ID, 0);
  for (int j=0; j<5; j++) {
    UK_RECORD_EVENT(unikorn_session, PRINT_START_ID, j);
    printf("Earth to Mars!\n");
    UK_RECORD_EVENT(unikorn_session, PRINT_END_ID, 299792458);
  }
  UK_RECORD_EVENT(unikorn_session, FOR_LOOP_END_ID, 0);
  UK_CLOSE_FOLDER(unikorn_session);

  // Another recorded print
  UK_RECORD_EVENT(unikorn_session, PRINT_START_ID, 0);
  printf("Good Bye!\n");
  UK_RECORD_EVENT(unikorn_session, PRINT_END_ID, 23.33);

  // Clean up
  UK_FLUSH(unikorn_session);
  UK_DESTROY(unikorn_session, &flush_info);
#ifdef ENABLE_UNIKORN_RECORDING
  printf("Events were recorded. Use UnikornViewer to view the .events file.\n");
#else
  printf("Event recording is not enabled.\n");
#endif

  return 0;
}
