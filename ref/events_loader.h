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

#ifndef _EVENTS_LOADER_H_
#define _EVENTS_LOADER_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
  uint16_t id;
  char *name;
} FolderInfo;

typedef struct {
  uint16_t start_id;
  uint16_t end_id;
  uint16_t rgb;
  char *name;
} EventInfo;

typedef struct {
  uint64_t time;
  uint16_t event_id;
  uint64_t instance;
  double value;
  uint16_t thread_index;
  uint16_t file_name_index;
  uint16_t function_name_index;
  uint16_t line_number;
} Event;

typedef struct {
  // Header
  bool is_threaded;
  uint16_t version_major;
  uint16_t version_minor;
  bool includes_instance;
  bool includes_value;
  bool includes_file_location;
  uint16_t folder_info_count;
  FolderInfo *folder_info_list;
  uint16_t event_info_count;
  EventInfo *event_info_list;
  // Recorded Events
  uint16_t file_name_count;
  char **file_name_list;
  uint16_t function_name_count;
  char **function_name_list;
  uint16_t thread_id_count;
  uint64_t *thread_id_list;
  uint16_t num_open_folders; // Stack of folders that were already open before the first event to be saved
  uint16_t *folder_id_list;
  uint32_t event_count;
  Event *event_buffer;
} Events;

#ifdef __cplusplus
extern "C"
{
#endif

extern Events *loadEventsFile(const char *filename);
extern void freeEvents(Events *instance);

#ifdef __cplusplus
}
#endif

#endif
