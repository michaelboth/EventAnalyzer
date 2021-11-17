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

#ifndef _EVENT_INSTRUMENTING_H_
#define _EVENT_INSTRUMENTING_H_

#include "unikorn.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern void *G_instance;

extern void initEventIntrumenting(const char *filename, uint32_t max_events, bool flush_when_full, bool is_threaded, bool record_instance, bool record_value, bool record_location,
				  uint16_t num_folders, UkFolderInfo *folder_info_list, uint16_t num_event_types, UkEventInfo *event_info_list);
extern void finalizeEventIntrumenting();

#ifdef __cplusplus
}
#endif

#endif
