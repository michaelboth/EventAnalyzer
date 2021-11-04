#include "event_clocks.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef __APPLE__
  #define UINT64_FORMAT "llu"
#else
  #define UINT64_FORMAT "zu"
#endif

int main() {
  // Allocate the number lists
  int num_elements = 10000;
  uint64_t *time_list = malloc(num_elements*sizeof(uint64_t));

  // Run the clock a few times just to get things cached up
  int priming_cycles = 10;
  for (int i=0; i<priming_cycles; i++) {
    time_list[i] = getEventTime();
  }

  // Run the clock lots more
  for (int i=0; i<num_elements; i++) {
    time_list[i] = getEventTime();
  }

  // Calculate the overhead: how long it takes to call getEventTime()
  uint64_t overall_time = time_list[num_elements-1] - time_list[0];
  if (overall_time == 0) {
    printf("Need more than %d itterations to calculate time attributes\n", num_elements);
  } else {
    uint64_t overhead_nanoseconds = overall_time / (num_elements-1);
    printf("Overhead: how long it takes to call getEventTime()\n");
    printf("    %"UINT64_FORMAT" nanoseconds ()\n", overhead_nanoseconds);

    // Calculate the precision: the smallest amount of time it can report
    uint64_t min_non_zero_diff = 9999999;
    for (int i=1; i<num_elements; i++) {
      uint64_t diff = time_list[i] - time_list[i-1];
      if (diff > 0 && diff < min_non_zero_diff) {
        min_non_zero_diff = diff;
      }
    }
    printf("Precision: the smallest amount of time getEventTime() can report\n");
    printf("    %"UINT64_FORMAT" nanoseconds ()\n", min_non_zero_diff);
  }

  // Clean up
  free(time_list);

  return 0;
}
