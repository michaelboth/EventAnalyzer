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

#define DEFINE_FOLDERS_AND_EVENTS /*+ remove */
#include "event_instrumenting.h"
#include "event_clocks.h"
#include "event_file_flush.h"
//*+*/#include <stddef.h>

void *G_instance = NULL; // Global to all file; hence 'G_'

static FileFlushInfo L_flush_info; // Global only to this file; hence 'L_'

void initEventIntrumenting(const char *filename, uint32_t max_events, bool flush_when_full, bool is_threaded, bool record_instance, bool record_value, bool record_location) {
  uint16_t num_folders = 0; /*+ pass in L_folders as argument */
  if (L_folders != NULL) { /*+ pass in L_folders as argument */
    num_folders = sizeof(L_folders) / sizeof(UkFolderInfo);
  }
  uint16_t num_event_types = sizeof(L_events) / sizeof(UkEventInfo); /*+ pass in L_folders as argument */

  UkAttrs attrs = {
    .max_event_count = max_events,
    .flush_when_full = flush_when_full,
    .is_threaded = is_threaded,
    .record_instance = record_instance,
    .record_value = record_value,
    .record_file_location = record_location,
    .folder_info_count = num_folders,
    .folder_info_list = L_folders,
    .event_info_count = num_event_types,
    .event_info_list = L_events,
  };

  // Prepare the flush info
  L_flush_info.filename = filename;
  L_flush_info.file = NULL;
  L_flush_info.events_saved = false;
  L_flush_info.append_subsequent_saves = false;

  // Create event session
  G_instance = ukCreate(&attrs, getEventTime, &L_flush_info, prepareFileFlush, fileFlush, finishFileFlush);
}

void finalizeEventIntrumenting() {
  ukDestroy(G_instance);
  G_instance = NULL;
}
