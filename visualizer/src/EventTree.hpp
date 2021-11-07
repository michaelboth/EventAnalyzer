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

#include <QList>
#include "events_loader.h"

typedef enum {
  SORT_BY_ID,
  SORT_BY_NAME,
  SORT_BY_TIME
} SortType;

typedef enum {
  TREE_NODE_IS_FILE,
  TREE_NODE_IS_FOLDER,
  TREE_NODE_IS_THREAD,
  TREE_NODE_IS_EVENT
} TreeNodeType;

class EventTreeNode {
public:
  TreeNodeType tree_node_type = TREE_NODE_IS_FILE;
  uint16_t ID = 0;
  uint32_t max_event_instances = 0;
  uint32_t num_event_instances = 0;
  uint32_t *event_indices = NULL; // Ordered list of indices into the events
  QList<EventTreeNode*> children;
};

class EventTree {
public:
  EventTree(Events *events, QString filename);
  ~EventTree();
  void sortTree(SortType sort_type);
private:
  Events *events;
  QString filename;
  EventTreeNode *tree;
  void buildTree(EventTreeNode *node, uint32_t &event_index, QList<uint16_t> &open_folders);
  void deleteTree(EventTreeNode *node);
  EventTreeNode *getChildWithEventId(EventTreeNode *parent, uint16_t event_id);
};

#endif
