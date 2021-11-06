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

EventTree::EventTree(Events *_events, QString _filename) {
  events = _events;
  filename = _filename;
  // Create the root of the tree
  tree = new EventTreeNode();
  tree->tree_node_type = TREE_NODE_IS_FILE;
  // Recursively build tree
  uint32_t event_index = 0;
  QList<uint16_t> open_folders;
  buildTree(tree, event_index, open_folders);
}

EventTree::~EventTree() {
  /*+ recursively free tree */
  /*+*/printf("Delete tree\n");
}

EventTreeNode *EventTree::getChildWithEventId(EventTreeNode *parent, uint16_t event_id) {
  QList<EventTreeNode*> children;
  for (auto child: parent->children) {
    if (child->tree_node_type == TREE_NODE_IS_FILE && child->ID == event_id) return child;
  }
  return NULL;
}

void EventTree::buildTree(EventTreeNode *node, uint32_t &event_index, QList<uint16_t> &/*+open_folders*/) {
  while (true) {
    if (event_index == events->event_count) return; // Done processing events

    // Look at next event
    Event *event = &events->event_buffer[event_index];
    if (event->event_id < events->folder_info_count) {
      // Process folder
      /*+*/
    } else {
      // Process event
      /*+ check if threaded and put in folder */
      EventTreeNode *child = getChildWithEventId(node, event->event_id);
      if (child == NULL) {
        // Create event child
        /*+*/printf("Creating event node for ID %d\n", event->event_id);
        child = new EventTreeNode();
        child->tree_node_type = TREE_NODE_IS_EVENT;
        child->ID = event->event_id;
        child->max_event_instances = MIN_EVENT_INSTANCE_LIST_ELEMENTS;
        child->num_event_instances = 0;
        child->event_indices = (uint32_t *)malloc(child->max_event_instances * sizeof(uint32_t));
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

void EventTree::sortTree(SortType /*+sort_type*/) {
  /*+*/
}
