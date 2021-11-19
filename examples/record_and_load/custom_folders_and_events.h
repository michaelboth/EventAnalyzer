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

#ifndef _CUSTOM_FOLDERS_AND_EVENTS_H_
#define _CUSTOM_FOLDERS_AND_EVENTS_H_

#ifdef INSTRUMENT_APP

#include "unikorn.h"
#include "event_instrumenting.h"
#include <stdlib.h>

enum {
  // Make sure first ID starts with 1, and +1 for each adtional ID
  // Define folders first (if any)
  FOLDER1_ID = 1,
  FOLDER2_ID,
  // Events
  PRINT_START_ID,
  PRINT_END_ID,
  SQRT_START_ID,
  SQRT_END_ID
};

// IMPORTANT: Call #define DEFINE_FOLDERS_AND_EVENTS, just before #include "event_instrumenting.h", in the file that calls EVENTS_INIT()
#ifdef DEFINE_FOLDERS_AND_EVENTS
static UkFolderInfo L_folders[] = {
  { "Folder 1", FOLDER1_ID},
  { "Folder 2", FOLDER2_ID}
};
static UkEventInfo L_events[] = {
  { "Print", PRINT_START_ID, PRINT_END_ID, UK_TEAL},
  { "Sqrt",  SQRT_START_ID,  SQRT_END_ID,  UK_BLACK}
};
#endif

// Overall
#define EVENTS_GLOBAL_INSTANCE extern void *G_instance     // Use this in every file that need to record events
#ifdef DEFINE_FOLDERS_AND_EVENTS
  #define EVENTS_INIT(filename, max_events, flush_when_full, is_threaded, record_instance, record_value, record_location) initEventIntrumenting(filename, max_events, flush_when_full, is_threaded, record_instance, record_value, record_location, sizeof(L_folders) / sizeof(UkFolderInfo), L_folders, sizeof(L_events) / sizeof(UkEventInfo), L_events)
#endif
#define EVENTS_FLUSH() ukFlush(G_instance)
#define EVENTS_FINALIZE() finalizeEventIntrumenting()
// Folders
#define EVENTS_FOLDER1() ukRecordFolder(G_instance, FOLDER1_ID)
#define EVENTS_FOLDER2() ukRecordFolder(G_instance, FOLDER2_ID)
#define EVENTS_CLOSE_FOLDER() ukCloseFolder(G_instance)
// Events
#define EVENTS_START_PRINT() ukRecordEvent(G_instance, PRINT_START_ID, 0.0, __FILE__, __FUNCTION__, __LINE__)
#define EVENTS_END_PRINT()   ukRecordEvent(G_instance, PRINT_END_ID,   0.0, __FILE__, __FUNCTION__, __LINE__)
#define EVENTS_START_SQRT()  ukRecordEvent(G_instance, SQRT_START_ID,  0.0, __FILE__, __FUNCTION__, __LINE__)
#define EVENTS_END_SQRT()    ukRecordEvent(G_instance, SQRT_END_ID,    0.0, __FILE__, __FUNCTION__, __LINE__)

#else

// Overall
#define EVENTS_GLOBAL_INSTANCE
#define EVENTS_INIT(filename, max_events, flush_when_full, is_threaded, record_instance, record_value, record_location)
#define EVENTS_FLUSH()
#define EVENTS_FINALIZE()
// Folders
#define EVENTS_FOLDER1()
#define EVENTS_FOLDER2()
#define EVENTS_CLOSE_FOLDER()
// Events
#define EVENTS_START_PRINT()
#define EVENTS_END_PRINT()
#define EVENTS_START_SQRT()
#define EVENTS_END_SQRT()

#endif
#endif