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
#ifdef __APPLE__
  // Mac OS X / Darwin features
  #include <libkern/OSByteOrder.h>
  #define bswap_16(x) OSSwapInt16(x)
  #define bswap_32(x) OSSwapInt32(x)
  #define bswap_64(x) OSSwapInt64(x)
#else
  #include <byteswap.h>
#endif

//#define PRINT_LOAD_INFO

static bool isBigEndian() {
  uint32_t a = 1;
  unsigned char *b = (unsigned char *)&a;
  return (b[3] == 1);
}

static bool readBool(FILE *file) {
  bool value;
  assert(1 == fread(&value, sizeof(value), 1, file));
  return value;
}

static uint16_t readUint16(bool swap_endian, FILE *file) {
  uint16_t value;
  assert(1 == fread(&value, sizeof(value), 1, file));
  if (swap_endian) value = bswap_16(value);
  return value;
}

static uint32_t readUint32(bool swap_endian, FILE *file) {
  uint32_t value;
  assert(1 == fread(&value, sizeof(value), 1, file));
  if (swap_endian) value = bswap_32(value);
  return value;
}

static uint64_t readUint64(bool swap_endian, FILE *file) {
  uint64_t value;
  assert(1 == fread(&value, sizeof(value), 1, file));
  if (swap_endian) value = bswap_64(value);
  return value;
}

static double readDouble(bool swap_endian, FILE *file) {
  double value;
  assert(1 == fread(&value, sizeof(value), 1, file));
  if (swap_endian) {
    uint64_t *uint64_ptr = (uint64_t *)&value;
    *uint64_ptr = bswap_64(*uint64_ptr);
  }
  return value;
}

static void readChars(char *name, int num_name_chars, FILE *file) {
  assert(1 == fread(name, num_name_chars, 1, file));
}

Events *loadEventsFile(const char *filename) {
  FILE *file = fopen(filename, "rb");
  if (file == NULL) return NULL;

  Events *object = calloc(1, sizeof(Events));
  assert(object != NULL);

  // Get the event parameters
  bool is_big_endian = readBool(file);
  bool swap_endian = (is_big_endian != isBigEndian());
  object->is_threaded = readBool(file);
  object->includes_instance = readBool(file);
  object->includes_value = readBool(file);
  object->includes_file_location = readBool(file);
#ifdef PRINT_LOAD_INFO
  printf("%s():\n", __FUNCTION__);
  printf("  is_big_endian = %s (need endian swapping = %s)\n", is_big_endian ? "yes" : "no", swap_endian ? "yes" : "no");
  printf("  is_threaded = %s\n", object->is_threaded ? "yes" : "no");
  printf("  includes_instance = %s\n", object->includes_instance ? "yes" : "no");
  printf("  includes_value = %s\n", object->includes_value ? "yes" : "no");
  printf("  includes_file_location = %s\n", object->includes_file_location ? "yes" : "no");
#endif

  // Folder info
  object->folder_info_count = readUint16(swap_endian, file);
#ifdef PRINT_LOAD_INFO
  printf("  folder_info_count = %d\n", object->folder_info_count);
#endif
  object->folder_info_list = malloc(object->folder_info_count*sizeof(FolderInfo));
  assert(object->folder_info_list != NULL);
  for (uint16_t i=0; i<object->folder_info_count; i++) {
    FolderInfo *folder = &object->folder_info_list[i];
    folder->id = readUint16(swap_endian, file);
    uint16_t num_name_chars = readUint16(swap_endian, file);
    folder->name = malloc(num_name_chars);
    assert(folder->name != NULL);
    readChars(folder->name, num_name_chars, file);
#ifdef PRINT_LOAD_INFO
    printf("    ID=%d, name='%s'\n", folder->id, folder->name);
#endif
  }

  // Event info
  object->event_info_count = readUint16(swap_endian, file);
#ifdef PRINT_LOAD_INFO
  printf("  event_info_count = %d\n", object->event_info_count);
#endif
  object->event_info_list = malloc(object->event_info_count*sizeof(EventInfo));
  assert(object->event_info_list != NULL);
  for (uint16_t i=0; i<object->event_info_count; i++) {
    EventInfo *event = &object->event_info_list[i];
    event->start_id = readUint16(swap_endian, file);
    event->end_id = readUint16(swap_endian, file);
    event->rgb = readUint16(swap_endian, file);
    uint16_t num_name_chars = readUint16(swap_endian, file);
    event->name = malloc(num_name_chars);
    assert(event->name != NULL);
    readChars(event->name, num_name_chars, file);
#ifdef PRINT_LOAD_INFO
    printf("    startID=%d, endID=%d, RGB=0x%04x, name='%s'\n", event->start_id, event->end_id, event->rgb, event->name);
#endif
  }

  if (object->includes_file_location) {
    // File names
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
      printf("    index %d: ID=%llu\n", i, object->thread_id_list[i]);
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

  // Events
  object->event_count = readUint32(swap_endian, file);
#ifdef PRINT_LOAD_INFO
  printf("  event_count = %d\n", object->event_count);
#endif
  object->event_buffer = malloc(object->event_count*sizeof(Event));
  assert(object->event_buffer != NULL);
  for (uint16_t i=0; i<object->event_count; i++) {
    Event *event = &object->event_buffer[i];
    event->time = readUint64(swap_endian, file);
    event->event_id = readUint16(swap_endian, file);
#ifdef PRINT_LOAD_INFO
    printf("    time = %llu, ID = %d\n", event->time, event->event_id);
#endif
    if (object->includes_instance) {
      event->instance = readUint64(swap_endian, file);
#ifdef PRINT_LOAD_INFO
      printf("    instance = %llu\n", event->instance);
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
