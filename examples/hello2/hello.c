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

#define DEFINE_FOLDERS_AND_EVENTS
#include "custom_folders_and_events.h"
#include <stdio.h>
#include <stdlib.h>

// Define the event session global variable
EVENTS_GLOBAL_INSTANCE;

static void swap(int *a, int *b) {
  int temp = *a;
  *a = *b;
  *b = temp;
}

static int split(int *element_list, int first, int last) { 
  int pivot = element_list[last];
  int mid = first;
  for (int j=first; j<last; j++) {
    if (element_list[j] < pivot) {
      swap(&element_list[mid], &element_list[j]);
      mid++;
    }
  }
  swap(&element_list[mid], &element_list[last]);
  return mid;
}

static void myQuickSort(int *element_list, int first, int last) {
  if (first < last) {
    int mid = split(element_list, first, last);
    myQuickSort(element_list, first, mid-1);
    myQuickSort(element_list, mid+1, last);
  }
}

static void myBubbleSort(int *element_list, int num_elements) {
  for (int i=0; i<num_elements-1; i++) {
    for (int j=1; j<num_elements-i; j++) {
      if (element_list[j-1] > element_list[j]) {
        swap(&element_list[j-1], &element_list[j]);
      }
    }
  }
}

int main() {
  // Create event session
#ifdef INSTRUMENT_APP
  const char *filename = "./hello.events";
  uint32_t max_events = 10000;
  bool flush_when_full = false;
  bool is_threaded = true;
  bool record_instance = true;
  bool record_value = true;
  bool record_location = true;
#endif
  EVENTS_INIT(filename, max_events, flush_when_full, is_threaded, record_instance, record_value, record_location);

  // Allocate the number lists
  int num_elements = 10000;
  int *number_list = malloc(num_elements*sizeof(int));
  int *quick_sort_list = malloc(num_elements*sizeof(int));
  int *bubble_sort_list = malloc(num_elements*sizeof(int));
  for (int i=0; i<num_elements; i++) {
    number_list[i] = rand();
  }

  // Do the sorting multiple times to see if the timing changes from iteration to iteration
  for (int j=0; j<10; j++) {
    // Prepare the lists
    printf("Sorting: round %d\n", j+1);
    for (int i=0; i<num_elements; i++) {
      quick_sort_list[i] = number_list[i];
      bubble_sort_list[i] = number_list[i];
    }

    // Quick sort
    EVENTS_START_QUICK_SORT();
    myQuickSort(quick_sort_list, 0, num_elements-1);
    EVENTS_END_QUICK_SORT();

    // Bubble sort
    EVENTS_START_BUBBLE_SORT();
    myBubbleSort(bubble_sort_list, num_elements);
    EVENTS_END_BUBBLE_SORT();
  }

  // Clean up
  EVENTS_FLUSH();
  EVENTS_FINALIZE();
  free(number_list);
  free(quick_sort_list);
  free(bubble_sort_list);
#ifdef INSTRUMENT_APP
  printf("Events were recorded to the file '%s'. Use the Unikorn Viewer to view the results.\n", filename);
#else
  printf("Event recording is not enabled.\n");
#endif

  return 0;
}
