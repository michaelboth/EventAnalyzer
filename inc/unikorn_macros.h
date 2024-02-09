// Copyright 2024 Michael Both
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

#ifndef _UNIKORN_MACROS_H_
#define _UNIKORN_MACROS_H_

// BENEFIT: If macros are used for the Unikorn instrumentation, then it's easy to compile out when not needed
// NOTE: This file assumes flushing to a file is being used via unikorn_file_flush.h

#ifdef ENABLE_UNIKORN_RECORDING

#include "unikorn.h"
#include "unikorn_clock.h"       // Provide your own clock functionality if you don't want to use one of the clocks provided with the Unikorn distribution
#include "unikorn_file_flush.h"  // Provide your own flush functionality (e.g. socket) here if you don't want to flush to a file
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
  #define strdup _strdup
#endif

// Argument types:
//    const char *_filename
//    uint32_t _max_events
//    bool _flush_when_full
//    bool _is_multi_threaded
//    bool _record_instance
//    bool _record_value
//    bool _record_location
//    uint16_t _folder_registration_count
//    UkFolderRegistration *_folder_registration_list
//    uint16_t _event_registration_count
//    UkEventRegistration *_event_registration_list
//    UkFileFlushInfo *_flush_info
//    void **_session_out
#define UK_CREATE(_filename, _max_events, _flush_when_full, _is_multi_threaded, _record_instance, _record_value, _record_location, _folder_registration_count, _folder_registration_list, _event_registration_count, _event_registration_list, _flush_info, _session_out) \
  UkAttrs attrs = { \
    .max_event_count = (_max_events), \
    .flush_when_full = (_flush_when_full), \
    .is_multi_threaded = (_is_multi_threaded), \
    .record_instance = (_record_instance), \
    .record_value = (_record_value), \
    .record_file_location = (_record_location), \
    .folder_registration_count = (_folder_registration_count), \
    .folder_registration_list = (_folder_registration_list), \
    .event_registration_count = (_event_registration_count), \
    .event_registration_list = (_event_registration_list) \
  }; \
  (_flush_info)->filename = strdup(_filename); \
  (_flush_info)->file = NULL; \
  (_flush_info)->events_saved = false; \
  (_flush_info)->append_subsequent_saves = true; \
  *(_session_out) = ukCreate(&attrs, ukGetTime, _flush_info, ukPrepareFileFlush, ukFileFlush, ukFinishFileFlush)

#define UK_DESTROY(_session, _flush_info) ukDestroy(_session); free((_flush_info)->filename)
#define UK_FLUSH(_session) ukFlush(_session)
#define UK_OPEN_FOLDER(_session, _folder_id) ukOpenFolder(_session, _folder_id)
#define UK_CLOSE_FOLDER(_session) ukCloseFolder(_session)
#define UK_RECORD_EVENT(_session, _event_id, _value) ukRecordEvent(_session, _event_id, _value, __FILE__, __FUNCTION__, __LINE__)

#else  // ENABLE_UNIKORN_RECORDING

#define UK_CREATE(_filename, _max_events, _flush_when_full, _is_multi_threaded, _record_instance, _record_value, _record_location, _folder_registration_count, _folder_registration_list, _event_registration_count, _event_registration_list, _flush_info, _session_out)
#define UK_DESTROY(_session, _flush_info)
#define UK_FLUSH(_session)
#define UK_OPEN_FOLDER(_session, _folder_id)
#define UK_CLOSE_FOLDER(_session)
#define UK_RECORD_EVENT(_session, _event_id, _value)

#endif   // ENABLE_UNIKORN_RECORDING

#endif   // _UNIKORN_MACROS_H_
