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

#if !defined(USE_clock_gettime_MONOTONIC_CLOCK) && !defined(USE_gettimeofday_CLOCK) && !defined(USE_QueryPerformanceCounter_CLOCK) && !defined(USE_ftime_CLOCK)
need to define one of USE_clock_gettime_MONOTONIC_CLOCK, USE_gettimeofday_CLOCK, USE_QueryPerformanceCounter_CLOCK, USE_ftime_CLOCK
#endif

#include "event_clocks.h"
#include <stdbool.h>
#include <assert.h>
#if defined(USE_QueryPerformanceCounter_CLOCK) || defined(USE_ftime_CLOCK)
  #define WIN32_LEAN_AND_MEAN
  #include <Windows.h>
  #include <sys/timeb.h>
#endif
#ifdef USE_clock_gettime_MONOTONIC_CLOCK
  #include <time.h>
#endif
#ifdef USE_gettimeofday_CLOCK
  #include <stddef.h>
  #include <sys/time.h>
#endif

uint64_t getEventTime() {
  static uint64_t base_time;
  static bool got_base_time = 0;

#ifdef USE_QueryPerformanceCounter_CLOCK
  // This is monotonic & high precision time
  bool got_frequency = false;
  static LARGE_INTEGER frequency;
  if (!got_frequency) {
    bool ok = QueryPerformanceFrequency(&frequency);
    assert(ok);
    got_frequency = true;
  }
  LARGE_INTEGER count;
  bool ok = QueryPerformanceCounter((LARGE_INTEGER *)&count);
  assert(ok);
  uint64_t seconds = count.QuadPart / frequency.QuadPart;
  uint64_t nanoseconds = (1000000000 * (count.QuadPart % frequency.QuadPart)) / frequency.QuadPart;
  uint64_t total_nanoseconds = (seconds * 1000000000) + nanoseconds;
#endif

#ifdef USE_ftime_CLOCK
  // Resolution is really bad
  struct _timeb curr_time;
  _ftime64_s(&curr_time);
  uint64_t total_nanoseconds = (uint64_t)curr_time.time * 1000000000 + (uint64_t)curr_time.millitm * 1000000;
#endif

#ifdef USE_clock_gettime_MONOTONIC_CLOCK
  // clock_gettime() has nanosecond precision
  struct timespec curr_time;
  clock_gettime(CLOCK_MONOTONIC, &curr_time); // CLOCK_MONOTONIC is only increasing
  uint64_t total_nanoseconds = ((uint64_t)curr_time.tv_sec * 1000000000) + (uint64_t)curr_time.tv_nsec;
#endif

#ifdef USE_gettimeofday_CLOCK
  // gettimeofday() only has microsecond precision but is more portable
  struct timeval tp;
  gettimeofday(&tp, NULL);
  uint64_t total_nanoseconds = (uint64_t)tp.tv_sec * 1000000000 + (uint64_t)tp.tv_usec * 1000;
#endif

  if (!got_base_time) {
    base_time = total_nanoseconds;
    got_base_time = 1;
  }
  return (total_nanoseconds - base_time);
}
