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
#include <QRect>
#include <QColor>
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
  bool is_open = true;
  uint16_t thread_index = 0;
  uint16_t event_info_index = 0;
  uint16_t ID = 0;
  QColor color;
  uint64_t first_time = 0; // A convenince here for sorting
  QString name; // A convenince here for sorting
  uint32_t max_event_instances = 0;
  uint32_t num_event_instances = 0;
  uint32_t *event_indices = NULL; // Ordered list of indices into the events
  QRect events_row_rect;
  QRect hierarchy_row_rect;
  QRect folder_rect;
  bool row_selected = false;
  QList<EventTreeNode*> children;
};

class EventTree {
public:
  Events *events;
  QString name;
  QString folder;
  EventTreeNode *tree;

  EventTree(Events *events, QString name, QString folder, bool show_folders, bool show_threads);
  ~EventTree();
  void sortTree(SortType sort_type);
  void openAllFolders();
  void closeAllFolders();

private:
  void buildTree(EventTreeNode *node, uint32_t &event_index, bool show_folders, bool show_threads);
  void deleteTree(EventTreeNode *node);
  EventTreeNode *getChildWithEventInfoIndex(EventTreeNode *parent, uint16_t event_info_index);
  EventTreeNode *getThreadFolder(EventTreeNode *parent, uint16_t thread_index);
  void printTree(EventTreeNode *parent, const char *title, int level);
  void sortNode(EventTreeNode *parent, SortType sort_type);
  void setFoldersExpanded(EventTreeNode *parent, bool is_expanded);
};

#endif
