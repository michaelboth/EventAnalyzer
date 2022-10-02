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
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <sys/timeb.h>

//#define USE_ZERO_BASE_TIME

uint64_t ukGetTime() {
#ifdef USE_ZERO_BASE_TIME
  static uint64_t base_time;
  static bool got_base_time = false;
#endif

  // This is monotonic & high precision time
  static bool got_frequency = false;
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

#ifdef USE_ZERO_BASE_TIME
  if (!got_base_time) {
    base_time = total_nanoseconds;
    got_base_time = true;
  }
  total_nanoseconds -= base_time;
#endif

  return total_nanoseconds;
}
