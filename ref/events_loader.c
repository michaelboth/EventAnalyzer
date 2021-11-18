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

#include "events_loader.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#ifdef __APPLE__
  // Mac OS X / Darwin features
  #include <libkern/OSByteOrder.h>
  #define bswap_16(x) OSSwapInt16(x)
  #define bswap_32(x) OSSwapInt32(x)
  #define bswap_64(x) OSSwapInt64(x)
  #define UINT64_FORMAT "llu"
#elif _WIN32
  #define bswap_16(x) _byteswap_ushort(x)
  #define bswap_32(x) _byteswap_ulong(x)
  #define bswap_64(x) _byteswap_uint64(x)
  #define UINT64_FORMAT "zu"
#else
  #include <byteswap.h>
  #define UINT64_FORMAT "zu"
#endif

//#define PRINT_LOAD_INFO

static bool isBigEndian() {
  uint32_t a = 1;
  unsigned char *b = (unsigned char *)&a;
  return (b[3] == 1);
}

static bool readBool(FILE *file) {
  bool value;
  if (1 != fread(&value, sizeof(value), 1, file)) {
    printf("End of event file reached before all expected data processed\n");
    exit(0);
  }
  return value;
}

static uint16_t readUint16(bool swap_endian, FILE *file) {
  uint16_t value;
  if (1 != fread(&value, sizeof(value), 1, file)) {
    printf("End of event file reached before all expected data processed\n");
    exit(0);
  }
  if (swap_endian) value = bswap_16(value);
  return value;
}

static uint32_t readUint32(bool swap_endian, FILE *file) {
  uint32_t value;
  if (1 != fread(&value, sizeof(value), 1, file)) {
    printf("End of event file reached before all expected data processed\n");
    exit(0);
  }
  if (swap_endian) value = bswap_32(value);
  return value;
}

static uint64_t readUint64(bool swap_endian, FILE *file) {
  uint64_t value;
  if (1 != fread(&value, sizeof(value), 1, file)) {
    printf("End of event file reached before all expected data processed\n");
    exit(0);
  }
  if (swap_endian) value = bswap_64(value);
  return value;
}

static double readDouble(bool swap_endian, FILE *file) {
  double value;
  if (1 != fread(&value, sizeof(value), 1, file)) {
    printf("End of event file reached before all expected data processed\n");
    exit(0);
  }
  if (swap_endian) {
    uint64_t *uint64_ptr = (uint64_t *)&value;
    *uint64_ptr = bswap_64(*uint64_ptr);
  }
  return value;
}

static void readChars(char *name, int num_name_chars, FILE *file) {
  if (1 != fread(name, num_name_chars, 1, file)) {
    printf("End of event file reached before all expected data processed\n");
    exit(0);
  }
}

static void loadEventsHeader(FILE *file, bool first_time_loaded, bool swap_endian, Events *object) {
  // Get and verify version
  uint16_t version_major = readUint16(swap_endian, file);
  uint16_t version_minor = readUint16(swap_endian, file);
  // Currently only supporting version 1.0
  assert(version_major == 1);
  assert(version_minor == 0);
  if (first_time_loaded) {
    object->version_major = version_major;
    object->version_minor = version_minor;
  } else {
    assert(object->version_major == version_major);
    assert(object->version_minor == version_minor);
  }

  // Get the event parameters
  bool is_threaded = readBool(file);
  if (first_time_loaded) {
    object->is_threaded = is_threaded;
  } else {
    assert(object->is_threaded == is_threaded);
  }
  bool includes_instance = readBool(file);
  if (first_time_loaded) {
    object->includes_instance = includes_instance;
  } else {
    assert(object->includes_instance == includes_instance);
  }
  bool includes_value = readBool(file);
  if (first_time_loaded) {
    object->includes_value = includes_value;
  } else {
    assert(object->includes_value == includes_value);
  }
  bool includes_file_location = readBool(file);
  if (first_time_loaded) {
    object->includes_file_location = includes_file_location;
  } else {
    assert(object->includes_file_location == includes_file_location);
  }
#ifdef PRINT_LOAD_INFO
  printf("\n");
  printf("Event File Header: -------------------------------\n");
  if (!first_time_loaded) printf("  VALIDATING SUBSEQUENT HEADER MATCHES FIRST HEADER\n");
  bool is_big_endian = swap_endian ? !isBigEndian() : isBigEndian();
  printf("  file is big endian = %s (need endian swapping = %s)\n", is_big_endian ? "yes" : "no", swap_endian ? "yes" : "no");
  printf("  version = %d.%d\n", version_major, version_minor);
  printf("  is_threaded = %s\n", object->is_threaded ? "yes" : "no");
  printf("  includes_instance = %s\n", object->includes_instance ? "yes" : "no");
  printf("  includes_value = %s\n", object->includes_value ? "yes" : "no");
  printf("  includes_file_location = %s\n", object->includes_file_location ? "yes" : "no");
#endif

  // Folder info
  uint16_t folder_info_count = readUint16(swap_endian, file);
  if (first_time_loaded) {
    object->folder_info_count = folder_info_count;
  } else {
    assert(object->folder_info_count == folder_info_count);
  }
#ifdef PRINT_LOAD_INFO
  printf("  folder_info_count = %d\n", object->folder_info_count);
#endif
  if (first_time_loaded) {
    object->folder_info_list = malloc(object->folder_info_count*sizeof(FolderInfo));
    assert(object->folder_info_list != NULL);
  }
  for (uint16_t i=0; i<object->folder_info_count; i++) {
    FolderInfo *folder = &object->folder_info_list[i];
    uint16_t id = readUint16(swap_endian, file);
    if (first_time_loaded) {
      folder->id = id;
    } else {
      assert(folder->id == id);
    }
    uint16_t num_name_chars = readUint16(swap_endian, file);
    if (first_time_loaded) {
      folder->name = malloc(num_name_chars);
      assert(folder->name != NULL);
      readChars(folder->name, num_name_chars, file);
    } else {
      assert((uint16_t)strlen(folder->name)+1 == num_name_chars);
      char ch;
      for (uint16_t j=0; j<num_name_chars; j++) {
	readChars(&ch, 1, file);
	assert(folder->name[j] == ch);
      }
    }
#ifdef PRINT_LOAD_INFO
    printf("    ID=%d, name='%s'\n", folder->id, folder->name);
#endif
  }

  // Event info
  uint16_t event_info_count = readUint16(swap_endian, file);
  if (first_time_loaded) {
    object->event_info_count = event_info_count;
  } else {
    assert(object->event_info_count == event_info_count);
  }
#ifdef PRINT_LOAD_INFO
  printf("  event_info_count = %d\n", object->event_info_count);
#endif
  if (first_time_loaded) {
    object->event_info_list = malloc(object->event_info_count*sizeof(EventInfo));
    assert(object->event_info_list != NULL);
  }
  for (uint16_t i=0; i<object->event_info_count; i++) {
    EventInfo *event = &object->event_info_list[i];
    uint16_t start_id = readUint16(swap_endian, file);
    if (first_time_loaded) {
      event->start_id = start_id;
    } else {
      assert(event->start_id == start_id);
    }
    uint16_t end_id = readUint16(swap_endian, file);
    if (first_time_loaded) {
      event->end_id = end_id;
    } else {
      assert(event->end_id == end_id);
    }
    uint16_t rgb = readUint16(swap_endian, file);
    if (first_time_loaded) {
      event->rgb = rgb;
    } else {
      assert(event->rgb == rgb);
    }
    uint16_t num_name_chars = readUint16(swap_endian, file);
    if (first_time_loaded) {
      event->name = malloc(num_name_chars);
      assert(event->name != NULL);
      readChars(event->name, num_name_chars, file);
    } else {
      assert((uint16_t)strlen(event->name)+1 == num_name_chars);
      char ch;
      for (uint16_t j=0; j<num_name_chars; j++) {
	readChars(&ch, 1, file);
	assert(event->name[j] == ch);
      }
    }
#ifdef PRINT_LOAD_INFO
    printf("    startID=%d, endID=%d, RGB=0x%04x, name='%s'\n", event->start_id, event->end_id, event->rgb, event->name);
#endif
  }
}

static void loadEventsData(FILE *file, bool swap_endian, Events *object) {
#ifdef PRINT_LOAD_INFO
  printf("\n");
  printf("Event File Data: ---------------------------------\n");
#endif
  if (object->includes_file_location) {
    // File names
    uint16_t file_name_count = readUint16(swap_endian, file);
#ifdef PRINT_LOAD_INFO
    printf("  file_name_count = %d\n", file_name_count);
#endif
    for (uint16_t i=0; i<file_name_count; i++) {
      uint16_t num_name_chars = readUint16(swap_endian, file);
      char *name = malloc(num_name_chars);
      assert(name != NULL);
      readChars(name, num_name_chars, file);
#ifdef PRINT_LOAD_INFO
      printf("    name='%s'\n", name);
#endif
      bool found = false;
      for (uint16_t j=0; j<object->file_name_count; j++) {
        if (strcmp(object->file_name_list[j], name) == 0) {
          found = true;
          break;
        }
      }
      if (!found) {
        object->file_name_count++;
        object->file_name_list = realloc(object->file_name_list, object->file_name_count*sizeof(char *));
        assert(object->file_name_list != NULL);
        object->file_name_list[object->file_name_count-1] = name;
      } else {
        free(name);
      }
    }

    // Function names
    uint16_t function_name_count = readUint16(swap_endian, file);
#ifdef PRINT_LOAD_INFO
    printf("  function_name_count = %d\n", function_name_count);
#endif
    for (uint16_t i=0; i<function_name_count; i++) {
      uint16_t num_name_chars = readUint16(swap_endian, file);
      char *name = malloc(num_name_chars);
      assert(name != NULL);
      readChars(name, num_name_chars, file);
#ifdef PRINT_LOAD_INFO
      printf("    name='%s'\n", name);
#endif
      bool found = false;
      for (uint16_t j=0; j<object->function_name_count; j++) {
        if (strcmp(object->function_name_list[j], name) == 0) {
          found = true;
          break;
        }
      }
      if (!found) {
        object->function_name_count++;
        object->function_name_list = realloc(object->function_name_list, object->function_name_count*sizeof(char *));
        assert(object->function_name_list != NULL);
        object->function_name_list[object->function_name_count-1] = name;
      } else {
        free(name);
      }
    }
  }

  // Thread IDs
  if (object->is_threaded) {
    uint16_t thread_id_count = readUint16(swap_endian, file);
#ifdef PRINT_LOAD_INFO
    printf("  thread_id_count = %d\n", thread_id_count);
#endif
    for (uint16_t i=0; i<thread_id_count; i++) {
      uint64_t thread_id = readUint64(swap_endian, file);
#ifdef PRINT_LOAD_INFO
      printf("    index %d: ID=%"UINT64_FORMAT"\n", i, thread_id);
#endif
      if (i < object->thread_id_count) {
        // Verify the thread ID has not changed since the last flush
        assert(object->thread_id_list[i] == thread_id);
      } else {
        // Add the thread ID to the list
        assert(i == object->thread_id_count);
        object->thread_id_count++;
        object->thread_id_list = realloc(object->thread_id_list, object->thread_id_count*sizeof(uint64_t));
        assert(object->thread_id_list != NULL);
        object->thread_id_list[i] = thread_id;
      }
    }
  }

  // Open Folders: Stack of folders that were already open before the first event that was saved
  /*+*/
  uint16_t num_open_folders = readUint16(swap_endian, file);
#ifdef PRINT_LOAD_INFO
  printf("  num_open_folders = %d\n", num_open_folders);
#endif
  uint16_t *folder_id_list = NULL;
  if (num_open_folders > 0) {
    folder_id_list = malloc(num_open_folders*sizeof(uint16_t));
    assert(folder_id_list != NULL);
    for (uint16_t i=0; i<num_open_folders; i++) {
      folder_id_list[i] = readUint16(swap_endian, file);
#ifdef PRINT_LOAD_INFO
      printf("    index = %d: '%s'\n", folder_id_list[i], object->folder_info_list[folder_id_list[i]].name);
#endif
    }
  }

  // From the existing events, determine the list of folders that were open after the last event
  uint16_t num_final_open_folders = 0;
  uint16_t *final_folder_id_list = NULL;
  if (object->folder_info_count > 0) {
    final_folder_id_list = malloc(object->folder_info_count*sizeof(uint16_t));
    assert(final_folder_id_list != NULL);
    uint16_t first_event_id = (object->folder_info_count == 0) ? 1 : object->folder_info_count;
    for (uint32_t i=0; i<object->event_count; i++) {
      Event *event = &object->event_buffer[i];
      if (event->event_id < first_event_id) {
        // This is a folder event
        if (event->event_id == 0) {
          // Close folder
          assert(num_final_open_folders > 0);
          num_final_open_folders--;
        } else {
          // Push folder on stack
          assert(num_final_open_folders < object->folder_info_count);
          final_folder_id_list[num_final_open_folders] = event->event_id;
          num_final_open_folders++;
        }
      }
    }
  }

  // Compare the final open folders with the expected open folders
  // NOTE: if the two sets of folders are the same, then no need to create folder events to compensate
  if (num_open_folders > 0 && num_open_folders == num_final_open_folders) {
    bool are_equal = true;
    for (uint16_t i=0; i<num_open_folders; i++) {
      if (folder_id_list[i] != final_folder_id_list[i]) {
        are_equal = false;
        break;
      }
    }
    if (are_equal) {
      // Can drop the folders since the existing open folder in the previous events are already what is expected
      num_open_folders = 0;
      num_final_open_folders = 0;
      free(folder_id_list);
      free(final_folder_id_list);
#ifdef PRINT_LOAD_INFO
      printf("  Existing folders and expected folders are the same, no need to insert extra folder events\n");
#endif
    }
  }

  // Allocate events buffer
  uint32_t event_count = readUint32(swap_endian, file);
#ifdef PRINT_LOAD_INFO
  printf("  event_count = %d\n", object->event_count);
#endif
  Event *prev_event = NULL; // Keep track of the latest event to do time comparisons later
  if (object->event_count > 0) {
    prev_event = &object->event_buffer[object->event_count-1];
  }
  uint32_t event_index = object->event_count;
  object->event_count += event_count;
  object->event_buffer = realloc(object->event_buffer, (num_final_open_folders+num_open_folders+object->event_count)*sizeof(Event));
  assert(object->event_buffer != NULL);

  // Create folder events for closing old folder and opening expected open folders
  uint32_t first_inserted_folder_event_index = event_index;
  // Close old folders
  for (uint16_t i=0; i<num_final_open_folders; i++) {
    Event *event = &object->event_buffer[event_index];
    event->time = 0; // IMPORTANT: need to set this value to the first loaded event time... do this after loading events
    event->event_id = 0; // Reserved ID for close folder
#ifdef PRINT_LOAD_INFO
    printf("  Adding close folder event\n");
#endif
    event_index++;
  }
  // Open new folders
  for (uint16_t i=0; i<num_open_folders; i++) {
    Event *event = &object->event_buffer[event_index];
    event->time = 0; // IMPORTANT: need to set this value to the first loaded event time... do this after loading events
    event->event_id = folder_id_list[i];
#ifdef PRINT_LOAD_INFO
    printf("  Adding open folder event: ID = %d\n", event->event_id);
#endif
    event_index++;
  }

  // Load events
  uint64_t time_adjustment = 0;
  Event *first_loaded_event = NULL;
  for (uint32_t i=0; i<event_count; i++) {
    Event *event = &object->event_buffer[event_index];
    if (first_loaded_event == NULL) {
      first_loaded_event = event;
    }
    event->time = readUint64(swap_endian, file) + time_adjustment;
    // Verify time is increasing
    if (prev_event != NULL) {
      if (event->time < prev_event->time) {
	printf("The event file contains an event that go backwards in time. Following event times will be adjusted to be forward in time. To avoid this, use a monotonically increasing clock when recording.\n");
	time_adjustment += (prev_event->time - event->time);
      }
      prev_event = event;
    }
    event->event_id = readUint16(swap_endian, file);
#ifdef PRINT_LOAD_INFO
    printf("    time = %"UINT64_FORMAT", ID = %d\n", event->time, event->event_id);
#endif
    if (object->includes_instance) {
      event->instance = readUint64(swap_endian, file);
#ifdef PRINT_LOAD_INFO
      printf("    instance = %"UINT64_FORMAT"\n", event->instance);
#endif
    }
    if (object->includes_value) {
      event->value = readDouble(swap_endian, file);
#ifdef PRINT_LOAD_INFO
      printf("    value = %f\n", event->value);
#endif
    }
    if (object->is_threaded) {
      event->thread_index = readUint16(swap_endian, file);
#ifdef PRINT_LOAD_INFO
      printf("    thread index = %d\n", event->thread_index);
#endif
    }
    if (object->includes_file_location) {
      event->file_name_index = readUint16(swap_endian, file);
      event->function_name_index = readUint16(swap_endian, file);
      event->line_number = readUint16(swap_endian, file);
#ifdef PRINT_LOAD_INFO
      printf("    file = '%s', function = '%s', line = %d\n", object->file_name_list[event->file_name_index], object->function_name_list[event->function_name_index], event->line_number);
#endif
    }
    event_index++;
  }

  // Set the event times of the inserted close and open folders
  for (uint16_t i=0; i<num_final_open_folders+num_open_folders; i++) {
    Event *event = &object->event_buffer[first_inserted_folder_event_index+i];
    event->time = first_loaded_event->time;
  }
  object->event_count += num_open_folders;

  // Clean up
  if (num_open_folders > 0) {
    free(folder_id_list);
  }
  if (num_final_open_folders > 0) {
    free(final_folder_id_list);
  }
}

Events *loadEventsFile(const char *filename) {
#ifdef _WIN32
  FILE *file;
  errno_t status = fopen_s(&file, filename, "rb");
  if (status != 0) return NULL;
#else
  FILE *file = fopen(filename, "rb");
  if (file == NULL) return NULL;
#endif

  Events *object = calloc(1, sizeof(Events));
  assert(object != NULL);

  // Get endian
  bool first_time_loaded = true;
  bool swap_endian = false;
  bool is_big_endian = false;
  while (true) {
    // Get the endian
    if (first_time_loaded) {
      // Must load something
      if (1 != fread(&is_big_endian, sizeof(is_big_endian), 1, file)) {
	printf("End of event file reached before all expected data processed\n");
	exit(0);
      }
      swap_endian = (is_big_endian != isBigEndian());
    } else {
      // Trying loading a subsequent flush, which is not required
      bool subsequent_is_big_endian;
      if (1 != fread(&subsequent_is_big_endian, sizeof(subsequent_is_big_endian), 1, file)) {
	// At end of file, so OK to break out of while loop
	break;
      }
      // Was able to load a byte indicating there was a subsequent flush, so load the event info
      assert(subsequent_is_big_endian == is_big_endian);
    }
    loadEventsHeader(file, first_time_loaded, swap_endian, object);
    loadEventsData(file, swap_endian, object);
    first_time_loaded = false;
  }

  int rc = fclose(file);
  assert(rc == 0);

  return object;
}

void freeEvents(Events *object) {
  for (uint16_t i=0; i<object->folder_info_count; i++) {
    free(object->folder_info_list[i].name);
  }
  free(object->folder_info_list);
  for (uint16_t i=0; i<object->event_info_count; i++) {
    free(object->event_info_list[i].name);
  }
  free(object->event_info_list);
  for (uint16_t i=0; i<object->file_name_count; i++) {
    free(object->file_name_list[i]);
  }
  free(object->file_name_list);
  for (uint16_t i=0; i<object->function_name_count; i++) {
    free(object->function_name_list[i]);
  }
  free(object->function_name_list);
  free(object->thread_id_list);
  free(object->event_buffer);
  free(object);
}
