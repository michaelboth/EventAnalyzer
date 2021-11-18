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
    assert(object->includes_file_location = includes_file_location);
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

static void loadEventsData(FILE *file, bool first_time_loaded, bool swap_endian, Events *object) {
#ifdef PRINT_LOAD_INFO
  printf("\n");
  printf("Event File Data: ---------------------------------\n");
#endif
  if (object->includes_file_location) {
    // File names
    //*+*/first_time_loaded
    object->file_name_count = readUint16(swap_endian, file);
#ifdef PRINT_LOAD_INFO
    printf("  file_name_count = %d\n", object->file_name_count);
#endif
    object->file_name_list = malloc(object->file_name_count*sizeof(char *));
    assert(object->file_name_list != NULL);
    for (uint16_t i=0; i<object->file_name_count; i++) {
      uint16_t num_name_chars = readUint16(swap_endian, file);
      object->file_name_list[i] = malloc(num_name_chars);
      assert(object->file_name_list[i] != NULL);
      readChars(object->file_name_list[i], num_name_chars, file);
#ifdef PRINT_LOAD_INFO
      printf("    name='%s'\n", object->file_name_list[i]);
#endif
    }

    // Function names
    object->function_name_count = readUint16(swap_endian, file);
#ifdef PRINT_LOAD_INFO
    printf("  function_name_count = %d\n", object->function_name_count);
#endif
    object->function_name_list = malloc(object->function_name_count*sizeof(char *));
    assert(object->function_name_list != NULL);
    for (uint16_t i=0; i<object->function_name_count; i++) {
      uint16_t num_name_chars = readUint16(swap_endian, file);
      object->function_name_list[i] = malloc(num_name_chars);
      assert(object->function_name_list[i] != NULL);
      readChars(object->function_name_list[i], num_name_chars, file);
#ifdef PRINT_LOAD_INFO
      printf("    name='%s'\n", object->function_name_list[i]);
#endif
    }
  }

  // Thread IDs
  if (object->is_threaded) {
    object->thread_id_count = readUint16(swap_endian, file);
#ifdef PRINT_LOAD_INFO
    printf("  thread_id_count = %d\n", object->thread_id_count);
#endif
    object->thread_id_list = malloc(object->thread_id_count*sizeof(uint64_t));
    assert(object->thread_id_list != NULL);
    for (uint16_t i=0; i<object->thread_id_count; i++) {
      object->thread_id_list[i] = readUint64(swap_endian, file);
#ifdef PRINT_LOAD_INFO
      printf("    index %d: ID=%"UINT64_FORMAT"\n", i, object->thread_id_list[i]);
#endif
    }
  }

  // Open Folders
  object->num_open_folders = readUint16(swap_endian, file);
#ifdef PRINT_LOAD_INFO
  printf("  num_open_folders = %d\n", object->num_open_folders);
#endif
  object->folder_id_list = malloc(object->num_open_folders*sizeof(uint16_t));
  assert(object->folder_id_list != NULL);
  for (uint16_t i=0; i<object->num_open_folders; i++) {
    object->folder_id_list[i] = readUint16(swap_endian, file);
#ifdef PRINT_LOAD_INFO
    printf("    index = %d: '%s'\n", object->folder_id_list[i], object->folder_info_list[object->folder_id_list[i]].name);
#endif
  }

  // Allocate events buffer
  object->event_count = readUint32(swap_endian, file);
#ifdef PRINT_LOAD_INFO
  printf("  event_count = %d\n", object->event_count);
#endif
  object->event_buffer = malloc((object->num_open_folders+object->event_count)*sizeof(Event));
  assert(object->event_buffer != NULL);

  // Create folder events for open folders
  uint32_t event_index = 0;
  for (uint16_t i=0; i<object->num_open_folders; i++) {
    Event *event = &object->event_buffer[event_index];
    event->time = 0; // IMPORTANT: need to set this value to the first loaded event time... do this after loading events
    event->event_id = object->folder_id_list[i];
#ifdef PRINT_LOAD_INFO
    printf("  Adding open folder event: ID = %d\n", event->event_id);
#endif
    event_index++;
  }

  // Load events
  uint64_t time_adjustment = 0;
  for (uint32_t i=0; i<object->event_count; i++) {
    Event *event = &object->event_buffer[event_index];
    event->time = readUint64(swap_endian, file) + time_adjustment;
    // Verify time is increasing
    if (i>0) {
      Event *prev_event = &object->event_buffer[event_index-1];
      if (event->time < prev_event->time) {
	printf("The event file contains an event that go backwards in time. Following event times will be adjusted to be forward in time. To avoid this, use a monotonically increasing clock when recording.\n");
	time_adjustment += (prev_event->time - event->time);
      }
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

  // Set the event times of the inserted open folders
  Event *first_loaded_event = &object->event_buffer[object->num_open_folders];
  for (uint16_t i=0; i<object->num_open_folders; i++) {
    Event *event = &object->event_buffer[i];
    event->time = first_loaded_event->time;
  }
  object->event_count += object->num_open_folders;
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
    loadEventsData(file, first_time_loaded, swap_endian, object);
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
  free(object->folder_id_list);
  free(object->event_buffer);
  free(object);
}
