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

#ifndef _EventTree_hpp_
#define _EventTree_hpp_

typedef enum {
  SORT_BY_ID,
  SORT_BY_NAME,
  SORT_BY_TIME
} SortType;

class EventLeaf {
public:
  bool is_folder = false;
  uint16_t ID = 0;
  uint32_t *event_indices = NULL; // Ordered list of indices into the events
  QList<EventLeaf*> children;
};

class EventTree {
public:
  explicit EventTree(void *events, SortType sort_type);
  ~EventTree();
private:
  void *events;
  EventLeaf *tree;
};

#endif
