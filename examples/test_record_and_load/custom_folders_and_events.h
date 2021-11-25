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
#include "event_recorder_clock.h"
#include "event_recorder_file_flush.h"
#include <stdlib.h>

// Define the unique IDs for the folders and events
enum {  // IMPORTANT, IDs must start with 1 since 0 is reserved for 'close_folder'
  // Folders
  FOLDER1_ID = 1,
  FOLDER2_ID,
  // Events
  PRINT_START_ID,
  PRINT_END_ID,
  SQRT_START_ID,
  SQRT_END_ID
};

// IMPORTANT: Call #define DEFINE_FOLDERS_AND_EVENTS, just before #include "custom_folders_and_events.h", in the file that calls EVENTS_INIT()
#ifdef DEFINE_FOLDERS_AND_EVENTS

// Define custom folders
static UkFolderInfo L_folders[] = {
  // Name       ID
  { "Folder 1", FOLDER1_ID},
  { "Folder 2", FOLDER2_ID}
};
#define NUM_FOLDERS (sizeof(L_folders) / sizeof(UkFolderInfo))
//#define L_folders NULL
//#define NUM_FOLDERS 0

// Define custom events
static UkEventInfo L_events[] = {
  // Name          Start ID              End ID              Color
  { "Print",       PRINT_START_ID,       PRINT_END_ID,       UK_TEAL},
  { "Sqrt",        SQRT_START_ID,        SQRT_END_ID,        UK_BLACK}
};
#define NUM_EVENT_TYPES (sizeof(L_events) / sizeof(UkEventInfo))

// Define the global event session
void *G_event_recording_session = NULL;

// Define a private handle to flushing events to a file
static FileFlushInfo L_flush_info;

// Init the event session
#define EVENTS_INIT(filename, max_events, flush_when_full, is_threaded, record_instance, record_value, record_location) { \
  UkAttrs attrs = { \
    .max_event_count = max_events, \
    .flush_when_full = flush_when_full, \
    .is_threaded = is_threaded, \
    .record_instance = record_instance, \
    .record_value = record_value, \
    .record_file_location = record_location, \
    .folder_info_count = NUM_FOLDERS, \
    .folder_info_list = L_folders, \
    .event_info_count = NUM_EVENT_TYPES, \
    .event_info_list = L_events \
  }; \
  L_flush_info.filename = filename; \
  L_flush_info.file = NULL; \
  L_flush_info.events_saved = false; \
  L_flush_info.append_subsequent_saves = true; \
  G_event_recording_session = ukCreate(&attrs, getEventTime, &L_flush_info, prepareFileFlush, fileFlush, finishFileFlush); \
}

#endif  // DEFINE_FOLDERS_AND_EVENTS

// Macro for the event session: Use this in every file that needs to record events
#define EVENTS_GLOBAL_INSTANCE extern void *G_event_recording_session
// Macro to flush events
#define EVENTS_FLUSH() ukFlush(G_event_recording_session)
// Macro to finalize event recording
#define EVENTS_FINALIZE() ukDestroy(G_event_recording_session); G_event_recording_session = NULL;

// Folder recording macros
#define EVENTS_FOLDER1() ukRecordFolder(G_event_recording_session, FOLDER1_ID)
#define EVENTS_FOLDER2() ukRecordFolder(G_event_recording_session, FOLDER2_ID)
#define EVENTS_CLOSE_FOLDER() ukCloseFolder(G_event_recording_session)

// Events recording macros
#define EVENTS_START_PRINT() ukRecordEvent(G_event_recording_session, PRINT_START_ID, 0.0, __FILE__, __FUNCTION__, __LINE__)
#define EVENTS_END_PRINT()   ukRecordEvent(G_event_recording_session, PRINT_END_ID,   0.0, __FILE__, __FUNCTION__, __LINE__)
#define EVENTS_START_SQRT()  ukRecordEvent(G_event_recording_session, SQRT_START_ID,  0.0, __FILE__, __FUNCTION__, __LINE__)
#define EVENTS_END_SQRT()    ukRecordEvent(G_event_recording_session, SQRT_END_ID,    0.0, __FILE__, __FUNCTION__, __LINE__)

#else

// Compile out all event recording macros
#define EVENTS_GLOBAL_INSTANCE
#define EVENTS_INIT(filename, max_events, flush_when_full, is_threaded, record_instance, record_value, record_location)
#define EVENTS_FLUSH()
#define EVENTS_FINALIZE()
#define EVENTS_FOLDER1()
#define EVENTS_FOLDER2()
#define EVENTS_CLOSE_FOLDER()
#define EVENTS_START_PRINT()
#define EVENTS_END_PRINT()
#define EVENTS_START_SQRT()
#define EVENTS_END_SQRT()

#endif
#endif
