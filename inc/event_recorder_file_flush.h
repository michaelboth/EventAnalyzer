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

#ifndef _EVENT_RECORDER_FILE_FLUSH_H_
#define _EVENT_RECORDER_FILE_FLUSH_H_

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

typedef struct {
  const char *filename;
  FILE *file;
  bool events_saved;
  bool append_subsequent_saves;
} FileFlushInfo;

#ifdef __cplusplus
extern "C"
{
#endif

extern bool prepareFileFlush(void *user_data);
extern bool fileFlush(void *user_data, const void *data, size_t bytes);
extern bool finishFileFlush(void *user_data);

#ifdef __cplusplus
}
#endif

#endif
