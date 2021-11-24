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

#include "event_recorder_clock.h"
#include <stdbool.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <sys/timeb.h>

//#define USE_ZERO_BASE_TIME

uint64_t getEventTime() {
#ifdef USE_ZERO_BASE_TIME
  static uint64_t base_time;
  static bool got_base_time = 0;
#endif

  // Resolution is really bad
  struct _timeb curr_time;
  _ftime64_s(&curr_time);
  uint64_t total_nanoseconds = (uint64_t)curr_time.time * 1000000000 + (uint64_t)curr_time.millitm * 1000000;

#ifdef USE_ZERO_BASE_TIME
  if (!got_base_time) {
    base_time = total_nanoseconds;
    got_base_time = 1;
  }
  total_nanoseconds -= base_time;
#endif

  return total_nanoseconds;
}
