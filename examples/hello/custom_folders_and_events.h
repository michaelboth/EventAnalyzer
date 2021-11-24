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
#include "app_event_recording.h"
#include <stdlib.h>

enum {  // IMPORTANT, IDs must start with 1 since 0 is reserved for 'close_folder'
  // Folders
  //  FOLDER1_ID=1,
  //  FOLDER2_ID,
  // Events
  FOR_LOOP_START_ID=1,
  FOR_LOOP_END_ID,
  PRINTF_START_ID,
  PRINTF_END_ID
};

// IMPORTANT: Call #define DEFINE_FOLDERS_AND_EVENTS, just before #include "event_instrumenting.h", in the file that calls EVENTS_INIT()
#ifdef DEFINE_FOLDERS_AND_EVENTS

// Folders
//static UkFolderInfo L_folders[] = {
//    Name        ID
//  { "Folder 1", FOLDER1_ID},
//  { "Folder 2", FOLDER2_ID}
//};
//#define NUM_FOLDERS (sizeof(L_folders) / sizeof(UkFolderInfo))
#define L_folders NULL
#define NUM_FOLDERS 0

// Events
static UkEventInfo L_events[] = {
  // Name          Start ID              End ID              Color
  { "For Loop",    FOR_LOOP_START_ID,    FOR_LOOP_END_ID,    UK_BLACK},
  { "printf()",    PRINTF_START_ID,      PRINTF_END_ID,      UK_BLUE}
};
#define NUM_EVENT_TYPES (sizeof(L_events) / sizeof(UkEventInfo))

#endif

// Define macro to the event session
#define EVENTS_GLOBAL_INSTANCE extern void *G_event_recording_session     // Use this in every file that needs to record events

// Define the init function macro but only for one file
#ifdef DEFINE_FOLDERS_AND_EVENTS
  #define EVENTS_INIT(filename, max_events, flush_when_full, is_threaded, record_instance, record_value, record_location) initEventRecording(filename, max_events, flush_when_full, is_threaded, record_instance, record_value, record_location, NUM_FOLDERS, L_folders, NUM_EVENT_TYPES, L_events)
#endif

// Define the macros to flushing and finalizing
#define EVENTS_FLUSH() ukFlush(G_event_recording_session)
#define EVENTS_FINALIZE() finalizeEventRecording()

// Folder recording macros
//#define EVENTS_FOLDER1() ukRecordFolder(G_event_recording_session, FOLDER1_ID)
//#define EVENTS_FOLDER2() ukRecordFolder(G_event_recording_session, FOLDER2_ID)
//#define EVENTS_CLOSE_FOLDER() ukCloseFolder(G_event_recording_session)

// Events recording macros
#define EVENTS_START_FOR_LOOP() ukRecordEvent(G_event_recording_session, FOR_LOOP_START_ID, 0.0, __FILE__, __FUNCTION__, __LINE__)
#define EVENTS_END_FOR_LOOP()   ukRecordEvent(G_event_recording_session, FOR_LOOP_END_ID,   0.0, __FILE__, __FUNCTION__, __LINE__)
#define EVENTS_START_PRINTF()   ukRecordEvent(G_event_recording_session, PRINTF_START_ID,   0.0, __FILE__, __FUNCTION__, __LINE__)
#define EVENTS_END_PRINTF()     ukRecordEvent(G_event_recording_session, PRINTF_END_ID,     0.0, __FILE__, __FUNCTION__, __LINE__)

#else

// Compile out all event recording macros
#define EVENTS_GLOBAL_INSTANCE
#define EVENTS_INIT(filename, max_events, flush_when_full, is_threaded, record_instance, record_value, record_location)
#define EVENTS_FLUSH()
#define EVENTS_FINALIZE()
//#define EVENTS_FOLDER1()
//#define EVENTS_FOLDER2()
//#define EVENTS_CLOSE_FOLDER()
#define EVENTS_START_FOR_LOOP()
#define EVENTS_END_FOR_LOOP()
#define EVENTS_START_PRINTF()
#define EVENTS_END_PRINTF()

#endif
#endif
