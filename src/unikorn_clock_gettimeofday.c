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

#include "unikorn_clock.h"
#include <stdbool.h>
#ifdef NDEBUG // Don't want assert compiled out
  #undef NDEBUG
#endif
#include <assert.h>
#include <stddef.h>
#include <sys/time.h>

//#define USE_ZERO_BASE_TIME

uint64_t ukGetTime() {
#ifdef USE_ZERO_BASE_TIME
  static uint64_t base_time;
  static bool got_base_time = false;
#endif

  // gettimeofday() only has microsecond precision but is more portable
  struct timeval tp;
  gettimeofday(&tp, NULL);
  uint64_t total_nanoseconds = (uint64_t)tp.tv_sec * 1000000000 + (uint64_t)tp.tv_usec * 1000;

#ifdef USE_ZERO_BASE_TIME
  if (!got_base_time) {
    base_time = total_nanoseconds;
    got_base_time = true;
  }
  total_nanoseconds -= base_time;
#endif

  return total_nanoseconds;
}
