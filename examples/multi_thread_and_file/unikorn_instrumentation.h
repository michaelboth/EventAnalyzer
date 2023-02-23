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

#ifndef _UNIKORN_INSTRUMENTATION_H_
#define _UNIKORN_INSTRUMENTATION_H_

#ifdef ENABLE_UNIKORN_RECORDING

#include "unikorn.h"
#include "unikorn_clock.h"       // Provide your own clock functionality if you don't want to use one of the clocks provided with the Unikorn distribution
#include "unikorn_file_flush.h"  // Provide your own flush functionality (e.g. socket) here if you don't want to flush to a file
#include <stdlib.h>

// Define the unique IDs for the folders and events
enum {  // IMPORTANT, IDs must start with 1 since 0 is reserved for 'close folder'
  // Folders   (not required to have any folders)
  //  FOLDER1_ID=1,
  //  FOLDER2_ID,
  // Events   (must have at least one start/end ID combo)
  ALLOC_START_ID=1,
  ALLOC_END_ID,
  FREE_START_ID,
  FREE_END_ID,
  INIT_THREADS_START_ID,
  INIT_THREADS_END_ID,
  JOIN_THREADS_START_ID,
  JOIN_THREADS_END_ID,
  BARRIER_START_ID,
  BARRIER_END_ID,
  SQRT_START_ID,
  SQRT_END_ID
};

// IMPORTANT: Call #define ENABLE_UNIKORN_SESSION_CREATION, just before #include "unikorn_instrumentation.h", in the file that calls UNIKORN_INIT()
#ifdef ENABLE_UNIKORN_SESSION_CREATION

// Define custom folders
//static UkFolderRegistration L_folders[] = {
//    Name        ID
//  { "Folder 1", FOLDER1_ID},
//  { "Folder 2", FOLDER2_ID}
//   IMPORTANT: This folder registration list must be in the same order as the folder ID enumerations above
//};
//#define NUM_FOLDERS (sizeof(L_folders) / sizeof(UkFolderRegistration))
#define L_folders NULL
#define NUM_FOLDERS 0

// Define custom events
static UkEventRegistration L_events[] = {
  // Name                  Color      Start ID               End ID                Start Value Name  End Value Name
  { "Allocate Resources",  UK_PURPLE, ALLOC_START_ID,        ALLOC_END_ID,         "",               ""},
  { "Free Resources",      UK_PURPLE, FREE_START_ID,         FREE_END_ID,          "",               ""},
  { "Start Threads",       UK_GREEN,  INIT_THREADS_START_ID, INIT_THREADS_END_ID,  "Thread Count",   ""},
  { "Join Threads",        UK_GREEN,  JOIN_THREADS_START_ID, JOIN_THREADS_END_ID,  "Thread Count",   ""},
  { "Barrier",             UK_RED,    BARRIER_START_ID,      BARRIER_END_ID,       "",               ""},
  { "Sqrt()",              UK_BLUE,   SQRT_START_ID,         SQRT_END_ID,          "Iteration",      "Vector Size"}
  // IMPORTANT: This event registration list must be in the same order as the event ID enumerations above
};
#define NUM_EVENT_REGISTRATIONS (sizeof(L_events) / sizeof(UkEventRegistration))

// Init the event session
void *UK_CREATE(const char *_filename, uint32_t _max_events, bool _flush_when_full, bool _is_multi_threaded, bool _record_instance, bool _record_value, bool _record_location, UkFileFlushInfo *_flush_info) {
  UkAttrs attrs = {
    .max_event_count = _max_events,
    .flush_when_full = _flush_when_full,
    .is_multi_threaded = _is_multi_threaded,
    .record_instance = _record_instance,
    .record_value = _record_value,
    .record_file_location = _record_location,
    .folder_registration_count = NUM_FOLDERS,
    .folder_registration_list = L_folders,
    .event_registration_count = NUM_EVENT_REGISTRATIONS,
    .event_registration_list = L_events
  };
  _flush_info->filename = _filename;
  _flush_info->file = NULL;
  _flush_info->events_saved = false;
  _flush_info->append_subsequent_saves = true;
  void *session = ukCreate(&attrs, ukGetTime, _flush_info, ukPrepareFileFlush, ukFileFlush, ukFinishFileFlush);
  return session;
}

#endif  // ENABLE_UNIKORN_SESSION_CREATION


// Macros to compile in event recording
#define UK_FLUSH(_session) ukFlush(_session)
#define UK_DESTROY(_session) ukDestroy(_session)
#define UK_OPEN_FOLDER(_session, _folder_id) ukOpenFolder(_session, _folder_id)
#define UK_CLOSE_FOLDER(_session) ukCloseFolder(_session)
#define UK_RECORD_EVENT(_session, _event_id, _value) ukRecordEvent(_session, _event_id, _value, __FILE__, __FUNCTION__, __LINE__)


#else


// Macros to compile out event recording
#define UK_FLUSH(_session)
#define UK_DESTROY(_session)
#define UK_OPEN_FOLDER(_session, _folder_id)
#define UK_CLOSE_FOLDER(_session)
#define UK_RECORD_EVENT(_session, _event_id, _value)

#endif
#endif
