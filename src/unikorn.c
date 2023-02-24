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
#ifdef NDEBUG // Don't want assert compiled out
  #undef NDEBUG
#endif
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#ifdef ENABLE_UNIKORN_ATOMIC_RECORDING
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
//     - Thread ID              sizeof(uint64_t)  (only if is_multi_threaded == true)      Thread ID (can be used as a folder in the GUI)
//     - File Name Pointer      sizeof(char *)    (only if record_file_location == true)   Resolves to the name of the file where the event was stored
//     - Function Name Pointer  sizeof(char *)    (only if record_file_location == true)   Resolves to the name of the function where the event was stored
//     - Line number            sizeof(uint16_t)  (only if record_file_location == true)   Line number in the file where the event was stored

#define MAX_NAME_LENGTH 100      // Don't want event and folder names to get unruly, but this can increase without changing the spec
#define MIN_EVENT_COUNT 10       // Need some reasonable min
#define MAGIC_VALUE1 123456789   // Use to partically validate the data structure
#define MAGIC_VALUE2 987654321   // Use to partically validate the data structure
#define CLOSE_FOLDER_ID 0        // Reserved ID
#define INITIAL_LIST_SIZE 10
#ifdef UNIKORN_RELEASE_BUILD
  #define OPTIONAL_ASSERT(condition)
#else
  #define OPTIONAL_ASSERT(condition) assert(condition)
#endif

//#define PRINT_INIT_INFO
//#define PRINT_FLUSH_INFO
//#define PRINT_RECORD_INFO

// IMPORTANT: Use this to test the performance of recording events
//#define TEST_RECORDING_OVERHEAD

typedef struct {
  char *name;
  uint16_t id;        // ID's must start with 1 and be contiguous across folders (defined first) and events
} PrivateFolderInfo;

typedef struct {
  char *name;
  uint16_t start_id;  // ID's must start with 1 and be contiguous across folders (defined first) and events
  uint16_t end_id;    // ID's must start with 1 and be contiguous across folders (defined first) and events
  uint16_t rgb;       // 0x0RGB
  uint64_t start_instance; // Number of times the start event was used
  uint64_t end_instance;   // Number of times the end event was used
  char *start_value_name;
  char *end_value_name;
} PrivateEventInfo;

typedef struct {
  uint64_t time;
  uint16_t event_id;
  uint64_t instance;
  double value;
  uint64_t thread_id;
  char *file_name;
  char *function_name;
  uint16_t line_number;
} Event;

typedef struct {
  uint32_t magic_value1;
  // User defined functions
  uint64_t (*clockNanoseconds)();
  void *flush_user_data;
  bool (*prepareFlush)(void *user_data);
  bool (*flush)(void *user_data, const void *data, size_t bytes);
  bool (*finishFlush)(void *user_data);
  // Recording attributes
  bool flush_when_full;
  bool is_multi_threaded;
  bool record_instance;
  bool record_file_location;
  bool record_value;
  // Folders
  uint16_t folder_registration_count;
  PrivateFolderInfo *folder_registration_list;
  uint16_t curr_folder_stack_count;
  uint16_t *curr_folder_stack;
  uint16_t starting_folder_stack_count;
  uint16_t *starting_folder_stack;
  // Event types
  uint16_t first_event_id;
  uint16_t event_registration_count;
  PrivateEventInfo *event_registration_list;
  // Event buffer
  uint32_t max_event_count;
  uint32_t num_stored_events;
  uint32_t curr_event_index;
  uint32_t first_unsaved_event_index;
  Event *events_buffer;
  // Thread safety
#ifdef ENABLE_UNIKORN_ATOMIC_RECORDING
  pthread_mutex_t mutex;
#endif
  uint16_t thread_id_list_count;  // This needs to be persistent and growing between flushes as threads come and go
  uint64_t *thread_id_list;       // This needs to be persistent and growing between flushes as threads come and go
  uint32_t magic_value2;
} UnikornSession;

static const char *L_unused_name = "N/A";

static bool isBigEndian() {
  uint32_t a = 1;
  unsigned char *b = (unsigned char *)&a;
  return (b[3] == 1);
}

#ifdef ENABLE_UNIKORN_ATOMIC_RECORDING
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
    // IMPORTANT: need to use strcmp() instead of ==. Can't assume compiler or app will use the same pointer value for __FILE__ or __FUNCTION__ (Microsoft compiler does not)
    if (strcmp(name, name_list[i]) == 0) return true;
  }
  return false;
}

static char **getFileNameList(UnikornSession *session, uint16_t *count_ret) {
  uint16_t name_count = 0;
  uint16_t max_name_count = INITIAL_LIST_SIZE;
  char **file_name_list = malloc(max_name_count*sizeof(char *));
  assert(file_name_list);

  uint32_t index = session->first_unsaved_event_index;
  for (uint32_t i=0; i<session->num_stored_events; i++) {
    char *name = session->events_buffer[index].file_name;
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
    index = (index + 1) % session->max_event_count;
  }

  *count_ret = name_count;
  return file_name_list;
}

static char **getFunctionNameList(UnikornSession *session, uint16_t *count_ret) {
  uint16_t name_count = 0;
  uint16_t max_name_count = INITIAL_LIST_SIZE;
  char **function_name_list = malloc(max_name_count*sizeof(char *));
  assert(function_name_list);

  uint32_t index = session->first_unsaved_event_index;
  for (uint32_t i=0; i<session->num_stored_events; i++) {
    char *name = session->events_buffer[index].function_name;
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
    index = (index + 1) % session->max_event_count;
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

static void getThreadIdList(UnikornSession *session) {
  uint32_t index = session->first_unsaved_event_index;
  for (uint32_t i=0; i<session->num_stored_events; i++) {
    uint64_t thread_id = session->events_buffer[index].thread_id;
    if (!containsValue(session->thread_id_list, session->thread_id_list_count, thread_id)) {
      // Thread ID not in list, add to it
      if (session->thread_id_list_count == USHRT_MAX) {
        printf("Unikorn is only defined to handle up to %d threads.\n", USHRT_MAX);
        assert(0);
      }
      session->thread_id_list_count++;
      session->thread_id_list = realloc(session->thread_id_list, session->thread_id_list_count*sizeof(uint64_t));
      assert(session->thread_id_list);
      session->thread_id_list[session->thread_id_list_count-1] = thread_id;
    }
    index = (index + 1) % session->max_event_count;
  }
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
    // IMPORTANT: need to use strcmp() instead of ==. Can't assume compiler or app will use the same pointer value for __FILE__ or __FUNCTION__ (Microsoft compiler does not)
    if (strcmp(name, name_list[i]) == 0) return i;
  }
  assert(0);
  return 0;
}

static void pushStartingFolderStack(UnikornSession *session, uint16_t folder_id) {
  uint16_t max_folder_stack_count = session->folder_registration_count - 1; // -1 for the close folder event
  assert(session->starting_folder_stack_count < max_folder_stack_count);
  for (uint16_t i=0; i<session->starting_folder_stack_count; i++) {
    assert(session->starting_folder_stack[i] != folder_id);
  }
  session->starting_folder_stack[session->starting_folder_stack_count] = folder_id;
  session->starting_folder_stack_count++;
}

static void popStartingFolderStack(UnikornSession *session) {
  assert(session->starting_folder_stack_count > 0);
  session->starting_folder_stack_count--;
}

void *ukCreate(UkAttrs *attrs,
	       uint64_t (*clockNanoseconds)(),
	       void *flush_user_data,
	       bool (*prepareFlush)(void *user_data),
	       bool (*flush)(void *user_data, const void *data, size_t bytes),
	       bool (*finishFlush)(void *user_data)) {
  // Verify attributes
  if (attrs->max_event_count < MIN_EVENT_COUNT) { printf("Expected max events count=%d to be at least %d\n", attrs->max_event_count, MIN_EVENT_COUNT); assert(0); }
  if (attrs->event_registration_count == 0) { printf("Expected at least one named event to be registered\n"); assert(0); }
#ifdef ENABLE_UNIKORN_ATOMIC_RECORDING
#else
  if (attrs->is_multi_threaded) { printf("Asked for threading, but the library is not compiled with threading.\n"); assert(0); }
#endif
  uint32_t num_event_types = 1;
  for (uint16_t i=0; i<attrs->folder_registration_count; i++) {
    if (attrs->folder_registration_list[i].name == NULL) { printf("Folder name[%d] is NULL\n", i); assert(0); }
    if (strlen(attrs->folder_registration_list[i].name) >= MAX_NAME_LENGTH) { printf("Folder name[%d]='%s' has more than %d chars.\n", i, attrs->folder_registration_list[i].name, MAX_NAME_LENGTH); assert(0); }
    if (attrs->folder_registration_list[i].id != num_event_types) { printf("Folder name[%d]='%s' was expected to have an ID=%d but has %d\n", i, attrs->folder_registration_list[i].name, num_event_types, attrs->folder_registration_list[i].id); assert(0); }
    num_event_types++;
  }
  uint16_t first_event_id = num_event_types;
  for (uint16_t i=0; i<attrs->event_registration_count; i++) {
    if (attrs->event_registration_list[i].name == NULL) { printf("Event name[%d] is NULL\n", i); assert(0); }
    if (strlen(attrs->event_registration_list[i].name) >= MAX_NAME_LENGTH) { printf("Event name[%d]='%s' has more than %d chars.\n", i, attrs->event_registration_list[i].name, MAX_NAME_LENGTH); assert(0); }
    if (attrs->event_registration_list[i].start_id != num_event_types) { printf("Event name[%d]='%s' was expected to have a start ID=%d but has %d\n", i, attrs->event_registration_list[i].name, num_event_types, attrs->event_registration_list[i].start_id); assert(0); }
    num_event_types++;
    if (attrs->event_registration_list[i].end_id != num_event_types) { printf("Event name[%d]='%s' was expected to have an end ID=%d but has %d\n", i, attrs->event_registration_list[i].name, num_event_types, attrs->event_registration_list[i].end_id); assert(0); }
    num_event_types++;
  }

  // Build session
  UnikornSession *session = calloc(1, sizeof(UnikornSession));
  assert(session != NULL);
  session->magic_value1 = MAGIC_VALUE1;
  session->magic_value2 = MAGIC_VALUE2;
  session->clockNanoseconds = clockNanoseconds;
  session->flush_user_data = flush_user_data;
  session->prepareFlush = prepareFlush;
  session->flush = flush;
  session->finishFlush = finishFlush;
  session->max_event_count = attrs->max_event_count;
  session->flush_when_full = attrs->flush_when_full;
  session->is_multi_threaded = attrs->is_multi_threaded;
  session->record_instance = attrs->record_instance;
  session->record_value = attrs->record_value;
  session->record_file_location = attrs->record_file_location;
  session->folder_registration_count = (attrs->folder_registration_count == 0) ? 0 : attrs->folder_registration_count + 1; // Also need the close folder event
  session->event_registration_count = attrs->event_registration_count;
  session->first_event_id = first_event_id;
#ifdef ENABLE_UNIKORN_ATOMIC_RECORDING
  pthread_mutex_init(&session->mutex, NULL);
#endif
  session->thread_id_list_count = 0;
  session->thread_id_list = NULL;

#ifdef PRINT_INIT_INFO
  printf("%s():\n", __FUNCTION__);
  printf("  max_event_count = %d\n", session->max_event_count);
  printf("  flush_when_full = %s\n", session->flush_when_full ? "yes" : "no");
  printf("  is_multi_threaded = %s\n", session->is_multi_threaded ? "yes" : "no");
  printf("  record_instance = %s\n", session->record_instance ? "yes" : "no");
  printf("  record_value = %s\n", session->record_value ? "yes" : "no");
  printf("  record_file_location = %s\n", session->record_file_location ? "yes" : "no");
  printf("  first_event_id = %d\n", session->first_event_id);
#endif

  // Named Folders
#ifdef PRINT_INIT_INFO
  printf("  folder_registration_count = %d\n", session->folder_registration_count);
#endif
  if (session->folder_registration_count > 0) {
    session->folder_registration_list = calloc(session->folder_registration_count, sizeof(PrivateFolderInfo));
    assert(session->folder_registration_list != NULL);
    // Register the CLOSE_FOLDER_ID event
    session->folder_registration_list[0].id = CLOSE_FOLDER_ID;
    session->folder_registration_list[0].name = strdup("Close Folder");
#ifdef PRINT_INIT_INFO
    printf("    ID=%d, name='%s'\n", session->folder_registration_list[0].id, session->folder_registration_list[0].name);
#endif
    assert(session->folder_registration_list[0].name != NULL);
    // Register custom folders
    for (uint16_t i=1; i<session->folder_registration_count; i++) {
      session->folder_registration_list[i].id = attrs->folder_registration_list[i-1].id;
      session->folder_registration_list[i].name = strdup(attrs->folder_registration_list[i-1].name);
#ifdef PRINT_INIT_INFO
      printf("    ID=%d, name='%s'\n", session->folder_registration_list[i].id, session->folder_registration_list[i].name);
#endif
      assert(session->folder_registration_list[i].name != NULL);
    }
    // Create the stacks to track what folders are open
    uint16_t max_folder_stack_count = session->folder_registration_count - 1; // -1 due to the close folder event
    session->curr_folder_stack_count = 0;
    session->curr_folder_stack = calloc(max_folder_stack_count, sizeof(uint16_t));
    session->starting_folder_stack_count = 0;
    session->starting_folder_stack = calloc(max_folder_stack_count, sizeof(uint16_t));
  }

  // Named events
#ifdef PRINT_INIT_INFO
  printf("  event_registration_count = %d\n", session->event_registration_count);
#endif
  session->event_registration_list = calloc(session->event_registration_count, sizeof(PrivateEventInfo));
  assert(session->event_registration_list != NULL);
  // Register custom events
  for (uint16_t i=0; i<session->event_registration_count; i++) {
    session->event_registration_list[i].start_id = attrs->event_registration_list[i].start_id;
    session->event_registration_list[i].end_id = attrs->event_registration_list[i].end_id;
    session->event_registration_list[i].rgb = attrs->event_registration_list[i].rgb;
    session->event_registration_list[i].name = strdup(attrs->event_registration_list[i].name);
    assert(session->event_registration_list[i].name != NULL);
    session->event_registration_list[i].start_value_name = strdup(attrs->event_registration_list[i].start_value_name);
    assert(session->event_registration_list[i].start_value_name != NULL);
    session->event_registration_list[i].end_value_name = strdup(attrs->event_registration_list[i].end_value_name);
    assert(session->event_registration_list[i].end_value_name != NULL);
#ifdef PRINT_INIT_INFO
    printf("    startID=%d, endID=%d, RGB=0x%04x, name='%s', start_value_name='%s', end_value_name='%s'\n", session->event_registration_list[i].start_id, session->event_registration_list[i].end_id, session->event_registration_list[i].rgb,
           session->event_registration_list[i].name, session->event_registration_list[i].start_value_name, session->event_registration_list[i].end_value_name);
#endif
    session->event_registration_list[i].start_instance = 1;
    session->event_registration_list[i].end_instance = 1;
  }

  // Prepare the storage buffer
  session->num_stored_events = 0;
  session->curr_event_index = 0;
  session->first_unsaved_event_index = 0;
  session->events_buffer = malloc(session->max_event_count * sizeof(Event));
  assert(session->events_buffer != NULL);

  return session;
}

static void flushEvents(UnikornSession *session) {
  if (session->num_stored_events == 0) return; // Nothing to flush

  // Make sure the application defined file, socket, etc. is ready for the data
  bool ok = session->prepareFlush(session->flush_user_data);
  assert(ok);

  // Endian
  bool is_big_endian = isBigEndian();
#ifdef PRINT_FLUSH_INFO
  printf("%s():\n", __FUNCTION__);
  printf("  is_big_endian = %s\n", is_big_endian ? "yes" : "no");
#endif
  assert(session->flush(session->flush_user_data, &is_big_endian, sizeof(is_big_endian)));

  // Version
  uint16_t major = UK_API_VERSION_MAJOR;
  uint16_t minor = UK_API_VERSION_MINOR;
#ifdef PRINT_FLUSH_INFO
  printf("  version: %d.%d\n", major, minor);
#endif
  assert(session->flush(session->flush_user_data, &major, sizeof(major)));
  assert(session->flush(session->flush_user_data, &minor, sizeof(minor)));

  // Miscellanyous info
#ifdef PRINT_FLUSH_INFO
  printf("  is_multi_threaded = %s\n", session->is_multi_threaded ? "yes" : "no");
  printf("  record_instance = %s\n", session->record_instance ? "yes" : "no");
  printf("  record_value = %s\n", session->record_value ? "yes" : "no");
  printf("  record_file_location = %s\n", session->record_file_location ? "yes" : "no");
#endif
  assert(session->flush(session->flush_user_data, &session->is_multi_threaded, sizeof(session->is_multi_threaded)));
  assert(session->flush(session->flush_user_data, &session->record_instance, sizeof(session->record_instance)));
  assert(session->flush(session->flush_user_data, &session->record_value, sizeof(session->record_value)));
  assert(session->flush(session->flush_user_data, &session->record_file_location, sizeof(session->record_file_location)));

  // Folder info
#ifdef PRINT_FLUSH_INFO
  printf("  folder_registration_count = %d\n", session->folder_registration_count);
#endif
  assert(session->flush(session->flush_user_data, &session->folder_registration_count, sizeof(session->folder_registration_count)));
  for (uint16_t i=0; i<session->folder_registration_count; i++) {
    PrivateFolderInfo *folder = &session->folder_registration_list[i];
#ifdef PRINT_FLUSH_INFO
    printf("    ID=%d, name='%s'\n", folder->id, folder->name);
#endif
    assert(session->flush(session->flush_user_data, &folder->id, sizeof(folder->id)));
    uint16_t num_chars = 1 + (uint16_t)strlen(folder->name);
    assert(session->flush(session->flush_user_data, &num_chars, sizeof(num_chars)));
    assert(session->flush(session->flush_user_data, folder->name, num_chars));
  }

  // Event info
#ifdef PRINT_FLUSH_INFO
  printf("  event_registration_count = %d\n", session->event_registration_count);
#endif
  assert(session->flush(session->flush_user_data, &session->event_registration_count, sizeof(session->event_registration_count)));
  for (uint16_t i=0; i<session->event_registration_count; i++) {
    PrivateEventInfo *event = &session->event_registration_list[i];
#ifdef PRINT_FLUSH_INFO
    printf("    startID=%d, endID=%d, RGB=0x%04x, name='%s', start_value_name='%s', end_value_name='%s'\n", event->start_id, event->end_id, event->rgb, event->name, event->start_value_name, event->end_value_name);
#endif
    assert(session->flush(session->flush_user_data, &event->start_id, sizeof(event->start_id)));
    assert(session->flush(session->flush_user_data, &event->end_id, sizeof(event->end_id)));
    assert(session->flush(session->flush_user_data, &event->rgb, sizeof(event->rgb)));
    uint16_t num_chars = 1 + (uint16_t)strlen(event->name);
    assert(session->flush(session->flush_user_data, &num_chars, sizeof(num_chars)));
    assert(session->flush(session->flush_user_data, event->name, num_chars));
    num_chars = 1 + (uint16_t)strlen(event->start_value_name);
    assert(session->flush(session->flush_user_data, &num_chars, sizeof(num_chars)));
    assert(session->flush(session->flush_user_data, event->start_value_name, num_chars));
    num_chars = 1 + (uint16_t)strlen(event->end_value_name);
    assert(session->flush(session->flush_user_data, &num_chars, sizeof(num_chars)));
    assert(session->flush(session->flush_user_data, event->end_value_name, num_chars));
  }

  // File names and function names
  uint16_t file_name_count = 0;
  uint16_t function_name_count = 0;
  char **file_name_list = NULL;
  char **function_name_list = NULL;
  if (session->record_file_location) {
    // File names
    file_name_list = getFileNameList(session, &file_name_count);
#ifdef PRINT_FLUSH_INFO
    printf("  file_name_count = %d\n", file_name_count);
#endif
    assert(session->flush(session->flush_user_data, &file_name_count, sizeof(file_name_count)));
    for (uint16_t i=0; i<file_name_count; i++) {
      char *name = file_name_list[i];
#ifdef PRINT_FLUSH_INFO
      printf("    '%s'\n", name);
#endif
      uint16_t num_chars = 1 + (uint16_t)strlen(name);
      assert(session->flush(session->flush_user_data, &num_chars, sizeof(num_chars)));
      assert(session->flush(session->flush_user_data, name, num_chars));
    }
    // Functions names
    function_name_list = getFunctionNameList(session, &function_name_count);
#ifdef PRINT_FLUSH_INFO
    printf("  function_name_count = %d\n", function_name_count);
#endif
    assert(session->flush(session->flush_user_data, &function_name_count, sizeof(function_name_count)));
    for (uint16_t i=0; i<function_name_count; i++) {
      char *name = function_name_list[i];
#ifdef PRINT_FLUSH_INFO
      printf("    '%s'\n", name);
#endif
      uint16_t num_chars = 1 + (uint16_t)strlen(name);
      assert(session->flush(session->flush_user_data, &num_chars, sizeof(num_chars)));
      assert(session->flush(session->flush_user_data, name, num_chars));
    }
  }

  // Build thread ID list (this needs maintain old thread IDs between flushes since the flush pushes out an index into the list, which needs to be consistent over time
  if (session->is_multi_threaded) {
    getThreadIdList(session);
#ifdef PRINT_FLUSH_INFO
    printf("  thread_id_list_count = %d\n", session->thread_id_list_count);
#endif
    assert(session->flush(session->flush_user_data, &session->thread_id_list_count, sizeof(session->thread_id_list_count)));
    for (uint16_t i=0; i<session->thread_id_list_count; i++) {
      uint64_t thread_id = session->thread_id_list[i];
#ifdef PRINT_FLUSH_INFO
      printf("    %" UINT64_FORMAT "\n", thread_id);
#endif
      assert(session->flush(session->flush_user_data, &thread_id, sizeof(thread_id)));
    }
  }

  // Save list of folders that were open just prior to the first event in the buffer being saved
#ifdef PRINT_FLUSH_INFO
  printf("  Open folders at start of recording = %d\n", session->starting_folder_stack_count);
#endif
  assert(session->flush(session->flush_user_data, &session->starting_folder_stack_count, sizeof(session->starting_folder_stack_count)));
  for (uint16_t i=0; i<session->starting_folder_stack_count; i++) {
#ifdef PRINT_FLUSH_INFO
    printf("    '%s'\n", session->folder_registration_list[session->starting_folder_stack[i]].name);
#endif
    assert(session->flush(session->flush_user_data, &session->starting_folder_stack[i], sizeof(session->starting_folder_stack[i])));
  }
  // Now that there are no events in the buffer, need to reset the starting folder stack to be the same as the current folder stack
  session->starting_folder_stack_count = session->curr_folder_stack_count;
  for (uint16_t i=0; i<session->starting_folder_stack_count; i++) {
    session->starting_folder_stack[i] = session->curr_folder_stack[i];
  }

  // Events
#ifdef PRINT_FLUSH_INFO
  printf("  num_stored_events = %d (max count = %d)\n", session->num_stored_events, session->max_event_count);
#endif
  assert(session->flush(session->flush_user_data, &session->num_stored_events, sizeof(session->num_stored_events)));
  uint32_t index = session->first_unsaved_event_index;
  for (uint32_t i=0; i<session->num_stored_events; i++) {
    Event *event = &session->events_buffer[index];
    // Time
    assert(session->flush(session->flush_user_data, &event->time, sizeof(event->time)));
    // Event ID
    assert(session->flush(session->flush_user_data, &event->event_id, sizeof(event->event_id)));
#ifdef PRINT_FLUSH_INFO
    printf("    time=%"UINT64_FORMAT", event_id=%d\n", time, event_id);
#endif
    // Instance
    if (session->record_instance) {
      assert(session->flush(session->flush_user_data, &event->instance, sizeof(event->instance)));
#ifdef PRINT_FLUSH_INFO
      printf("    event_instance=%"UINT64_FORMAT"\n", event_instance);
#endif
    }
    // Value
    if (session->record_value) {
      assert(session->flush(session->flush_user_data, &event->value, sizeof(event->value)));
#ifdef PRINT_FLUSH_INFO
      printf("    value=%f\n", value);
#endif
    }
    // Thread ID
    if (session->is_multi_threaded) {
      uint16_t thread_index = getThreadIndex(event->thread_id, session->thread_id_list, session->thread_id_list_count);
      assert(session->flush(session->flush_user_data, &thread_index, sizeof(thread_index)));
#ifdef PRINT_FLUSH_INFO
      printf("    thread_index=%d\n", thread_index);
#endif
    }
    // Location
    if (session->record_file_location) {
      // File name
      uint16_t file_name_index = getNameIndex(event->file_name, file_name_list, file_name_count);
      assert(session->flush(session->flush_user_data, &file_name_index, sizeof(file_name_index)));
      // Function name
      uint16_t function_name_index = getNameIndex(event->function_name, function_name_list, function_name_count);
      assert(session->flush(session->flush_user_data, &function_name_index, sizeof(function_name_index)));
      // Line number
      assert(session->flush(session->flush_user_data, &event->line_number, sizeof(event->line_number)));
#ifdef PRINT_FLUSH_INFO
      printf("    file='%s', function='%s', line=%d\n", file_name, function_name, line_number);
#endif
    }

    index = (index + 1) % session->max_event_count;
  }

  // Reset accounting of the event buffer
  session->num_stored_events = 0;
  session->curr_event_index = 0;
  session->first_unsaved_event_index = 0;

  // Cleanup
  ok = session->finishFlush(session->flush_user_data);
  assert(ok);
  if (file_name_count > 0) free(file_name_list);
  if (function_name_count > 0) free(function_name_list);
}

void ukFlush(void *session_ref) {
  UnikornSession *session = (UnikornSession *)session_ref;
  assert(session->magic_value1 == MAGIC_VALUE1);
  assert(session->magic_value2 == MAGIC_VALUE2);
#ifdef ENABLE_UNIKORN_ATOMIC_RECORDING
  if (session->is_multi_threaded) pthread_mutex_lock(&session->mutex);
#endif
  flushEvents(session);
#ifdef ENABLE_UNIKORN_ATOMIC_RECORDING
  if (session->is_multi_threaded) pthread_mutex_unlock(&session->mutex);
#endif
}

void ukDestroy(void *session_ref) {
  // NOTE: this should be called after all other threads usiing this session are done
  UnikornSession *session = (UnikornSession *)session_ref;
  assert(session->magic_value1 == MAGIC_VALUE1);
  assert(session->magic_value2 == MAGIC_VALUE2);
  if (session->folder_registration_count > 0) {
    for (uint16_t i=0; i<session->folder_registration_count; i++) {
      free(session->folder_registration_list[i].name);
    }
    free(session->folder_registration_list);
    free(session->curr_folder_stack);
    free(session->starting_folder_stack);
  }
  if (session->event_registration_count > 0) {
    for (uint16_t i=0; i<session->event_registration_count; i++) {
      free(session->event_registration_list[i].name);
      free(session->event_registration_list[i].start_value_name);
      free(session->event_registration_list[i].end_value_name);
    }
    free(session->event_registration_list);
  }
  if (session->thread_id_list_count > 0) free(session->thread_id_list);
  free(session->events_buffer);
#ifdef ENABLE_UNIKORN_ATOMIC_RECORDING
  pthread_mutex_destroy(&session->mutex);
#endif
  free(session);
}

#ifdef TEST_RECORDING_OVERHEAD
#include <time.h>
static uint64_t getTime() {
  // clock_gettime() has nanosecond precision
  struct timespec curr_time;
  clock_gettime(CLOCK_MONOTONIC, &curr_time); // CLOCK_MONOTONIC is only increasing
  uint64_t total_nanoseconds = ((uint64_t)curr_time.tv_sec * 1000000000) + (uint64_t)curr_time.tv_nsec;
  return total_nanoseconds;
}
#endif

static void recordEvent(UnikornSession *session, uint16_t event_id, double value, uint64_t instance, const char *file, const char *function, uint16_t line_number) {
#ifdef TEST_RECORDING_OVERHEAD
  uint64_t t1 = getTime();
  t1 = getTime();
#endif

  // Store the required values
  Event *event = &session->events_buffer[session->curr_event_index];
  uint16_t replaced_event_id = event->event_id; // Need for book keeping at the end of this function
  event->time = session->clockNanoseconds();
  event->event_id = event_id;

  // Store the optional values
  // IMPORTANT: Even if the following will not be flushed, statistically the time overhead to record this extra info was indistinguisable from commenting it out and re-timing.
  //            The most costly part is myThreadId(), which multiplies the overhead by about 10x:
  //            Testing: Intel® Core™ i7-7700K CPU @ 4.20GHz × 8 using clock_gettime(CLOCK_MONOTONIC, &curr_time)
  //                     No thread ID recorded:  ~250 ns
  //                        Thread ID recorded: ~2000 ns
  event->instance = instance;
  event->value = value;
#ifdef ENABLE_UNIKORN_ATOMIC_RECORDING
  if (session->is_multi_threaded) {
    event->thread_id = myThreadId();
  }
#endif
  event->file_name = (char *)file;
  event->function_name = (char *)function;
  event->line_number = line_number;

  // Set the index of the next future event
  session->curr_event_index = (session->curr_event_index + 1) % session->max_event_count;
#ifdef TEST_RECORDING_OVERHEAD
  uint64_t t2 = getTime();
#endif

  // See if time to flush or wrap
  bool buffer_is_full = session->num_stored_events == session->max_event_count;
  if (buffer_is_full) { // This can only happen if auto flush is off
    // Buffer was already full, must not have auto save enabled
    session->first_unsaved_event_index = (session->first_unsaved_event_index + 1) % session->max_event_count;
    // If the event is a folder, need to remember it was opened/closed
    if (replaced_event_id < session->folder_registration_count) {
      // This is a folder event
      if (replaced_event_id == CLOSE_FOLDER_ID) {
        popStartingFolderStack(session);
      } else {
        pushStartingFolderStack(session, replaced_event_id);
      }
    }
  } else {
    // Buffer not yet full
    session->num_stored_events++;
    if (session->flush_when_full && session->num_stored_events == session->max_event_count) {
      flushEvents(session);
    }
  }

#ifdef TEST_RECORDING_OVERHEAD
  uint64_t t3 = getTime();
  printf("record time: %zd ns, book keeping time: %zd ns\n", t2-t1, t3-t2);
#endif
}

void ukRecordEvent(void *session_ref, uint16_t event_id, double value, const char *file, const char *function, uint16_t line_number) {
  UnikornSession *session = (UnikornSession *)session_ref;
  OPTIONAL_ASSERT(session->magic_value1 == MAGIC_VALUE1);
  OPTIONAL_ASSERT(session->magic_value2 == MAGIC_VALUE2);
  uint16_t event_registration_index = (event_id - session->first_event_id) / 2;
  OPTIONAL_ASSERT(event_registration_index < session->event_registration_count*2);
  PrivateEventInfo *event = &session->event_registration_list[event_registration_index];
#ifdef PRINT_RECORD_INFO
  printf("%s(): ID=%d, value=%f, file=%s, function=%s, line_number=%d\n", __FUNCTION__, event_id, value, file, function, line_number);
#endif

#ifdef ENABLE_UNIKORN_ATOMIC_RECORDING
  if (session->is_multi_threaded) pthread_mutex_lock(&session->mutex);
#endif

  // Add the event to the event buffer
  uint64_t instance = (event->start_id == event_id) ? event->start_instance++ : event->end_instance++;
  recordEvent(session, event_id, value, instance, file, function, line_number);

#ifdef ENABLE_UNIKORN_ATOMIC_RECORDING
  if (session->is_multi_threaded) pthread_mutex_unlock(&session->mutex);
#endif
}

void ukOpenFolder(void *session_ref, uint16_t folder_id) {
  UnikornSession *session = (UnikornSession *)session_ref;
  OPTIONAL_ASSERT(session->magic_value1 == MAGIC_VALUE1);
  OPTIONAL_ASSERT(session->magic_value2 == MAGIC_VALUE2);
  OPTIONAL_ASSERT(session->folder_registration_count > 0);
  OPTIONAL_ASSERT(folder_id >= 1 && folder_id < session->folder_registration_count);
#ifdef PRINT_RECORD_INFO
  printf("%s(): ID=%d\n", __FUNCTION__, folder_id);
#endif

#ifdef ENABLE_UNIKORN_ATOMIC_RECORDING
  if (session->is_multi_threaded) pthread_mutex_lock(&session->mutex);
#endif

  // Push folder onto current folder stack
  OPTIONAL_ASSERT(session->curr_folder_stack_count < (session->folder_registration_count - 1)); // -1 for the close folder event
  for (uint16_t i=0; i<session->curr_folder_stack_count; i++) {
    OPTIONAL_ASSERT(session->curr_folder_stack[i] != folder_id);
  }
  session->curr_folder_stack[session->curr_folder_stack_count] = folder_id;
  session->curr_folder_stack_count++;

  // Add the folder event to the event buffer
  recordEvent(session, folder_id, 0, 0, L_unused_name, L_unused_name, 0);

#ifdef ENABLE_UNIKORN_ATOMIC_RECORDING
  if (session->is_multi_threaded) pthread_mutex_unlock(&session->mutex);
#endif
}

void ukCloseFolder(void *session_ref) {
  UnikornSession *session = (UnikornSession *)session_ref;
  OPTIONAL_ASSERT(session->magic_value1 == MAGIC_VALUE1);
  OPTIONAL_ASSERT(session->magic_value2 == MAGIC_VALUE2);
  OPTIONAL_ASSERT(session->folder_registration_count > 0);
#ifdef PRINT_RECORD_INFO
  printf("%s()\n", __FUNCTION__);
#endif

#ifdef ENABLE_UNIKORN_ATOMIC_RECORDING
  if (session->is_multi_threaded) pthread_mutex_lock(&session->mutex);
#endif

  // Pop the latest folder from the current folder stack
  OPTIONAL_ASSERT(session->curr_folder_stack_count > 0);
  session->curr_folder_stack_count--;

  // Add the folder event to the event buffer
  recordEvent(session, CLOSE_FOLDER_ID, 0, 0, L_unused_name, L_unused_name, 0);

#ifdef ENABLE_UNIKORN_ATOMIC_RECORDING
  if (session->is_multi_threaded) pthread_mutex_unlock(&session->mutex);
#endif
}
