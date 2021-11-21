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
  // Folders
  //  FOLDER1_ID=1,
  //  FOLDER2_ID,
  // Events
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
  PROCESSING_START_ID,
  PROCESSING_END_ID,
  SQRT_START_ID,
  SQRT_END_ID
};

// IMPORTANT: Call #define DEFINE_FOLDERS_AND_EVENTS, just before #include "event_instrumenting.h", in the file that calls EVENTS_INIT()
#ifdef DEFINE_FOLDERS_AND_EVENTS
//static UkFolderInfo L_folders[] = {
//  { "Folder 1", FOLDER1_ID},
//  { "Folder 2", FOLDER2_ID}
//};
static UkEventInfo L_events[] = {
  { "Allocate Resources", ALLOC_START_ID,        ALLOC_END_ID,        UK_GREEN},
  { "Free Resources",     FREE_START_ID,         FREE_END_ID,         UK_GREEN},
  { "Start Threads",      INIT_THREADS_START_ID, INIT_THREADS_END_ID, UK_GREEN},
  { "Join Threads",       JOIN_THREADS_START_ID, JOIN_THREADS_END_ID, UK_GREEN},
  { "Barrier",            BARRIER_START_ID,      BARRIER_END_ID,      UK_RED},
  { "Processing",         PROCESSING_START_ID,   PROCESSING_END_ID,   UK_BLACK},
  { "Sqrt()",             SQRT_START_ID,         SQRT_END_ID,         UK_BLUE}
};
#endif

// Overall
#define EVENTS_GLOBAL_INSTANCE extern void *G_instance     // Use this in every file that need to record events
#ifdef DEFINE_FOLDERS_AND_EVENTS
  #define EVENTS_INIT(filename, max_events, flush_when_full, is_threaded, record_instance, record_value, record_location) initEventIntrumenting(filename, max_events, flush_when_full, is_threaded, record_instance, record_value, record_location, 0, NULL, sizeof(L_events) / sizeof(UkEventInfo), L_events)
#endif
#define EVENTS_FLUSH() ukFlush(G_instance)
#define EVENTS_FINALIZE() finalizeEventIntrumenting()
// Folders
//#define EVENTS_FOLDER1() ukRecordFolder(G_instance, FOLDER1_ID)
//#define EVENTS_FOLDER2() ukRecordFolder(G_instance, FOLDER2_ID)
//#define EVENTS_CLOSE_FOLDER() ukCloseFolder(G_instance)
// Events
#define EVENTS_START_ALLOC()        ukRecordEvent(G_instance, ALLOC_START_ID,        0.0, __FILE__, __FUNCTION__, __LINE__)
#define EVENTS_END_ALLOC()          ukRecordEvent(G_instance, ALLOC_END_ID,          0.0, __FILE__, __FUNCTION__, __LINE__)
#define EVENTS_START_FREE()         ukRecordEvent(G_instance, FREE_START_ID,         0.0, __FILE__, __FUNCTION__, __LINE__)
#define EVENTS_END_FREE()           ukRecordEvent(G_instance, FREE_END_ID,           0.0, __FILE__, __FUNCTION__, __LINE__)
#define EVENTS_START_INIT_THREADS() ukRecordEvent(G_instance, INIT_THREADS_START_ID, 0.0, __FILE__, __FUNCTION__, __LINE__)
#define EVENTS_END_INIT_THREADS()   ukRecordEvent(G_instance, INIT_THREADS_END_ID,   0.0, __FILE__, __FUNCTION__, __LINE__)
#define EVENTS_START_JOIN_THREADS() ukRecordEvent(G_instance, JOIN_THREADS_START_ID, 0.0, __FILE__, __FUNCTION__, __LINE__)
#define EVENTS_END_JOIN_THREADS()   ukRecordEvent(G_instance, JOIN_THREADS_END_ID,   0.0, __FILE__, __FUNCTION__, __LINE__)
#define EVENTS_START_BARRIER()      ukRecordEvent(G_instance, BARRIER_START_ID,      0.0, __FILE__, __FUNCTION__, __LINE__)
#define EVENTS_END_BARRIER()        ukRecordEvent(G_instance, BARRIER_END_ID,        0.0, __FILE__, __FUNCTION__, __LINE__)
#define EVENTS_START_PROCESSING()   ukRecordEvent(G_instance, PROCESSING_START_ID,   0.0, __FILE__, __FUNCTION__, __LINE__)
#define EVENTS_END_PROCESSING()     ukRecordEvent(G_instance, PROCESSING_END_ID,     0.0, __FILE__, __FUNCTION__, __LINE__)
#define EVENTS_START_SQRT()         ukRecordEvent(G_instance, SQRT_START_ID,         0.0, __FILE__, __FUNCTION__, __LINE__)
#define EVENTS_END_SQRT()           ukRecordEvent(G_instance, SQRT_END_ID,           0.0, __FILE__, __FUNCTION__, __LINE__)

#else

// Overall
#define EVENTS_GLOBAL_INSTANCE
#define EVENTS_INIT(filename, max_events, flush_when_full, is_threaded, record_instance, record_value, record_location)
#define EVENTS_FLUSH()
#define EVENTS_FINALIZE()
// Folders
//#define EVENTS_FOLDER1()
//#define EVENTS_FOLDER2()
//#define EVENTS_CLOSE_FOLDER()
// Events
#define EVENTS_START_ALLOC()
#define EVENTS_END_ALLOC()
#define EVENTS_START_FREE()
#define EVENTS_END_FREE()
#define EVENTS_START_INIT_THREADS()
#define EVENTS_END_INIT_THREADS()
#define EVENTS_START_JOIN_THREADS()
#define EVENTS_END_JOIN_THREADS()
#define EVENTS_START_BARRIER()
#define EVENTS_END_BARRIER()
#define EVENTS_START_PROCESSING()
#define EVENTS_END_PROCESSING()
#define EVENTS_START_SQRT()
#define EVENTS_END_SQRT()

#endif
#endif
