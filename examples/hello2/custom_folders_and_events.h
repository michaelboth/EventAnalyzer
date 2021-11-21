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
  QUICK_SORT_START_ID=1,
  QUICK_SORT_END_ID,
  BUBBLE_SORT_START_ID,
  BUBBLE_SORT_END_ID
};

// IMPORTANT: Call #define DEFINE_FOLDERS_AND_EVENTS, just before #include "event_instrumenting.h", in the file that calls EVENTS_INIT()
#ifdef DEFINE_FOLDERS_AND_EVENTS
//static UkFolderInfo L_folders[] = {
//  { "Folder 1", FOLDER1_ID},
//  { "Folder 2", FOLDER2_ID}
//};
static UkEventInfo L_events[] = {
  { "Quick Sort",  QUICK_SORT_START_ID,  QUICK_SORT_END_ID,  UK_BLUE},
  { "Bubble Sort", BUBBLE_SORT_START_ID, BUBBLE_SORT_END_ID, UK_RED}
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
#define EVENTS_START_QUICK_SORT()  ukRecordEvent(G_instance, QUICK_SORT_START_ID,  0.0, __FILE__, __FUNCTION__, __LINE__)
#define EVENTS_END_QUICK_SORT()    ukRecordEvent(G_instance, QUICK_SORT_END_ID,    0.0, __FILE__, __FUNCTION__, __LINE__)
#define EVENTS_START_BUBBLE_SORT() ukRecordEvent(G_instance, BUBBLE_SORT_START_ID, 0.0, __FILE__, __FUNCTION__, __LINE__)
#define EVENTS_END_BUBBLE_SORT()   ukRecordEvent(G_instance, BUBBLE_SORT_END_ID,   0.0, __FILE__, __FUNCTION__, __LINE__)

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
#define EVENTS_START_QUICK_SORT()
#define EVENTS_END_QUICK_SORT()
#define EVENTS_START_BUBBLE_SORT()
#define EVENTS_END_BUBBLE_SORT()

#endif
#endif
