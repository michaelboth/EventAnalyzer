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

#include "unikorn.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#ifdef ALLOW_THREADING
  #ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
  #else
    #include <unistd.h>       // For syscall()
    #include <sys/syscall.h>  // For SYS_gettid
  #endif
  #include <pthread.h>
#endif
#ifdef _WIN32
  #define strdup _strdup
#endif
#ifdef __APPLE__
  #define UINT64_FORMAT "llu"
#else
  #define UINT64_FORMAT "zu"
#endif

// Memory layout of the event buffer
//     - Time                   sizeof(uint64_t)
//     - Event ID               sizeof(uint16_t)
//     - Instance               sizeof(uint64_t)  (only if record_instance==true)          Number of times the event ID was stored
//     - Value                  sizeof(double)    (only if store_value == true)            64bit float value
//     - Thread ID              sizeof(uint64_t)  (only if is_threaded == true)            Thread ID (can be used as a folder in the GUI)
//     - File Name Pointer      sizeof(char *)    (only if record_file_location == true)   Resolves to the name of the file where the event was stored
//     - Function Name Pointer  sizeof(char *)    (only if record_file_location == true)   Resolves to the name of the function where the event was stored
//     - Line number            sizeof(uint16_t)  (only if record_file_location == true)   Line number in the file where the event was stored

#define MAX_NAME_LENGTH 100      // Don't want event and folder names to get unruly, but this can increase without changing the spec
#define MIN_EVENT_COUNT 10       // Need some reasonable min
#define MAGIC_VALUE1 123456789   // Use to partically validate the data structure
#define MAGIC_VALUE2 987654321   // Use to partically validate the data structure
#define CLOSE_FOLDER_ID 0        // Reserved ID
#define INITIAL_LIST_SIZE 10
//*+*/#define ENABLE_ASSERTS only use with recording functions

//#define PRINT_INIT_INFO
//#define PRINT_FLUSH_INFO
//#define PRINT_RECORD_INFO

typedef struct {
  char *name;
  uint16_t id;        // ID's must start with 1 and be contiguous across folders (defined first) and events
  uint64_t instance;  // Number of times the folder was used
} PrivateFolderInfo;

typedef struct {
  char *name;
  uint16_t start_id;  // ID's must start with 1 and be contiguous across folders (defined first) and events
  uint16_t end_id;    // ID's must start with 1 and be contiguous across folders (defined first) and events
  uint16_t rgb;       // 0x0RGB
  uint64_t instance;  // Number of times the event was used
} PrivateEventInfo;

typedef struct {
  size_t time;
  size_t event_id;
  size_t instance;
  size_t value;
  size_t thread_id;
  size_t file_name;
  size_t function_name;
  size_t line_number;
} EventOffsets;

typedef struct {
  uint32_t magic_value1;
  // User defined functions
  uint64_t (*clock)();
  void *flush_user_data;
  bool (*prepareFlush)(void *user_data);
  bool (*flush)(void *user_data, const void *data, size_t bytes);
  bool (*finishFlush)(void *user_data);
  // Recording attributes
  bool flush_when_full;
  bool is_threaded;
  bool record_instance;
  bool record_file_location;
  bool record_value;
  size_t bytes_per_event;
  EventOffsets offsets;
  // Folders
  uint16_t folder_info_count;
  PrivateFolderInfo *folder_info_list;
  uint16_t curr_folder_stack_count;
  uint16_t *curr_folder_stack;
  uint16_t starting_folder_stack_count;
  uint16_t *starting_folder_stack;
  // Event types
  uint16_t first_event_id;
  uint16_t event_info_count;
  PrivateEventInfo *event_info_list;
  // Event buffer
  uint32_t max_event_count;
  uint32_t num_stored_events;
  uint32_t curr_event_index;
  uint32_t first_unsaved_event_index;
  void *event_buffer;
#ifdef ALLOW_THREADING
  pthread_mutex_t mutex;
#endif
  uint32_t magic_value2;
} EventObject;

static const char *L_unused_name = "N/A";

static bool isBigEndian() {
  uint32_t a = 1;
  unsigned char *b = (unsigned char *)&a;
  return (b[3] == 1);
}

#ifdef ALLOW_THREADING
static uint64_t myThreadId() {
  // IMPORTANT: Can't use pthread_self() because it's opaque
#ifdef _WIN32
  DWORD tid = GetCurrentThreadId();
#elif __APPLE__
  uint64_t tid;
  pthread_threadid_np(NULL, &tid);
#else
  pid_t tid = syscall(SYS_gettid);
#endif
  return tid;
}
#endif

static bool containsName(char **name_list, uint16_t name_count, char *name) {
  for (uint16_t i=0; i<name_count; i++) {
    if (name == name_list[i]) return true;
  }
  return false;
}

static char **getFileNameList(EventObject *object, uint16_t *count_ret) {
  uint16_t name_count = 0;
  uint16_t max_name_count = INITIAL_LIST_SIZE;
  char **file_name_list = malloc(max_name_count*sizeof(char *));
  assert(file_name_list);

  uint32_t index = object->first_unsaved_event_index;
  size_t offset = object->offsets.file_name;
  for (uint32_t i=0; i<object->num_stored_events; i++) {
    char **name_list = (char **)((size_t)object->event_buffer + index * object->bytes_per_event + offset);
    char *name = name_list[0];
    if (!containsName(file_name_list, name_count, name)) {
      if (name_count == max_name_count) {
	assert(max_name_count <= (USHRT_MAX/2));
	max_name_count *= 2;
	file_name_list = realloc(file_name_list, max_name_count*sizeof(char *));
	assert(file_name_list);
      }
      file_name_list[name_count] = name;
      name_count++;
    }
    index = (index + 1) % object->max_event_count;
  }

  *count_ret = name_count;
  return file_name_list;
}

static char **getFunctionNameList(EventObject *object, uint16_t *count_ret) {
  uint16_t name_count = 0;
  uint16_t max_name_count = INITIAL_LIST_SIZE;
  char **function_name_list = malloc(max_name_count*sizeof(char *));
  assert(function_name_list);

  uint32_t index = object->first_unsaved_event_index;
  size_t offset = object->offsets.function_name;
  for (uint32_t i=0; i<object->num_stored_events; i++) {
    char **name_list = (char **)((size_t)object->event_buffer + index * object->bytes_per_event + offset);
    char *name = name_list[0];
    if (!containsName(function_name_list, name_count, name)) {
      if (name_count == max_name_count) {
	assert(max_name_count <= (USHRT_MAX/2));
	max_name_count *= 2;
	function_name_list = realloc(function_name_list, max_name_count*sizeof(char *));
	assert(function_name_list);
      }
      function_name_list[name_count] = name;
      name_count++;
    }
    index = (index + 1) % object->max_event_count;
  }

  *count_ret = name_count;
  return function_name_list;
}

static bool containsValue(uint64_t *value_list, uint16_t value_count, uint64_t value) {
  for (uint16_t i=0; i<value_count; i++) {
    if (value == value_list[i]) return true;
  }
  return false;
}

static uint64_t *getThreadIdList(EventObject *object, uint16_t *count_ret) {
  uint16_t id_count = 0;
  uint16_t max_id_count = INITIAL_LIST_SIZE;
  uint64_t *thread_id_list = malloc(max_id_count*sizeof(uint64_t));
  assert(thread_id_list);

  uint32_t index = object->first_unsaved_event_index;
  size_t offset = object->offsets.thread_id;
  for (uint32_t i=0; i<object->num_stored_events; i++) {
    uint64_t *thread_id_ptr = (uint64_t *)((size_t)object->event_buffer + index * object->bytes_per_event + offset);
    uint64_t thread_id;
    memcpy(&thread_id, thread_id_ptr, sizeof(uint64_t)); // Copying to local address to make sure alignment is good
    if (!containsValue(thread_id_list, id_count, thread_id)) {
      if (id_count == max_id_count) {
	assert(max_id_count <= (USHRT_MAX/2));
	max_id_count *= 2;
	thread_id_list = realloc(thread_id_list, max_id_count*sizeof(uint64_t));
	assert(thread_id_list);
      }
      thread_id_list[id_count] = thread_id;
      id_count++;
    }
    index = (index + 1) % object->max_event_count;
  }

  *count_ret = id_count;
  return thread_id_list;
}

static uint16_t getThreadIndex(uint64_t thread_id, uint64_t *thread_id_list, uint16_t thread_id_count) {
  for (uint16_t i=0; i<thread_id_count; i++) {
    if (thread_id == thread_id_list[i]) return i;
  }
  assert(0);
  return 0;
}

static uint16_t getNameIndex(char *name, char **name_list, uint16_t name_count) {
  for (uint16_t i=0; i<name_count; i++) {
    if (name == name_list[i]) return i;
  }
  assert(0);
  return 0;
}

static void pushCurrentFolderStack(EventObject *object, uint16_t folder_id) {
  uint16_t max_folder_stack_count = object->folder_info_count - 1; // -1 for the close folder event
  assert(object->curr_folder_stack_count < max_folder_stack_count);
  for (uint16_t i=0; i<object->curr_folder_stack_count; i++) {
    assert(object->curr_folder_stack[i] != folder_id);
  }
  object->curr_folder_stack[object->curr_folder_stack_count] = folder_id;
  object->curr_folder_stack_count++;
}

static void popCurrentFolderStack(EventObject *object) {
  assert(object->curr_folder_stack_count > 0);
  object->curr_folder_stack_count--;
}

static void pushStartingFolderStack(EventObject *object, uint16_t folder_id) {
  uint16_t max_folder_stack_count = object->folder_info_count - 1; // -1 for the close folder event
  assert(object->starting_folder_stack_count < max_folder_stack_count);
  for (uint16_t i=0; i<object->starting_folder_stack_count; i++) {
    assert(object->starting_folder_stack[i] != folder_id);
  }
  object->starting_folder_stack[object->starting_folder_stack_count] = folder_id;
  object->starting_folder_stack_count++;
}

static void popStartingFolderStack(EventObject *object) {
  assert(object->starting_folder_stack_count > 0);
  object->starting_folder_stack_count--;
}

void *ukCreate(UkAttrs *attrs,
	       uint64_t (*clock)(),
	       void *flush_user_data,
	       bool (*prepareFlush)(void *user_data),
	       bool (*flush)(void *user_data, const void *data, size_t bytes),
	       bool (*finishFlush)(void *user_data)) {
  // Verify attributes
  assert(attrs->max_event_count >= MIN_EVENT_COUNT);
  assert(attrs->event_info_count > 0);
#ifdef ALLOW_THREADING
#else
  assert(!attrs->is_threaded);
#endif
  uint32_t num_event_types = 1;
  for (uint16_t i=0; i<attrs->folder_info_count; i++) {
    assert(attrs->folder_info_list[i].name != NULL && strlen(attrs->folder_info_list[i].name) < MAX_NAME_LENGTH);
    assert(attrs->folder_info_list[i].id == num_event_types);
    num_event_types++;
  }
  uint16_t first_event_id = num_event_types;
  for (uint16_t i=0; i<attrs->event_info_count; i++) {
    assert(attrs->event_info_list[i].name != NULL && strlen(attrs->event_info_list[i].name) < MAX_NAME_LENGTH);
    assert(attrs->event_info_list[i].start_id == num_event_types);
    num_event_types++;
    assert(attrs->event_info_list[i].end_id == num_event_types);
    num_event_types++;
  }

  // Build object
  EventObject *object = calloc(1, sizeof(EventObject));
  assert(object != NULL);
  object->magic_value1 = MAGIC_VALUE1;
  object->magic_value2 = MAGIC_VALUE2;
  object->clock = clock;
  object->flush_user_data = flush_user_data;
  object->prepareFlush = prepareFlush;
  object->flush = flush;
  object->finishFlush = finishFlush;
  object->max_event_count = attrs->max_event_count;
  object->flush_when_full = attrs->flush_when_full;
  object->is_threaded = attrs->is_threaded;
  object->record_instance = attrs->record_instance;
  object->record_value = attrs->record_value;
  object->record_file_location = attrs->record_file_location;
  object->folder_info_count = (attrs->folder_info_count == 0) ? 0 : attrs->folder_info_count + 1; // Also need the close folder event
  object->event_info_count = attrs->event_info_count;
  object->first_event_id = first_event_id;
#ifdef ALLOW_THREADING
  pthread_mutex_init(&object->mutex, NULL);
#endif

  // Calculate bytes and offset per event
  object->bytes_per_event = 0;
  object->offsets.time = object->bytes_per_event;
  object->bytes_per_event += sizeof(uint64_t);   // Time
  object->offsets.event_id = object->bytes_per_event;
  object->bytes_per_event += sizeof(uint16_t);   // Event ID
  if (object->record_instance) {
    object->offsets.instance = object->bytes_per_event;
    object->bytes_per_event += sizeof(uint64_t);   // Instance
  }
  if (object->record_value) {
    object->offsets.value = object->bytes_per_event;
    object->bytes_per_event += sizeof(double);   // Value
  }
  if (object->is_threaded) {
    object->offsets.thread_id = object->bytes_per_event;
    object->bytes_per_event += sizeof(uint64_t);   // Thread ID
  }
  if (object->record_file_location) {
    object->offsets.file_name = object->bytes_per_event;
    object->bytes_per_event += sizeof(char *);     // File name pointer
    object->offsets.function_name = object->bytes_per_event;
    object->bytes_per_event += sizeof(char *);     // Function name pointer
    object->offsets.line_number = object->bytes_per_event;
    object->bytes_per_event += sizeof(uint32_t);   // Line number
  }
#ifdef PRINT_INIT_INFO
  printf("%s():\n", __FUNCTION__);
  printf("  max_event_count = %d\n", object->max_event_count);
  printf("  flush_when_full = %s\n", object->flush_when_full ? "yes" : "no");
  printf("  is_threaded = %s\n", object->is_threaded ? "yes" : "no");
  printf("  record_instance = %s\n", object->record_instance ? "yes" : "no");
  printf("  record_value = %s\n", object->record_value ? "yes" : "no");
  printf("  record_file_location = %s\n", object->record_file_location ? "yes" : "no");
  printf("  first_event_id = %d\n", object->first_event_id);
  printf("  bytes_per_event = %zd\n", object->bytes_per_event);
#endif

  // Named Folders
#ifdef PRINT_INIT_INFO
  printf("  folder_info_count = %d\n", object->folder_info_count);
#endif
  if (object->folder_info_count > 0) {
    object->folder_info_list = calloc(object->folder_info_count, sizeof(PrivateFolderInfo));
    assert(object->folder_info_list != NULL);
    // Register the CLOSE_FOLDER_ID event
    object->folder_info_list[0].id = CLOSE_FOLDER_ID;
    object->folder_info_list[0].name = strdup("Close Folder");
#ifdef PRINT_INIT_INFO
    printf("    ID=%d, name='%s'\n", object->folder_info_list[0].id, object->folder_info_list[0].name);
#endif
    assert(object->folder_info_list[0].name != NULL);
    object->folder_info_list[0].instance = 1;
    // Register custom folders
    for (uint16_t i=1; i<object->folder_info_count; i++) {
      object->folder_info_list[i].id = attrs->folder_info_list[i-1].id;
      object->folder_info_list[i].name = strdup(attrs->folder_info_list[i-1].name);
#ifdef PRINT_INIT_INFO
      printf("    ID=%d, name='%s'\n", object->folder_info_list[i].id, object->folder_info_list[i].name);
#endif
      assert(object->folder_info_list[i].name != NULL);
      object->folder_info_list[i].instance = 1;
    }
    // Create the stacks to track what folders are open
    uint16_t max_folder_stack_count = object->folder_info_count - 1; // -1 due to the close folder event
    object->curr_folder_stack_count = 0;
    object->curr_folder_stack = calloc(max_folder_stack_count, sizeof(uint16_t));
    object->starting_folder_stack_count = 0;
    object->starting_folder_stack = calloc(max_folder_stack_count, sizeof(uint16_t));
  }

  // Named events
#ifdef PRINT_INIT_INFO
  printf("  event_info_count = %d\n", object->event_info_count);
#endif
  object->event_info_list = calloc(object->event_info_count, sizeof(PrivateEventInfo));
  assert(object->event_info_list != NULL);
  // Register custom events
  for (uint16_t i=0; i<object->event_info_count; i++) {
    object->event_info_list[i].start_id = attrs->event_info_list[i].start_id;
    object->event_info_list[i].end_id = attrs->event_info_list[i].end_id;
    object->event_info_list[i].rgb = attrs->event_info_list[i].rgb;
    object->event_info_list[i].name = strdup(attrs->event_info_list[i].name);
#ifdef PRINT_INIT_INFO
    printf("    startID=%d, endID=%d, RGB=0x%04x, name='%s'\n", object->event_info_list[i].start_id, object->event_info_list[i].end_id, object->event_info_list[i].rgb, object->event_info_list[i].name);
#endif
    assert(object->event_info_list[i].name != NULL);
    object->event_info_list[i].instance = 1;
  }

  // Prepare the storage buffer
  object->num_stored_events = 0;
  object->curr_event_index = 0;
  object->first_unsaved_event_index = 0;
  object->event_buffer = malloc(object->max_event_count * object->bytes_per_event);
  assert(object->event_buffer != NULL);

  return object;
}

static void flushEvents(EventObject *object) {
  if (object->num_stored_events == 0) return; // Nothing to flush

  // Make sure the application defined file, socket, etc. is ready for the data
  bool ok = object->prepareFlush(object->flush_user_data);
  assert(ok);

  // Endian
  bool is_big_endian = isBigEndian();
#ifdef PRINT_FLUSH_INFO
  printf("%s():\n", __FUNCTION__);
  printf("  is_big_endian = %s\n", is_big_endian ? "yes" : "no");
#endif
  assert(object->flush(object->flush_user_data, &is_big_endian, sizeof(is_big_endian)));

  /*+ version */

  // Miscellanyous info
#ifdef PRINT_FLUSH_INFO
  printf("  is_threaded = %s\n", object->is_threaded ? "yes" : "no");
  printf("  record_instance = %s\n", object->record_instance ? "yes" : "no");
  printf("  record_value = %s\n", object->record_value ? "yes" : "no");
  printf("  record_file_location = %s\n", object->record_file_location ? "yes" : "no");
#endif
  assert(object->flush(object->flush_user_data, &object->is_threaded, sizeof(object->is_threaded)));
  assert(object->flush(object->flush_user_data, &object->record_instance, sizeof(object->record_instance)));
  assert(object->flush(object->flush_user_data, &object->record_value, sizeof(object->record_value)));
  assert(object->flush(object->flush_user_data, &object->record_file_location, sizeof(object->record_file_location)));

  // Folder info
#ifdef PRINT_FLUSH_INFO
  printf("  folder_info_count = %d\n", object->folder_info_count);
#endif
  assert(object->flush(object->flush_user_data, &object->folder_info_count, sizeof(object->folder_info_count)));
  for (uint16_t i=0; i<object->folder_info_count; i++) {
    PrivateFolderInfo *folder = &object->folder_info_list[i];
#ifdef PRINT_FLUSH_INFO
    printf("    ID=%d, name='%s'\n", folder->id, folder->name);
#endif
    assert(object->flush(object->flush_user_data, &folder->id, sizeof(folder->id)));
    uint16_t num_chars = 1 + (uint16_t)strlen(folder->name);
    assert(object->flush(object->flush_user_data, &num_chars, sizeof(num_chars)));
    assert(object->flush(object->flush_user_data, folder->name, num_chars));
  }

  // Event info
#ifdef PRINT_FLUSH_INFO
  printf("  event_info_count = %d\n", object->event_info_count);
#endif
  assert(object->flush(object->flush_user_data, &object->event_info_count, sizeof(object->event_info_count)));
  for (uint16_t i=0; i<object->event_info_count; i++) {
    PrivateEventInfo *event = &object->event_info_list[i];
#ifdef PRINT_FLUSH_INFO
    printf("    startID=%d, endID=%d, RGB=0x%04x, name='%s'\n", event->start_id, event->end_id, event->rgb, event->name);
#endif
    assert(object->flush(object->flush_user_data, &event->start_id, sizeof(event->start_id)));
    assert(object->flush(object->flush_user_data, &event->end_id, sizeof(event->end_id)));
    assert(object->flush(object->flush_user_data, &event->rgb, sizeof(event->rgb)));
    uint16_t num_chars = 1 + (uint16_t)strlen(event->name);
    assert(object->flush(object->flush_user_data, &num_chars, sizeof(num_chars)));
    assert(object->flush(object->flush_user_data, event->name, num_chars));
  }

  // File names and function names
  uint16_t file_name_count = 0;
  uint16_t function_name_count = 0;
  char **file_name_list = NULL;
  char **function_name_list = NULL;
  if (object->record_file_location) {
    // File names
    file_name_list = getFileNameList(object, &file_name_count);
#ifdef PRINT_FLUSH_INFO
    printf("  file_name_count = %d\n", file_name_count);
#endif
    assert(object->flush(object->flush_user_data, &file_name_count, sizeof(file_name_count)));
    for (uint16_t i=0; i<file_name_count; i++) {
      char *name = file_name_list[i];
#ifdef PRINT_FLUSH_INFO
      printf("    '%s'\n", name);
#endif
      uint16_t num_chars = 1 + (uint16_t)strlen(name);
      assert(object->flush(object->flush_user_data, &num_chars, sizeof(num_chars)));
      assert(object->flush(object->flush_user_data, name, num_chars));
    }
    // Functions names
    function_name_list = getFunctionNameList(object, &function_name_count);
#ifdef PRINT_FLUSH_INFO
    printf("  function_name_count = %d\n", function_name_count);
#endif
    assert(object->flush(object->flush_user_data, &function_name_count, sizeof(function_name_count)));
    for (uint16_t i=0; i<function_name_count; i++) {
      char *name = function_name_list[i];
#ifdef PRINT_FLUSH_INFO
      printf("    '%s'\n", name);
#endif
      uint16_t num_chars = 1 + (uint16_t)strlen(name);
      assert(object->flush(object->flush_user_data, &num_chars, sizeof(num_chars)));
      assert(object->flush(object->flush_user_data, name, num_chars));
    }
  }

  // Build thread ID list
  uint16_t thread_id_count = 0;
  uint64_t *thread_id_list = NULL;
  if (object->is_threaded) {
    thread_id_list = getThreadIdList(object, &thread_id_count);
#ifdef PRINT_FLUSH_INFO
    printf("  thread_id_count = %d\n", thread_id_count);
#endif
    assert(object->flush(object->flush_user_data, &thread_id_count, sizeof(thread_id_count)));
    for (uint16_t i=0; i<thread_id_count; i++) {
      uint64_t thread_id = thread_id_list[i];
#ifdef PRINT_FLUSH_INFO
      printf("    %llu\n", thread_id);
#endif
      assert(object->flush(object->flush_user_data, &thread_id, sizeof(thread_id)));
    }
  }

  // Save list of folders that were open just prior to the first event in the buffer being saved
#ifdef PRINT_FLUSH_INFO
  printf("  Open folders at start of recording = %d\n", object->starting_folder_stack_count);
#endif
  assert(object->flush(object->flush_user_data, &object->starting_folder_stack_count, sizeof(object->starting_folder_stack_count)));
  for (uint16_t i=0; i<object->starting_folder_stack_count; i++) {
#ifdef PRINT_FLUSH_INFO
    printf("    '%s'\n", object->folder_info_list[object->starting_folder_stack[i]].name);
#endif
    assert(object->flush(object->flush_user_data, &object->starting_folder_stack[i], sizeof(object->starting_folder_stack[i])));
  }
  // Now that there are no events in the buffer, need to reset the starting folder stack to be the same as the current folder stack
  object->starting_folder_stack_count = object->curr_folder_stack_count;
  for (uint16_t i=0; i<object->starting_folder_stack_count; i++) {
    object->starting_folder_stack[i] = object->curr_folder_stack[i];
  }

  // Events
#ifdef PRINT_FLUSH_INFO
  printf("  num_stored_events = %d (max count = %d)\n", object->num_stored_events, object->max_event_count);
#endif
  assert(object->flush(object->flush_user_data, &object->num_stored_events, sizeof(object->num_stored_events)));
  uint32_t index = object->first_unsaved_event_index;
  for (uint32_t i=0; i<object->num_stored_events; i++) {
    size_t event_ptr = (size_t)object->event_buffer + index * object->bytes_per_event;

    // Time
    uint64_t time = *((uint64_t *)event_ptr);
    event_ptr += sizeof(time);
    assert(object->flush(object->flush_user_data, &time, sizeof(time)));

    // Event ID
    uint16_t event_id = *((uint16_t *)event_ptr);
    event_ptr += sizeof(event_id);
    assert(object->flush(object->flush_user_data, &event_id, sizeof(event_id)));
#ifdef PRINT_FLUSH_INFO
    printf("    time=%"UINT64_FORMAT", event_id=%d\n", time, event_id);
#endif

    // Instance
    if (object->record_instance) {
      uint64_t event_instance = *((uint64_t *)event_ptr);
      event_ptr += sizeof(event_instance);
      assert(object->flush(object->flush_user_data, &event_instance, sizeof(event_instance)));
#ifdef PRINT_FLUSH_INFO
      printf("    event_instance=%"UINT64_FORMAT"\n", event_instance);
#endif
    }

    // Value
    if (object->record_value) {
      double value = *((double *)event_ptr);
      event_ptr += sizeof(value);
      assert(object->flush(object->flush_user_data, &value, sizeof(value)));
#ifdef PRINT_FLUSH_INFO
      printf("    value=%f\n", value);
#endif
    }

    // Thread ID
    if (object->is_threaded) {
      uint64_t thread_id = *((uint64_t *)event_ptr);
      event_ptr += sizeof(thread_id);
      uint16_t thread_index = getThreadIndex(thread_id, thread_id_list, thread_id_count);
      assert(object->flush(object->flush_user_data, &thread_index, sizeof(thread_index)));
#ifdef PRINT_FLUSH_INFO
      printf("    thread_index=%d\n", thread_index);
#endif
    }

    // Location
    if (object->record_file_location) {
      // File name
      char *file_name = *((char **)event_ptr);
      event_ptr += sizeof(file_name);
      uint16_t file_name_index = getNameIndex(file_name, file_name_list, file_name_count);
      assert(object->flush(object->flush_user_data, &file_name_index, sizeof(file_name_index)));

      // Function name
      char *function_name = *((char **)event_ptr);
      event_ptr += sizeof(function_name);
      uint16_t function_name_index = getNameIndex(function_name, function_name_list, function_name_count);
      assert(object->flush(object->flush_user_data, &function_name_index, sizeof(function_name_index)));

      // Line number
      uint16_t line_number = *((uint16_t *)event_ptr);
      event_ptr += sizeof(line_number);
      assert(object->flush(object->flush_user_data, &line_number, sizeof(line_number)));
#ifdef PRINT_FLUSH_INFO
      printf("    file='%s', function='%s', line=%d\n", file_name, function_name, line_number);
#endif
    }

    index = (index + 1) % object->max_event_count;
  }

  // Reset accounting of the event buffer
  object->num_stored_events = 0;
  object->curr_event_index = 0;
  object->first_unsaved_event_index = 0;

  // Cleanup
  ok = object->finishFlush(object->flush_user_data);
  assert(ok);
  if (file_name_count > 0) free(file_name_list);
  if (function_name_count > 0) free(function_name_list);
  if (thread_id_count > 0) free(thread_id_list);
}

void ukFlush(void *object_ref) {
  EventObject *object = (EventObject *)object_ref;
  assert(object->magic_value1 == MAGIC_VALUE1);
  assert(object->magic_value2 == MAGIC_VALUE2);
#ifdef ALLOW_THREADING
  if (object->is_threaded) pthread_mutex_lock(&object->mutex);
#endif
  flushEvents(object);
#ifdef ALLOW_THREADING
  if (object->is_threaded) pthread_mutex_unlock(&object->mutex);
#endif
}

void ukDestroy(void *object_ref) {
  // NOTE: this should be called after all other threads usiing this object are done
  EventObject *object = (EventObject *)object_ref;
  assert(object->magic_value1 == MAGIC_VALUE1);
  assert(object->magic_value2 == MAGIC_VALUE2);
  if (object->folder_info_count > 0) {
    for (uint16_t i=0; i<object->folder_info_count; i++) {
      free(object->folder_info_list[i].name);
    }
    free(object->folder_info_list);
    free(object->curr_folder_stack);
    free(object->starting_folder_stack);
  }
  if (object->event_info_count > 0) {
    for (uint16_t i=0; i<object->event_info_count; i++) {
      free(object->event_info_list[i].name);
    }
    free(object->event_info_list);
  }
  free(object->event_buffer);
#ifdef ALLOW_THREADING
  pthread_mutex_destroy(&object->mutex);
#endif
  free(object);
}

//#define TEST_OVERHEAD
/*+ optimize recording */
#ifdef TEST_OVERHEAD
static uint64_t getTime() {
  // clock_gettime() has nanosecond precision
  struct timespec curr_time;
  clock_gettime(CLOCK_MONOTONIC, &curr_time); // CLOCK_MONOTONIC is only increasing
  uint64_t total_nanoseconds = ((uint64_t)curr_time.tv_sec * 1000000000) + (uint64_t)curr_time.tv_nsec;
  return total_nanoseconds;

  /*
  // gettimeofday() only has microsecond precision but is more portable
  struct timeval tp;
  gettimeofday(&tp, NULL);
  uint64_t total_nanoseconds = (uint64_t)tp.tv_sec * 1000000000 + (uint64_t)tp.tv_usec * 1000;
  return total_nanoseconds;
  */
}
#endif

/*+ need to optimize this function
  I = instance
  V = value
  T = threaded
  L = location
  recordIVTL
  I
  V
  T
  L
  IV
  IT
  IL
  IVT
  IVL
  IVTL
  VT
  VL
  VTL
  TL
 */
static void recordEvent(EventObject *object, uint16_t event_id, double value, uint64_t instance, const char *file, const char *function, uint16_t line_number) {
#ifdef TEST_OVERHEAD
  uint64_t t1 = getTime();
  uint64_t t2 = getTime();
  uint64_t t3 = getTime();
  uint64_t t4 = getTime();
#endif
  size_t base_addr = (size_t)object->event_buffer + object->curr_event_index * object->bytes_per_event;
  // Get pointers to each item in the event memory
  // Store the values
  uint64_t *time_ptr = (uint64_t *)(base_addr + object->offsets.time);
  *time_ptr = object->clock();
  uint16_t *event_id_ptr = (uint16_t *)(base_addr + object->offsets.event_id);
  uint16_t replaced_event_id = *event_id_ptr; // Need for book keeping at the end of this function
  *event_id_ptr = event_id;
#ifdef TEST_OVERHEAD
  uint64_t t5 = getTime();
#endif
  if (object->record_instance) {
    uint64_t *instance_ptr = (uint64_t *)(base_addr + object->offsets.instance);
    *instance_ptr = instance;
  }
  if (object->record_value) {
    double *value_ptr = (double *)(base_addr + object->offsets.value);
    *value_ptr = value;
  }
#ifdef TEST_OVERHEAD
  uint64_t t6 = getTime();
#endif
#ifdef ALLOW_THREADING
  if (object->is_threaded) {
    uint64_t *thread_id_ptr = (uint64_t *)(base_addr + object->offsets.thread_id);
    *thread_id_ptr = myThreadId();
  }
#endif
#ifdef TEST_OVERHEAD
  uint64_t t7 = getTime();
#endif
  if (object->record_file_location) {
    char **file_name_ptr = (char **)(base_addr + object->offsets.file_name);
    char **function_name_ptr = (char **)(base_addr + object->offsets.function_name);
    uint16_t *line_number_ptr = (uint16_t *)(base_addr + object->offsets.line_number);
    *file_name_ptr = (char *)file;
    *function_name_ptr = (char *)function;
    *line_number_ptr = line_number;
  }
#ifdef TEST_OVERHEAD
  uint64_t t8 = getTime();
#endif

  // Do the book keeping
  object->curr_event_index = (object->curr_event_index + 1) % object->max_event_count;
  bool buffer_is_full = object->num_stored_events == object->max_event_count;
  if (buffer_is_full) {
    // Buffer was already full, must not have auto save enabled
    if (event_id < object->folder_info_count) {
      // This is a folder event
      if (replaced_event_id == CLOSE_FOLDER_ID) {
	popStartingFolderStack(object);
      } else {
	pushStartingFolderStack(object, event_id);
      }
    }
    object->first_unsaved_event_index = (object->first_unsaved_event_index + 1) % object->max_event_count;
  } else {
    // Buffer not yet full
    object->num_stored_events++;
    if (object->flush_when_full && object->num_stored_events == object->max_event_count) {
      flushEvents(object);
    }
  }
#ifdef TEST_OVERHEAD
  uint64_t t9 = getTime();
  printf("time diff 1: %zd ns\n", t2-t1);
  printf("time diff 2: %zd ns\n", t3-t2);
  printf("time diff 3: %zd ns\n", t4-t3);
  printf("time diff 4 A: %zd ns (overhead)\n", t5-t4);
  printf("time diff 5 B: %zd ns (overhead)\n", t6-t5);
  printf("time diff 6 C: %zd ns (overhead)\n", t7-t6);
  printf("time diff 7 D: %zd ns (overhead)\n", t8-t7);
  printf("time diff 8 E: %zd ns (overhead)\n", t9-t8);
#endif
}

void ukRecordEvent(void *object_ref, uint16_t event_id, double value, const char *file, const char *function, uint16_t line_number) {
  EventObject *object = (EventObject *)object_ref;
  assert(object->magic_value1 == MAGIC_VALUE1);
  assert(object->magic_value2 == MAGIC_VALUE2);
#ifdef PRINT_RECORD_INFO
  printf("%s(): ID=%d, value=%f, file=%s, function=%s, line_number=%d\n", __FUNCTION__, event_id, value, file, function, line_number);
#endif
  uint16_t event_index = (event_id - object->first_event_id) / 2;
  assert(event_index < object->event_info_count*2);
#ifdef ALLOW_THREADING
  if (object->is_threaded) pthread_mutex_lock(&object->mutex);
#endif
  PrivateEventInfo *event = &object->event_info_list[event_index];
  uint64_t instance = event->instance++;
  recordEvent(object, event_id, value, instance, file, function, line_number);
#ifdef ALLOW_THREADING
  if (object->is_threaded) pthread_mutex_unlock(&object->mutex);
#endif
}

void ukRecordFolder(void *object_ref, uint16_t folder_id) {
  EventObject *object = (EventObject *)object_ref;
  /*+ separate functions for threaded */
  /*+ hide asserts */
  assert(object->magic_value1 == MAGIC_VALUE1);
  assert(object->magic_value2 == MAGIC_VALUE2);
#ifdef PRINT_RECORD_INFO
  printf("%s(): ID=%d\n", __FUNCTION__, folder_id);
#endif
  assert(object->folder_info_count > 0);
  assert(folder_id >= 1 && folder_id < object->folder_info_count);
  PrivateFolderInfo *folder = &object->folder_info_list[folder_id];
  uint64_t instance = folder->instance++;
  /*+ still record instance at time location if record_instance not true */
#ifdef ALLOW_THREADING
  if (object->is_threaded) pthread_mutex_lock(&object->mutex);
#endif
  pushCurrentFolderStack(object, folder_id);
  /*+ just record time, id , and instance */
  recordEvent(object, folder_id, 0, instance, L_unused_name, L_unused_name, 0);
#ifdef ALLOW_THREADING
  if (object->is_threaded) pthread_mutex_unlock(&object->mutex);
#endif
}

void ukCloseFolder(void *object_ref) {
  EventObject *object = (EventObject *)object_ref;
  assert(object->magic_value1 == MAGIC_VALUE1);
  assert(object->magic_value2 == MAGIC_VALUE2);
#ifdef PRINT_RECORD_INFO
  printf("%s()\n", __FUNCTION__);
#endif
  assert(object->folder_info_count > 0);
  PrivateFolderInfo *folder = &object->folder_info_list[CLOSE_FOLDER_ID];
  uint64_t instance = folder->instance++;
#ifdef ALLOW_THREADING
  if (object->is_threaded) pthread_mutex_lock(&object->mutex);
#endif
  popCurrentFolderStack(object);
  recordEvent(object, CLOSE_FOLDER_ID, 0, instance, L_unused_name, L_unused_name, 0);
#ifdef ALLOW_THREADING
  if (object->is_threaded) pthread_mutex_unlock(&object->mutex);
#endif
}
