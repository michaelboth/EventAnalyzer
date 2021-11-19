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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <Windows.h>
  #define strdup _strdup
#else
  #include <unistd.h>
#endif
#include "event_instrumenting.h"

// Globals only visibile in this file, and shared by all threads
static int num_elements = 0;
static volatile bool do_processing = false; // Using volatile to avoid compiler optimizations
static uint16_t sqrt_start_id = 0;
static uint16_t sqrt_end_id = 0;
static uint16_t sleep_start_id = 0;
static uint16_t sleep_end_id = 0;
static double **A_list = NULL;
static double **B_list = NULL;

static void *thread(void *user_data) {
  uint16_t index = (uint16_t)((uint64_t)user_data);
  double *A = A_list[index];
  double *B = B_list[index];

  // CPU expensive but simple and responsive way to do a barrier
  while (!do_processing);

  // Do the processing
  while (do_processing) {
    ukRecordEvent(G_instance, sqrt_start_id, 0.0, __FILE__, __FUNCTION__, __LINE__);
    for (int i=0; i<num_elements; i++) {
      B[i] = sqrt(A[i]);
    }
    ukRecordEvent(G_instance, sqrt_end_id, 0.0, __FILE__, __FUNCTION__, __LINE__);
  }

  return NULL;
}

int main(int argc, char **argv) {
  // Get arguments
  if (argc != 3) { printf("usage: %s <max_threads> <num_elements>\n", argv[0]); return 1; }
  int max_threads = atoi(argv[1]);
  num_elements = atoi(argv[2]);

  // Create folders
  int id = 1;
  int num_folders = max_threads;
  UkFolderInfo *folders = malloc(num_folders*sizeof(UkFolderInfo));
  for (int i=0; i<max_threads; i++) {
    char name[100];
    snprintf(name, 100, "%d Concurrent %s", i+1, i==0 ? "Thread" : "Threads");
    folders[i].name = strdup(name);
    folders[i].id = id++;
  }
  // Create event types
  sqrt_start_id = id++;
  sqrt_end_id = id++;
  sleep_start_id = id++;
  sleep_end_id = id++;
  int num_event_types = 2;
  UkEventInfo *event_types = malloc(num_event_types*sizeof(UkEventInfo));
  event_types[0].name = "Sqrt";
  event_types[0].start_id = sqrt_start_id;
  event_types[0].end_id = sqrt_end_id;
  event_types[0].rgb = UK_BLUE;
  event_types[1].name = "Sleep";
  event_types[1].start_id = sleep_start_id;
  event_types[1].end_id = sleep_end_id;
  event_types[1].rgb = UK_RED;
  // Create event session
  const char *filename = "./memory_contention.events";
  uint32_t max_events = 1000000;
  bool flush_when_full = false;
  bool is_threaded = true;
  bool record_instance = true;
  bool record_value = false;
  bool record_location = false;
  initEventIntrumenting(filename, max_events, flush_when_full, is_threaded, record_instance, record_value, record_location, num_folders, folders, num_event_types, event_types);

  // Allocate vectors
  A_list = malloc(max_threads*sizeof(double*));
  for (int i=0; i<max_threads; i++) {
    A_list[i] = malloc(num_elements*sizeof(double));
    for (int j=0; j<num_elements; j++) {
      A_list[i][j] = fabs((double)rand());
    }
  }
  B_list = malloc(max_threads*sizeof(double*));
  for (int i=0; i<max_threads; i++) {
    B_list[i] = malloc(num_elements*sizeof(double));
  }

  // Run the threads
  for (uint16_t num_concurrent_threads=1; num_concurrent_threads<=max_threads; num_concurrent_threads++) {
    uint16_t folder_id = num_concurrent_threads;
    ukRecordFolder(G_instance, folder_id);

    // Start threads
    pthread_t *thread_ids = malloc(num_concurrent_threads*sizeof(pthread_t));
    for (uint16_t i=0; i<num_concurrent_threads; i++) {
      pthread_create(&thread_ids[i], NULL, thread, (void *)((uint64_t)i));
    }

    // Allow the threads to start processing
    printf("Processing across %d %s\n", num_concurrent_threads, num_concurrent_threads==1 ? "thread" : "concurrent threads");
    do_processing = true;

    // Sleep 1 second
    ukRecordEvent(G_instance, sleep_start_id, 0.0, __FILE__, __FUNCTION__, __LINE__);
#ifdef _WIN32
    Sleep(100);
#else
    usleep(100000);
#endif
    ukRecordEvent(G_instance, sleep_end_id, 0.0, __FILE__, __FUNCTION__, __LINE__);

    // Stop processing
    do_processing = false;

    // Wait for threads to complete processing
    for (int i=0; i<num_concurrent_threads; i++) {
      pthread_join(thread_ids[i], NULL);
    }

    ukCloseFolder(G_instance);
    free(thread_ids);
  }

  // Clean up
  ukFlush(G_instance);
  finalizeEventIntrumenting();
  for (int i=0; i<max_threads; i++) {
    free(A_list[i]);
    free(B_list[i]);
  }
  free(A_list);
  free(B_list);
  for (int i=0; i<max_threads; i++) {
    free((void *)folders[i].name);
  }
  free(folders);
  free(event_types);
  printf("Events were recorded to the file '%s'. Use the Unikorn Viewer to view the results.\n", filename);

  return 0;
}