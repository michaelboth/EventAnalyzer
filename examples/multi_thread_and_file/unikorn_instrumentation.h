// Copyright 2021..2024 Michael Both
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

// NOTE: Include this header file in any source file that will use unikorn event intrumenting
#ifdef ENABLE_UNIKORN_RECORDING
#include "unikorn.h"

// ------------------------------------------------
// Define the unique IDs for the folders and events
// ------------------------------------------------
enum {
  // IMPORTANT, IDs must start with 1 since 0 is reserved for 'close folder'
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

// IMPORTANT: Call #define ENABLE_UNIKORN_SESSION_CREATION, just before #include "unikorn_instrumentation.h", in the file that calls UK_CREATE()
#ifdef ENABLE_UNIKORN_SESSION_CREATION

// ------------------------------------------------
// Define custom folders
// ------------------------------------------------
//static UkFolderRegistration L_unikorn_folders[] = {
//    Name        ID
//  { "Folder 1", FOLDER1_ID},
//  { "Folder 2", FOLDER2_ID}
//   IMPORTANT: This folder registration list must be in the same order as the folder ID enumerations above
//};
//#define NUM_UNIKORN_FOLDER_REGISTRATIONS (sizeof(L_unikorn_folders) / sizeof(UkFolderRegistration))
#define L_unikorn_folders NULL
#define NUM_UNIKORN_FOLDER_REGISTRATIONS 0

// ------------------------------------------------
// Define custom events
// ------------------------------------------------
static UkEventRegistration L_unikorn_events[] = {
  // Name                  Color      Start ID               End ID                Start Value Name  End Value Name
  { "Allocate Resources",  UK_PURPLE, ALLOC_START_ID,        ALLOC_END_ID,         "",               ""},
  { "Free Resources",      UK_PURPLE, FREE_START_ID,         FREE_END_ID,          "",               ""},
  { "Start Threads",       UK_GREEN,  INIT_THREADS_START_ID, INIT_THREADS_END_ID,  "Thread Count",   ""},
  { "Join Threads",        UK_GREEN,  JOIN_THREADS_START_ID, JOIN_THREADS_END_ID,  "Thread Count",   ""},
  { "Barrier",             UK_RED,    BARRIER_START_ID,      BARRIER_END_ID,       "",               ""},
  { "Sqrt()",              UK_BLUE,   SQRT_START_ID,         SQRT_END_ID,          "Iteration",      "Vector Size"}
  // IMPORTANT: This event registration list must be in the same order as the event ID enumerations above
};
#define NUM_UNIKORN_EVENT_REGISTRATIONS (sizeof(L_unikorn_events) / sizeof(UkEventRegistration))

#endif // ENABLE_UNIKORN_SESSION_CREATION
#endif // ENABLE_UNIKORN_RECORDING
#endif // _UNIKORN_INSTRUMENTATION_H_
