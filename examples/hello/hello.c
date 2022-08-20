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

  // Print without recording
  printf("Hello!\n");

  // Record print
  UNIKORN_START_PRINT(unikorn_session, 0);
  printf("Hello!\n");
  UNIKORN_END_PRINT(unikorn_session, 42);

  // Folders and loops
  UNIKORN_OPEN_FOLDER_SOLAR_SYSTEM(unikorn_session);
  UNIKORN_START_FOR_LOOP(unikorn_session, 0);
  for (int j=0; j<5; j++) {
    UNIKORN_START_PRINT(unikorn_session, j);
    printf("Earth to Mars!\n");
    UNIKORN_END_PRINT(unikorn_session, 299792458);
  }
  UNIKORN_END_FOR_LOOP(unikorn_session, 0);
  UNIKORN_CLOSE_FOLDER(unikorn_session);

  // Another recorded print
  UNIKORN_START_PRINT(unikorn_session, 0);
  printf("Good Bye!\n");
  UNIKORN_END_PRINT(unikorn_session, 23.33);

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
