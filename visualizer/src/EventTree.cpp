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

#ifdef __APPLE__
  #define UINT64_FORMAT "llu"
#else
  #define UINT64_FORMAT "zu"
#endif

#define MIN_EVENT_INSTANCE_LIST_ELEMENTS 100
#define PRINT_HELPFUL_MESSAGES

EventTree::EventTree(Events *_events, QString _name, QString _folder, bool show_folders, bool show_threads) {
  events = _events;
  name = _name;
  folder = _folder;
  // Create the root of the tree
  tree = new EventTreeNode();
  tree->tree_node_type = TREE_NODE_IS_FILE;
  tree->name = _folder + "/" + _name + ".events";
  // Recursively build tree
  uint32_t event_index = 0;
  buildTree(tree, event_index, show_folders, show_threads);
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

void EventTree::buildTree(EventTreeNode *node, uint32_t &event_index, bool show_folders, bool show_threads) {
  while (true) {
    if (event_index == events->event_count) return; // Done processing events

    // Look at next event
    Event *event = &events->event_buffer[event_index];
    if (event->event_id < events->folder_info_count) {
      // Process folder
      if (!show_folders) {
        // Skip over folder events
        event_index++;
      } else if (event->event_id == 0) {
        // This is the 'close folder' event. Move back to the parent node
        event_index++;
        return;
      } else {
        // Create the new folder and recurse into it
        EventTreeNode *folder = new EventTreeNode();
        folder->tree_node_type = TREE_NODE_IS_FOLDER;
        folder->ID = event->event_id;
        folder->name = events->folder_info_list[event->event_id].name;
        node->children += folder;
#ifdef PRINT_HELPFUL_MESSAGES
        printf("Creating folder for ID %d, name=%s\n", event->event_id, folder->name.toLatin1().data());
#endif
        // Recurse into folder node
        event_index++;
        buildTree(folder, event_index, show_folders, show_threads);
      }

    } else {
      // Process event
      uint16_t first_event_id = (events->folder_info_count==0) ? 1 : events->folder_info_count;
      uint16_t event_info_index = (event->event_id - first_event_id) / 2;
      EventInfo *event_info = &events->event_info_list[event_info_index];
      EventTreeNode *parent = node;
      // Get thread folder if threaded
      if (events->is_threaded && show_threads) {
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
      event_index++;
    }
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

void EventTree::printTree(EventTreeNode *parent, const char *title, int level) {
  if (level == 0) {
    printf("%s\n", title);
  }
  if (parent->is_open) {
    for (auto child: parent->children) {
      for (int i=0; i<level*2; i++) printf(" ");
      printf("  %s: %s, ID=%d, first_time=%" UINT64_FORMAT "\n",
             (child->tree_node_type == TREE_NODE_IS_FILE) ? "FILE" : (child->tree_node_type == TREE_NODE_IS_FOLDER) ? "FOLDER" : (child->tree_node_type == TREE_NODE_IS_THREAD) ? "THREAD" : "EVENT",
             child->name.toLatin1().data(), child->ID, child->first_time);
      if (child->tree_node_type == TREE_NODE_IS_FILE || child->tree_node_type == TREE_NODE_IS_FOLDER || child->tree_node_type == TREE_NODE_IS_THREAD) {
        printTree(child, NULL, level+1);
      }
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
  QString title = name;
  title += (sort_type==SORT_BY_NAME) ? " sorted by name:" : (sort_type==SORT_BY_ID) ? " sorted by ID:" : " sorted by time:";
  printTree(tree, title.toLatin1().data(), 0);
#endif
}

void EventTree::openAllFolders() {
  setFoldersExpanded(tree, true);
#ifdef PRINT_HELPFUL_MESSAGES
  printTree(tree, name.toLatin1().data(), 0);
#endif
}

void EventTree::closeAllFolders() {
  setFoldersExpanded(tree, false);
#ifdef PRINT_HELPFUL_MESSAGES
  printTree(tree, name.toLatin1().data(), 0);
#endif
}

void EventTree::setFoldersExpanded(EventTreeNode *parent, bool is_expanded) {
  parent->is_open = is_expanded;
  for (auto child: parent->children) {
    setFoldersExpanded(child, is_expanded);
  }
}
