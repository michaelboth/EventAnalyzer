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

#ifndef _UNIKORN_H_
#define _UNIKORN_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define UK_API_VERSION_MAJOR 1
#define UK_API_VERSION_MINOR 0

// Predefined RGB colors. Application can still use custom color values, format is 0x0RGB
enum {
  UK_RED    = 0x0f00,
  UK_ORANGE = 0x0f70,
  UK_YELLOW = 0x0ff0,
  UK_GREEN  = 0x0090,
  UK_PURPLE = 0x070f,
  UK_BLUE   = 0x000f,
  UK_TEAL   = 0x0088,
  UK_WHITE  = 0x0fff,
  UK_GRAY   = 0x0777,
  UK_BLACK  = 0x0000,
};

typedef struct {
  const char *name;
  uint16_t id;        // ID's must start with 1 and be contiguous across folders (defined first) and events. ID 0 is reserved for 'close folder' event.
} UkFolderInfo;

typedef struct {
  const char *name;
  uint16_t start_id;  // ID's must start with 1 and be contiguous across folders (defined first) and events
  uint16_t end_id;    // ID's must start with 1 and be contiguous across folders (defined first) and events
  uint16_t rgb;       // 0x0RGB
} UkEventInfo;

typedef struct {
  uint32_t max_event_count;     // Max number of events that can be stored before oldest events get overwritten
  bool flush_when_full;         // If true, flushes stored events when event buffer is full, by implicitly calling ukFlush(). This will significantly impact performance.
  bool is_threaded;             // If true, the API will be thread safe and the thread ID is stored in each event. 'true' only allowed if API built with mutex suppoprt.
  bool record_instance;         // If true, will store a counter value (per event type) each time the event is recorded
  bool record_value;            // If true, will store 64 uninterpreted bits (can be interpreted by GUI; e.g. bool, int, int64, float, double, etc)
  bool record_file_location;    // If true, will store the filename and line number to each event
  uint16_t folder_info_count;   // Folders are helpful with grouping events
  UkFolderInfo *folder_info_list;
  uint16_t event_info_count;    // Unique event types that can be recorded
  UkEventInfo *event_info_list;
} UkAttrs;

/* Flush format (stored as binary since millions of events might be stored):
  -------------------------------------------------
  - HEADER: does not change from flush to flush
  -------------------------------------------------
  (bool)           is_big_endian
  (bool)           is_threaded
  (bool)           includes_instance
  (bool)           includes_value
  (bool)           includes_file_location
  (uint16_t)       folder_info_count          (can be zero)
    (uint16_t)       id
    (uint16_t)       num_name_chars
    (char[])         chars
  (uint16_t)       event_info_count           (must be at least one or else this API is pretty useless)
    (uint16_t)       start_id
    (uint16_t)       end_id
    (uint16_t)       rgb_color
    (uint16_t)       num_name_chars
    (char[])         chars
  -------------------------------------------------
  - DATA: may be different with each flush
  -------------------------------------------------
  (uint16_t)       file_name_count            (0 if record_file_location==false)
    (uint16_t)       num_chars
    (char[])         chars
  (uint16_t)       function_name_count        (0 if record_file_location==false)
    (uint16_t)       num_chars
    (char[])         chars
  (uint16_t)       thread_id_count            (0 if is_threaded==false)
    (uint64_t)       thread_id
  (uint16_t)       num_open_folders           (stack of folders that were already open before the first event to be saved)
    (uint16_t)       folder id
  (uint32_t)       event_count
    (uint64_t)       elapsed time since clocks base time
    (uint16_t)       event id
    (uint64_t)       instance                     (only recorded if record_instance==true)
    (uint64_t)       value                        (only recorded if record_value==true)
    (uint16_t)       index in thread list         (only recorded if is_threaded==true)
    (uint16_t)       index in file name list      (only recorded if record_file_location==true)
    (uint16_t)       index in function name list  (only recorded if record_file_location==true)
    (uint16_t)       line number                  (only recorded if record_file_location==true)
*/

/* File suffix
   If an event file is stored, then to ensure portability between event loaders, the file suffix should be .events
*/

/* GUI display requirements
  To provide consistency between event viewers and minimize clutter of displaying events:
    - Start event is a vertical line with thickness == 1, and only fills from the top 1/4 to 1/2 of the line height
    - End event is a vertical line with thickness == 1, and only fills from the bottom 3/4 to 1/2 of the line height
    - The duration between the start and end events: width is fron the start to the end lines, the height is the middle 1/3 of the line height
    - If there is overlap, then fill the height of the line
*/

#ifdef __cplusplus
extern "C"
{
#endif

void *ukCreate(UkAttrs *attrs,
	       uint64_t (*clock)(),     // Application defined clock returning elapsed nanoseconds from some base time; e.g. base time could be zero at the time of this call.
	       void *flush_user_data,                    // Application defined object passed to flush functions. E.g. Socket or file name.
	       bool (*prepareFlush)(void *user_data),    // Prepare application object for flushing events. E.g. open a file or socket
	       bool (*flush)(void *user_data, const void *data, size_t bytes), // Flush part of event data. This will be called many times to complete the flush of all the events
	       bool (*finishFlush)(void *user_data)      // Done with flushing. Use this to close the file or socket if needed
	       );
void ukDestroy(void *instance);  // Will not automatically save unsaved events

// The remaining functions are thread safe if the library is compiled with THREADED=Yes and UkAttrs::is_threaded==true

// Record event
void ukRecordEvent(void *instance, uint16_t event_id, double value, const char *file, const char *function, uint16_t line_number);

// Open a folder to contain any subsequent events that are recorded
void ukRecordFolder(void *instance, uint16_t folder_id); // Open folder within current folder, and make this the current folder

// Close the previously opened folder
void ukCloseFolder(void *instance); // Close the last folder that was opened

// Push recorded events to app defined object, and then mark the event buffer as empty
void ukFlush(void *instance);

#ifdef __cplusplus
}
#endif

#endif
