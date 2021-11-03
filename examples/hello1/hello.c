#include "unikorn.h"
#include <stdio.h>
#include <stdlib.h>

enum {
  // Folders
  // Events
  QUICK_SORT_START_ID=1,
  QUICK_SORT_END_ID,
  BUBBLE_SORT_START_ID,
  BUBBLE_SORT_END_ID
};

UkEventInfo events[] = {
  { "Quick Sort",  QUICK_SORT_START_ID,  QUICK_SORT_END_ID,  UK_BLUE},
  { "Bubble Sort", BUBBLE_SORT_START_ID, BUBBLE_SORT_END_ID, UK_RED}
};

UkAttrs attrs = {
  .max_event_count = 10000,     // Max number of events that can be stored in the circular buffer
  .flush_when_full = false,     // Only record when the app explicitly calls ukFlush();
  .is_threaded = true,          // Record the thread ID; each thread will be displayed as a folder in the GUI
  .record_instance = true,      // Record the couter indicating how many times this event was recorded
  .record_value = true,         // Record a 64 bit double value with the event
  .record_file_location = true, // Record file name, function name, and line number where the event was recorded
  .folder_info_count = 0,
  .folder_info_list = NULL,
  .event_info_count = sizeof(events) / sizeof(UkEventInfo),
  .event_info_list = events,
};

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <Windows.h>
  #include <sys/timeb.h>
#else
  #include <stddef.h>
  #include <sys/time.h>
#endif

uint64_t getEventTime() {
  static uint64_t base_time;
  static bool got_base_time = 0;
#ifdef _WIN32
  // A common Windows clock
  struct _timeb curr_time;
  _ftime64_s(&curr_time);
  uint64_t total_nanoseconds = (uint64_t)curr_time.time * 1000000000 + (uint64_t)curr_time.millitm * 1000000;
#else
  // gettimeofday() only has microsecond precision but is more portable
  struct timeval tp;
  gettimeofday(&tp, NULL);
  uint64_t total_nanoseconds = (uint64_t)tp.tv_sec * 1000000000 + (uint64_t)tp.tv_usec * 1000;
#endif
  if (!got_base_time) {
    base_time = total_nanoseconds;
    got_base_time = 1;
  }
  return (total_nanoseconds - base_time); // Return a time starting from 0 indicating when the app started
}

typedef struct {
  const char *filename;
  FILE *file;
  bool events_saved;
  bool append_subsequent_saves;
} FileFlushInfo;

bool prepareFileFlush(void *user_data) {
  FileFlushInfo *flush_info = (FileFlushInfo *)user_data;
  // Open a file
  const char *save_mode = "wb";
  if (flush_info->events_saved && flush_info->append_subsequent_saves) {
    save_mode = "ab";
  }
#ifdef _WIN32
  errno_t status = fopen_s(&flush_info->file, flush_info->filename, save_mode);
  if (status != 0) {
    return false;
  }
#else
  flush_info->file = fopen(flush_info->filename, save_mode);
  if (flush_info->file == NULL) {
    return false;
  }
#endif
  return true;
}

bool fileFlush(void *user_data, const void *data, size_t bytes) {
  FileFlushInfo *flush_info = (FileFlushInfo *)user_data;
  if (fwrite(data, bytes, 1, flush_info->file) == 1) return true;
  return false;
}

bool finishFileFlush(void *user_data) {
  FileFlushInfo *flush_info = (FileFlushInfo *)user_data;
  int rc = fclose(flush_info->file);
  return (rc == 0);
}

FileFlushInfo flush_info = {
  .filename = "./hello.events",
  .file = NULL,
  .events_saved = false,
  .append_subsequent_saves = false
};

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
  void *session = ukCreate(&attrs, getEventTime, &flush_info, prepareFileFlush, fileFlush, finishFileFlush);

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
    ukRecordEvent(session, QUICK_SORT_START_ID, 0.0, __FILE__, __FUNCTION__, __LINE__);
    myQuickSort(quick_sort_list, 0, num_elements-1);
    ukRecordEvent(session, QUICK_SORT_END_ID, 0.0, __FILE__, __FUNCTION__, __LINE__);

    // Bubble sort
    ukRecordEvent(session, BUBBLE_SORT_START_ID, 0.0, __FILE__, __FUNCTION__, __LINE__);
    myBubbleSort(bubble_sort_list, num_elements);
    ukRecordEvent(session, BUBBLE_SORT_END_ID, 0.0, __FILE__, __FUNCTION__, __LINE__);
  }

  // Clean up
  ukFlush(session);
  ukDestroy(session);
  free(number_list);
  free(quick_sort_list);
  free(bubble_sort_list);
  printf("Events were recorded to the file '%s'. Use the Unikorn GUI to view the results.\n", flush_info.filename);

  return 0;
}
