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

#include "EventTree.hpp"

#define MIN_EVENT_INSTANCE_LIST_ELEMENTS 100
#define PRINT_HELPFUL_MESSAGES

EventTree::EventTree(Events *_events, QString _name, QString _folder) {
  events = _events;
  name = _name;
  folder = _folder;
  // Create the root of the tree
  tree = new EventTreeNode();
  tree->tree_node_type = TREE_NODE_IS_FILE;
  tree->name = _folder + "/" + _name + ".events";
  // Recursively build tree
  uint32_t event_index = 0;
  QList<uint16_t> open_folders;
  buildTree(tree, event_index, open_folders);
}

EventTree::~EventTree() {
  deleteTree(tree);
}

void EventTree::deleteTree(EventTreeNode *node) {
#ifdef PRINT_HELPFUL_MESSAGES
  printf("deleteTree: %s\n", node->name.toLatin1().data());
#endif
  for (auto child: node->children) {
    deleteTree(child);
  }
  if (node->max_event_instances > 0) {
    free(node->event_indices);
  }
  delete node;
}

EventTreeNode *EventTree::getChildWithEventInfoIndex(EventTreeNode *parent, uint16_t event_info_index) {
  for (auto child: parent->children) {
    if (child->tree_node_type == TREE_NODE_IS_EVENT && child->event_info_index == event_info_index) return child;
  }
  return NULL;
}

EventTreeNode *EventTree::getThreadFolder(EventTreeNode *parent, uint16_t thread_index) {
  for (auto child: parent->children) {
    if (child->tree_node_type == TREE_NODE_IS_THREAD && child->thread_index == thread_index) return child;
  }
  // Does not exist yet so create it
  assert(thread_index < events->thread_id_count);
  EventTreeNode *thread_folder = new EventTreeNode();
  thread_folder->tree_node_type = TREE_NODE_IS_THREAD;
  thread_folder->thread_index = thread_index;
  thread_folder->name = "Thread " + QString::number(events->thread_id_list[thread_index]);
  parent->children += thread_folder;
#ifdef PRINT_HELPFUL_MESSAGES
  printf("Creating thread folder for index %d, name=%s\n", thread_index, thread_folder->name.toLatin1().data());
#endif
  return thread_folder;
}

void EventTree::buildTree(EventTreeNode *node, uint32_t &event_index, QList<uint16_t> &/*+open_folders*/) {
  while (true) {
    if (event_index == events->event_count) return; // Done processing events

    // Look at next event
    Event *event = &events->event_buffer[event_index];
    if (event->event_id < events->folder_info_count) {
      // Process folder
      /*+ process folder */

    } else {
      // Process event
      uint16_t first_event_id = (events->folder_info_count==0) ? 1 : events->folder_info_count;
      uint16_t event_info_index = (event->event_id - first_event_id) / 2;
      EventInfo *event_info = &events->event_info_list[event_info_index];
      EventTreeNode *parent = node;
      // Get thread folder if threaded
      if (events->is_threaded) { /*+ only if thread folders are visible? */
        parent = getThreadFolder(node, event->thread_index);
      }
      EventTreeNode *child = getChildWithEventInfoIndex(parent, event_info_index);
      if (child == NULL) {
        // Create event child
#ifdef PRINT_HELPFUL_MESSAGES
        printf("Creating event node for ID %d, %s\n", event->event_id, event_info->name);
#endif
        child = new EventTreeNode();
        child->tree_node_type = TREE_NODE_IS_EVENT;
        child->event_info_index = event_info_index;
        child->first_time = event->time;
        child->ID = event->event_id;
        child->name = event_info->name;
        child->max_event_instances = MIN_EVENT_INSTANCE_LIST_ELEMENTS;
        child->num_event_instances = 0;
        child->event_indices = (uint32_t *)malloc(child->max_event_instances * sizeof(uint32_t));
	parent->children += child;
      }
      if (child->num_event_instances == child->max_event_instances) {
        // Instance list is full, double its size
        child->max_event_instances *= 2;
        child->event_indices = (uint32_t *)realloc(child->event_indices, child->max_event_instances * sizeof(uint32_t));
      }
      child->event_indices[child->num_event_instances] = event_index;
      child->num_event_instances++;
    }

    event_index++;
  }
}

static bool eventNameIncreasing(const EventTreeNode *a, const EventTreeNode *b) {
  return a->name < b->name;
}

static bool eventIdIncreasing(const EventTreeNode *a, const EventTreeNode *b) {
  // IMPORTANT: container will have the same ID, so sort by name
  if (a->ID == b->ID) return a->name < b->name;
  return a->ID < b->ID;
}

static bool eventFirstTimeIncreasing(const EventTreeNode *a, const EventTreeNode *b) {
  // IMPORTANT: container will have the same starting time, so sort by name
  if (a->first_time == b->first_time) return a->name < b->name;
  return a->first_time < b->first_time;
}

void EventTree::printTree(EventTreeNode *parent, SortType sort_type, const char *title, int level) {
  if (level == 0) {
    printf("%s sorted by %s:\n", title, (sort_type==SORT_BY_NAME) ? "name" : (sort_type==SORT_BY_ID) ? "ID" : "time");
  }
  for (auto child: parent->children) {
    for (int i=0; i<level*2; i++) printf(" ");
    printf("  %s: %s, ID=%d, first_time=%zu\n",
           (child->tree_node_type == TREE_NODE_IS_FILE) ? "FILE" : (child->tree_node_type == TREE_NODE_IS_FOLDER) ? "FOLDER" : (child->tree_node_type == TREE_NODE_IS_THREAD) ? "THREAD" : "EVENT",
           child->name.toLatin1().data(), child->ID, child->first_time);
    if (child->tree_node_type == TREE_NODE_IS_FILE || child->tree_node_type == TREE_NODE_IS_FOLDER || child->tree_node_type == TREE_NODE_IS_THREAD) {
      printTree(child, sort_type, NULL, level+1);
    }
  }
}

void EventTree::sortNode(EventTreeNode *parent, SortType sort_type) {
  // Sort the imediate leaves of this node
  if (parent->children.count() > 1) {
    std::sort(parent->children.begin(), parent->children.end(), (sort_type == SORT_BY_NAME) ? eventNameIncreasing : (sort_type == SORT_BY_ID) ? eventIdIncreasing : eventFirstTimeIncreasing);
  }
  // Recurse into childer if they are containers
  for (auto child: parent->children) {
    if (child->tree_node_type == TREE_NODE_IS_FILE || child->tree_node_type == TREE_NODE_IS_FOLDER || child->tree_node_type == TREE_NODE_IS_THREAD) {
      sortNode(child, sort_type);
    }
  }
}

void EventTree::sortTree(SortType sort_type) {
  sortNode(tree, sort_type);
#ifdef PRINT_HELPFUL_MESSAGES
  printTree(tree, sort_type, name.toLatin1().data(), 0);
#endif
}
