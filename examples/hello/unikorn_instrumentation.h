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

#ifndef _UNIKORN_INSTRUMENTATION_H_
#define _UNIKORN_INSTRUMENTATION_H_

#ifdef ENABLE_UNIKORN_RECORDING

#include "unikorn.h"
#include "unikorn_clock.h"
#include "unikorn_file_flush.h"
#include <stdlib.h>

// Define the unique IDs for the folders and events
enum {  // IMPORTANT, IDs must start with 1 since 0 is reserved for 'close folder'
  // Folders
  //  FOLDER1_ID=1,
  //  FOLDER2_ID,
  // Events
  FOR_LOOP_START_ID=1,
  FOR_LOOP_END_ID,
  PRINTF_START_ID,
  PRINTF_END_ID
};

// IMPORTANT: Call #define ENABLE_UNIKORN_SESSION_CREATION, just before #include "unikorn_instrumentation.h", in the file that calls UNIKORN_INIT()
#ifdef ENABLE_UNIKORN_SESSION_CREATION

// Define custom folders
//static UkFolderInfo L_folders[] = {
//    Name        ID
//  { "Folder 1", FOLDER1_ID},
//  { "Folder 2", FOLDER2_ID}
//};
//#define NUM_FOLDERS (sizeof(L_folders) / sizeof(UkFolderInfo))
#define L_folders NULL
#define NUM_FOLDERS 0

// Define custom events
static UkEventInfo L_events[] = {
  // Name        Color      Start ID            End ID           Start Value Name  End Value Name
  { "For Loop",  UK_BLACK,  FOR_LOOP_START_ID,  FOR_LOOP_END_ID, "",               ""},
  { "printf()",  UK_BLUE,   PRINTF_START_ID,    PRINTF_END_ID,   "Loop Index",     "Loop Index"},
};
#define NUM_EVENT_TYPES (sizeof(L_events) / sizeof(UkEventInfo))

// Init the event session
void *UNIKORN_INIT(const char *_filename, uint32_t _max_events, bool _flush_when_full, bool _is_threaded, bool _record_instance, bool _record_value, bool _record_location, UkFileFlushInfo *_flush_info) {
  UkAttrs attrs = {
    .max_event_count = _max_events,
    .flush_when_full = _flush_when_full,
    .is_threaded = _is_threaded,
    .record_instance = _record_instance,
    .record_value = _record_value,
    .record_file_location = _record_location,
    .folder_info_count = NUM_FOLDERS,
    .folder_info_list = L_folders,
    .event_info_count = NUM_EVENT_TYPES,
    .event_info_list = L_events
  };
  _flush_info->filename = _filename;
  _flush_info->file = NULL;
  _flush_info->events_saved = false;
  _flush_info->append_subsequent_saves = true;
  void *session = ukCreate(&attrs, ukGetTime, _flush_info, ukPrepareFileFlush, ukFileFlush, ukFinishFileFlush);
  return session;
}

#endif  // ENABLE_UNIKORN_SESSION_CREATION


// Cleanup functions
#define UNIKORN_FLUSH(_session) ukFlush(_session)
#define UNIKORN_FINALIZE(_session) ukDestroy(_session)

// Folder recording macros
//#define UNIKORN_OPEN_FOLDER1(_session) ukRecordFolder(_session, FOLDER1_ID)
//#define UNIKORN_OPEN_FOLDER2(_session) ukRecordFolder(_session, FOLDER2_ID)
//#define UNIKORN_CLOSE_FOLDER(_session) ukCloseFolder(_session)

// Events recording macros
#define UNIKORN_START_FOR_LOOP(_session, _value) ukRecordEvent(_session, FOR_LOOP_START_ID, _value, __FILE__, __FUNCTION__, __LINE__)
#define UNIKORN_END_FOR_LOOP(_session, _value)   ukRecordEvent(_session, FOR_LOOP_END_ID,   _value, __FILE__, __FUNCTION__, __LINE__)
#define UNIKORN_START_PRINTF(_session, _value)   ukRecordEvent(_session, PRINTF_START_ID,   _value, __FILE__, __FUNCTION__, __LINE__)
#define UNIKORN_END_PRINTF(_session, _value)     ukRecordEvent(_session, PRINTF_END_ID,     _value, __FILE__, __FUNCTION__, __LINE__)


#else


// Compile out all event recording macros
#define UNIKORN_FLUSH(_session)
#define UNIKORN_FINALIZE(_session)
//#define UNIKORN_OPEN_FOLDER1(_session)
//#define UNIKORN_OPEN_FOLDER2(_session)
//#define UNIKORN_CLOSE_FOLDER(_session)
#define UNIKORN_START_FOR_LOOP(_session, _value)
#define UNIKORN_END_FOR_LOOP(_session, _value)
#define UNIKORN_START_PRINTF(_session, _value)
#define UNIKORN_END_PRINTF(_session, _value)

#endif
#endif