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
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#define DEFINE_FOLDERS_AND_EVENTS
#include "custom_folders_and_events.h"

#define NUM_ITERATIONS 1000

// Globals only visibile in this file, and shared by all threads
static int num_elements = 0;
static volatile bool do_processing = false; // Using volatile to avoid compiler optimizations
static double **A_list = NULL;
static double **B_list = NULL;
#ifdef INSTRUMENT_APP
static void *session = NULL;
#endif

static void *thread(void *user_data) {
  uint16_t index = (uint16_t)((uint64_t)user_data);
  double *A = A_list[index];
  double *B = B_list[index];

  // CPU expensive but simple and responsive way to do a barrier
  while (!do_processing);

  // Do the processing
  for (int i=0; i<NUM_ITERATIONS; i++) {
    EVENTS_START_SQRT(session, i);
    for (int i=0; i<num_elements; i++) {
      B[i] = sqrt(A[i]);
    }
    EVENTS_END_SQRT(session, i);
  }

  return NULL;
}

int main(int argc, char **argv) {
  // Get arguments
  if (argc != 3) { printf("usage: %s <max_threads> <num_elements>\n", argv[0]); return 1; }
  int max_threads = atoi(argv[1]);
  num_elements = atoi(argv[2]);

  // Create a separate file for each thread grouping so it's easy to compare the results in the visualizer
  for (uint16_t num_concurrent_threads=1; num_concurrent_threads<=max_threads; num_concurrent_threads++) {
    printf("Processing across %d %s\n", num_concurrent_threads, num_concurrent_threads==1 ? "thread" : "concurrent threads");

    // Create event session
#ifdef INSTRUMENT_APP
    char filename[100];
    snprintf(filename, 100, "%d_concurrent_%s.events", num_concurrent_threads, num_concurrent_threads==1 ? "thread" : "threads");
    uint32_t max_events = 100000;
    bool flush_when_full = true;
    bool is_threaded = true;
    bool record_instance = true;
    bool record_value = false;
    bool record_location = false;
    FileFlushInfo flush_info;
    session = EVENTS_INIT(filename, max_events, flush_when_full, is_threaded, record_instance, record_value, record_location, &flush_info);
#endif

    // Allocate math resources
    EVENTS_START_ALLOC(session, 0);
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
    EVENTS_END_ALLOC(session, 0);

    // Start threads
    do_processing = false;
    EVENTS_START_INIT_THREADS(session, 0);
    pthread_t *thread_ids = malloc(num_concurrent_threads*sizeof(pthread_t));
    for (uint16_t i=0; i<num_concurrent_threads; i++) {
      pthread_create(&thread_ids[i], NULL, thread, (void *)((uint64_t)i));
    }
    EVENTS_END_INIT_THREADS(session, 0);

    // Allow the threads to start processing
    EVENTS_START_BARRIER(session, 0);
    do_processing = true;
    EVENTS_END_BARRIER(session, 0);

    // Wait for threads to complete processing
    EVENTS_START_JOIN_THREADS(session, 0);
    for (int i=0; i<num_concurrent_threads; i++) {
      pthread_join(thread_ids[i], NULL);
    }
    EVENTS_END_JOIN_THREADS(session, 0);

    // Clean up math resources
    EVENTS_START_FREE(session, 0);
    free(thread_ids);
    for (int i=0; i<max_threads; i++) {
      free(A_list[i]);
      free(B_list[i]);
    }
    free(A_list);
    free(B_list);
    EVENTS_END_FREE(session, 0);

    // Finish the event session
    EVENTS_FLUSH(session);
    EVENTS_FINALIZE(session);
  }

#ifdef INSTRUMENT_APP
  printf("Events were recorded to %d %s. Use the Unikorn Viewer to view the results.\n", max_threads, max_threads==1 ? "file" : "files");
#else
  printf("Event recording is not enabled.\n");
#endif

  return 0;
}
