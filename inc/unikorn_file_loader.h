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

#ifndef _UNIKORN_FILE_LOADER_H_
#define _UNIKORN_FILE_LOADER_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
  uint16_t id;
  char *name;
} UkLoaderFolderRegistration;

typedef struct {
  uint16_t start_id;
  uint16_t end_id;
  uint16_t rgb;
  char *name;
  char *start_value_name;
  char *end_value_name;
} UkLoaderEventRegistration;

typedef struct {
  uint64_t time;     // Nanoseconds since clock started (each clock has a different base time)
  uint16_t event_id; // Could also be a folder ID
  uint64_t instance;
  double value;
  uint16_t thread_index;
  uint16_t file_name_index;
  uint16_t function_name_index;
  uint16_t line_number;
} UkEvent;

typedef struct {
  // Header (should be same for each flush)
  uint16_t version_major;
  uint16_t version_minor;
  bool is_multi_threaded;
  bool includes_instance;
  bool includes_value;
  bool includes_file_location;
  uint16_t folder_registration_count;
  UkLoaderFolderRegistration *folder_registration_list;
  uint16_t event_registration_count;
  UkLoaderEventRegistration *event_registration_list;

  // Recorded Events (different for each flush)
  uint16_t file_name_count;
  char **file_name_list;
  uint16_t function_name_count;
  char **function_name_list;
  uint16_t thread_id_count;
  uint64_t *thread_id_list;
  uint32_t event_count;
  UkEvent *event_buffer;
} UkEvents;

#ifdef __cplusplus
extern "C"
{
#endif

extern UkEvents *ukLoadEventsFile(const char *filename);
extern void ukFreeEvents(UkEvents *instance);

#ifdef __cplusplus
}
#endif

#endif
