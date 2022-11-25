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
#include "main.hpp" // Needed for G_event_filters

#ifdef __APPLE__
  #define UINT64_FORMAT "llu"
#else
  #define UINT64_FORMAT "zu"
#endif

#define MIN_EVENT_INSTANCE_LIST_ELEMENTS 100
//#define PRINT_HELPFUL_MESSAGES

bool EventTreeNode::isAncestorCollapsed() {
  EventTreeNode *ancestor = this;
  while (ancestor != NULL) {
    if (!ancestor->is_open) return true;
    ancestor = ancestor->parent;
  }
  return false;
}

EventTree::EventTree(UkEvents *_events, QString _name, QString _folder, bool show_folders, bool show_threads) {
  events = _events;
  name = _name;
  folder = _folder;
  // Create the root of the tree
  tree = new EventTreeNode();
  tree->tree_node_type = TREE_NODE_IS_FILE;
  tree->name = _name + "   (" + _folder + ")";
  // Recursively build tree
  uint32_t event_index = 0;
  buildTree(tree, event_index, show_folders, show_threads);
  clearEventGhosting();
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

void EventTree::clearEventGhosting() {
  for (uint32_t i=0; i<events->event_count; i++) {
    events->event_buffer[i].is_ghosted = false;
  }
}

EventTreeNode *EventTree::getChildWithEventInfoIndex(EventTreeNode *parent, uint16_t event_registration_index) {
  for (auto child: parent->children) {
    if (child->tree_node_type == TREE_NODE_IS_EVENT && child->event_registration_index == event_registration_index) return child;
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
  thread_folder->parent = parent;
#ifdef PRINT_HELPFUL_MESSAGES
  printf("Creating thread folder for index %d, name=%s\n", thread_index, thread_folder->name.toLatin1().data());
#endif
  return thread_folder;
}

void EventTree::buildTree(EventTreeNode *node, uint32_t &event_index, bool show_folders, bool show_threads) {
  while (true) {
    if (event_index == events->event_count) return; // Done processing events

    // Look at next event
    UkEvent *event = &events->event_buffer[event_index];
    if (event->event_id < events->folder_registration_count) {
      // Process folder event
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
        folder->name = events->folder_registration_list[event->event_id].name;
        node->children += folder;
        folder->parent = node;
#ifdef PRINT_HELPFUL_MESSAGES
        printf("Creating folder for ID %d, name=%s\n", event->event_id, folder->name.toLatin1().data());
#endif
        // Recurse into folder node
        event_index++;
        buildTree(folder, event_index, show_folders, show_threads);
      }

    } else {
      // Process time event
      uint16_t first_event_id = (events->folder_registration_count==0) ? 1 : events->folder_registration_count;
      uint16_t event_registration_index = (event->event_id - first_event_id) / 2;
      UkLoaderEventRegistration *event_registration = &events->event_registration_list[event_registration_index];
      EventTreeNode *parent = node;

      // Get thread folder if threaded
      if (events->is_multi_threaded && show_threads) {
        parent = getThreadFolder(node, event->thread_index);
      }

      // Get the EventTreeNode (create one if doesn't exist)
      EventTreeNode *child = getChildWithEventInfoIndex(parent, event_registration_index);
      if (child == NULL) {
        // Create event child
#ifdef PRINT_HELPFUL_MESSAGES
        printf("Creating event node for ID %d, %s, event_registration_index %d, first_event_id %d\n", event->event_id, event_registration->name, event_registration_index, first_event_id);
#endif
        child = new EventTreeNode();
        child->tree_node_type = TREE_NODE_IS_EVENT;
        child->event_registration_index = event_registration_index;
        child->row_start_time = event->time;
        child->ID = event_registration->start_id;
        int red   = (int)(255 * (((event_registration->rgb & 0xf00) >> 8) / (float)0xf));
        int green = (int)(255 * (((event_registration->rgb & 0xf0) >> 4) / (float)0xf));
        int blue  = (int)(255 * ((event_registration->rgb & 0xf) / (float)0xf));
        child->color = QColor(red, green, blue);
        child->name = event_registration->name;
        child->max_event_instances = MIN_EVENT_INSTANCE_LIST_ELEMENTS;
        child->num_event_instances = 0;
        child->event_indices = (uint32_t *)malloc(child->max_event_instances * sizeof(uint32_t));
	parent->children += child;
        child->parent = parent;
      }

      // Double the event index buffer if it's full
      if (child->num_event_instances == child->max_event_instances) {
        // Index list is full, double its size
        child->max_event_instances *= 2;
        child->event_indices = (uint32_t *)realloc(child->event_indices, child->max_event_instances * sizeof(uint32_t));
      }

      // Add the event index to the event index buffer
      child->event_indices[child->num_event_instances] = event_index;

      // See if this is the largest duration (only check if there are at least two events in the list)
      if (child->num_event_instances > 0 && event->event_id == event_registration->end_id) {
        // This is an end event, and a prev event exists. See if the prev event is a start event
        uint32_t prev_event_index = child->event_indices[child->num_event_instances-1];
        UkEvent *prev_event = &events->event_buffer[prev_event_index];
        if (prev_event->event_id == event_registration->start_id) {
          // Prev event is the start. See if the duration is the largest
          uint64_t duration = event->time - prev_event->time;
          if (child->end_event_index_of_largest_duration == 0) {
            // This is the first duration
            child->end_event_index_of_largest_duration = child->num_event_instances;
          } else {
            // Subsequent duration
            uint32_t largest_start_event_index = child->event_indices[child->end_event_index_of_largest_duration-1];
            uint32_t largest_end_event_index = child->event_indices[child->end_event_index_of_largest_duration];
            UkEvent *largest_start_event = &events->event_buffer[largest_start_event_index];
            UkEvent *largest_end_event = &events->event_buffer[largest_end_event_index];
            uint64_t largest_duration = largest_end_event->time - largest_start_event->time;
            if (duration > largest_duration) {
              // This is larger
              child->end_event_index_of_largest_duration = child->num_event_instances;
            }
          }
        }
      }

      child->num_event_instances++;
      event_index++;
    }
  }
}

uint32_t EventTreeNode::filteredEventInstanceCount() {
  if (tree_node_type == TREE_NODE_IS_EVENT && G_event_filters.contains(name)) return num_event_instances;
  uint32_t count = 0;
  for (auto child: children) {
    count += child->filteredEventInstanceCount();
  }
  return count;
}

void EventTree::printTree(EventTreeNode *parent, const char *title, int level) {
  if (level == 0) {
    printf("%s\n", title);
  }
  if (parent->is_open) {
    for (auto child: parent->children) {
      for (int i=0; i<level*2; i++) printf(" ");
      printf("  %s: %s, ID=%d, row_start_time=%" UINT64_FORMAT "\n",
             (child->tree_node_type == TREE_NODE_IS_FILE) ? "FILE" : (child->tree_node_type == TREE_NODE_IS_FOLDER) ? "FOLDER" : (child->tree_node_type == TREE_NODE_IS_THREAD) ? "THREAD" : "EVENT",
             child->name.toLatin1().data(), child->ID, child->row_start_time);
      if (child->tree_node_type == TREE_NODE_IS_FILE || child->tree_node_type == TREE_NODE_IS_FOLDER || child->tree_node_type == TREE_NODE_IS_THREAD) {
        printTree(child, NULL, level+1);
      }
    }
  }
}

static bool eventNameIncreasing(const EventTreeNode *a, const EventTreeNode *b) {
  if (a->tree_node_type == TREE_NODE_IS_EVENT && b->tree_node_type != TREE_NODE_IS_EVENT) return true;
  if (a->tree_node_type != TREE_NODE_IS_EVENT && b->tree_node_type == TREE_NODE_IS_EVENT) return false;
  return a->name < b->name;
}

static bool eventIdIncreasing(const EventTreeNode *a, const EventTreeNode *b) {
  if (a->tree_node_type == TREE_NODE_IS_EVENT && b->tree_node_type != TREE_NODE_IS_EVENT) return true;
  if (a->tree_node_type != TREE_NODE_IS_EVENT && b->tree_node_type == TREE_NODE_IS_EVENT) return false;
  // IMPORTANT: container will have the same ID, so sort by name
  if (a->ID == b->ID) return a->name < b->name;
  return a->ID < b->ID;
}

static bool eventFirstTimeIncreasing(const EventTreeNode *a, const EventTreeNode *b) {
  if (a->tree_node_type == TREE_NODE_IS_EVENT && b->tree_node_type != TREE_NODE_IS_EVENT) return true;
  if (a->tree_node_type != TREE_NODE_IS_EVENT && b->tree_node_type == TREE_NODE_IS_EVENT) return false;
  // IMPORTANT: container will have the same starting time, so sort by name
  if (a->row_start_time == b->row_start_time) return a->name < b->name;
  return a->row_start_time < b->row_start_time;
}

void EventTree::sortNode(EventTreeNode *parent, SortType sort_type) {
  // Sort the imediate leaves of this node
  if (parent->children.count() > 1) {
    std::sort(parent->children.begin(), parent->children.end(), (sort_type == SORT_BY_NAME) ? eventNameIncreasing : (sort_type == SORT_BY_ID) ? eventIdIncreasing : eventFirstTimeIncreasing);
  }
  // Recurse into childen if they are containers
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
  if (parent->children.size() > 0) parent->is_open = is_expanded;
  for (auto child: parent->children) {
    setFoldersExpanded(child, is_expanded);
  }
}
