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

#include <QMouseEvent>
#include <QPainter>
#include "EventsView.hpp"
#include "HelpfulFunctions.hpp"
#include "main.hpp"

/*+ allow right click menu in events area to zoom? Or maybe just use keyboard shortcuts */

#define LINE_SEPARATOR_COLOR QColor(230, 230, 230)
#define MIN_SELECTION_PIXEL_DIFF 10
#define ROLLOVER_BG_COLOR QColor(200, 200, 200)
#define ROLLOVER_TEXT_COLOR QColor(50, 50, 50)

EventsView::EventsView(QWidget *parent) : QWidget(parent) {
  // Track mouse when not pressed
  setMouseTracking(true);

  // Allow keyboard input
  //setFocusPolicy(Qt::StrongFocus);
  // Allow for keyboard focus now
  //setFocus();

  // Load the logo
  QImage image = QImage(":/unikorn_logo.png");
  recolorImage(image, QColor(220,220,220));
  logo = QPixmap::fromImage(image);
}

EventsView::~EventsView() {
  // Nothing to do
}

void EventsView::updateLineHeight() {
  // Calculate geometry
  line_h = (int)(G_font_point_size * LINE_HEIGHT_FACTOR);

  // Update font size
  QFont font = this->font();
  font.setPointSize(G_font_point_size);
  this->setFont(font);
}

void EventsView::updateVOffset(int offset) {
  v_offset = offset;
  rebuildAndUpdate();
}

void EventsView::getTimeRange(double *percent_visible_ret, double *percent_offset_ret) {
  *percent_visible_ret = percent_visible;
  *percent_offset_ret = percent_offset;
}

bool EventsView::timeRangeSelected() {
  return (selected_time_range_x1 != -1 && selected_time_range_x2 != -1);
}

void EventsView::updateTimeOffset(double percent_scroll_offset) {
  double max_offset = 1.0 - percent_visible;
  percent_offset = percent_scroll_offset * max_offset;
  rebuildAndUpdate();
}

void EventsView::zoomToAll() {
  percent_visible = 1.0;
  percent_offset = 0.0;
  selected_time_range_x1 = -1;
  selected_time_range_x2 = -1;
  emit timeRangeChanged();
  rebuildAndUpdate();
}

void EventsView::zoomIn() {
  percent_visible /= 2.0;
  percent_offset += percent_visible/2.0;
  selected_time_range_x1 = -1;
  selected_time_range_x2 = -1;
  emit timeRangeChanged();
  rebuildAndUpdate();
}

void EventsView::zoomOut() {
  percent_offset -= percent_visible/2.0;
  percent_visible *= 2.0;
  if (percent_visible > 1.0) {
    percent_visible = 1.0;
    percent_offset = 0.0;
  }
  if (percent_offset < 0) {
    percent_offset = 0.0;
  }
  if (percent_offset > (1.0 - percent_visible)) {
    percent_offset = 1.0 - percent_visible;
  }
  selected_time_range_x1 = -1;
  selected_time_range_x2 = -1;
  emit timeRangeChanged();
  rebuildAndUpdate();
}

void EventsView::zoomToRegion() {
  if (selected_time_range_x1 != -1 && selected_time_range_x2 != -1) {
    int w = width();
    int x1 = std::min(selected_time_range_x1, selected_time_range_x2);
    int x2 = std::max(selected_time_range_x1, selected_time_range_x2);
    x1 = std::max(x1, 0);
    x2 = std::min(x2, w-1);
    double w_factor = (x2-x1) / (double)w;
    double offset_factor = x1 / (double)w;
    percent_offset += offset_factor*percent_visible;
    percent_visible *= w_factor;
  }
  selected_time_range_x1 = -1;
  selected_time_range_x2 = -1;
  emit timeRangeChanged();
  rebuildAndUpdate();
}

void EventsView::mousePressEvent(QMouseEvent *event) {
  if (G_event_tree_map.count() == 0) return;
  if (event->button() == Qt::LeftButton) {
    // Start selection of time range
    mouse_button_pressed = true;
    selected_time_range_x1 = event->x();
    selected_time_range_x2 = selected_time_range_x1;
    update(); // Avoids full redraw
  }
}

EventTreeNode *EventsView::mouseOnEventsLine(EventTreeNode *parent) {
  if (parent->events_row_rect.contains(mouse_location)) {
    return parent;
  }
  for (auto child: parent->children) {
    EventTreeNode *node_with_mouse = mouseOnEventsLine(child);
    if (node_with_mouse != NULL) return node_with_mouse;
  }
  return NULL;
}

void EventsView::mouseMoveEvent(QMouseEvent *event) {
  if (mouse_button_pressed) {
    selected_time_range_x2 = event->x();
    update(); // Avoids full redraw
  } else {
    // See if mouse is on an event line
    mouse_location = QPoint(event->x(), event->y());
    update(); // Avoids full redraw, but needed to draw rollover
  }
}

void EventsView::mouseReleaseEvent(QMouseEvent * /*event*/) {
  if (mouse_button_pressed) {
    mouse_button_pressed = false;
    int diff = std::abs(selected_time_range_x1 - selected_time_range_x2);
    if (diff < MIN_SELECTION_PIXEL_DIFF) {
      selected_time_range_x1 = -1;
      selected_time_range_x2 = -1;
    }
    emit timeRangeSelectionChanged();
    update(); // Avoids full redraw
  }
}

void EventsView::leaveEvent(QEvent * /*event*/) {
  mouse_location = QPoint(-1,-1);
  update(); // Avoids full redraw
}

static uint32_t findEventIndexAtTime(Events *events, EventTreeNode *node, uint64_t time, int32_t index_offset) {
  // Binary search
  uint32_t first = 0;
  uint32_t last = node->num_event_instances-1;
  while ((last-first) > 1) {
    uint32_t mid = first + (last-first)/2;
    uint32_t event_index = node->event_indices[mid];
    Event *event = &events->event_buffer[event_index];
    if (event->time < time) {
      first = mid;
    } else {
      last = mid;
    }
  }
  if (first > 0 && index_offset < 0) {
    first--;
    index_offset++;
  }
  if (first < (node->num_event_instances-1) && index_offset > 0) {
    first++;
    index_offset--;
  }
  return first;
}

void EventsView::drawHierarchyLine(QPainter *painter, Events *events, EventTreeNode *parent, int &line_index, int ancestor_open) {
  int h = height();
  int w = width();
  int y = -v_offset + line_index * line_h;
  parent->events_row_rect = QRect();

  // only draw if visible
  if (y > -line_h && y < h) {
    // Remember the row geometry
    QRect row_rect = QRect(0,y,w,line_h);
    parent->events_row_rect = row_rect;

    // Highlight row if selected (selected in hierarchy view)
    if (parent->row_selected) {
      painter->fillRect(row_rect, ROW_SELECTED_COLOR);
    }

    if (parent->num_event_instances > 0) {
      // Determin the ragne of events to draw based on visible time region
      uint32_t first_visible_event_index = findEventIndexAtTime(events, parent, start_time, -3);
      uint32_t last_visible_event_index = findEventIndexAtTime(events, parent, end_time, 3);
      //*+*/printf("  Draw event range: first=%u, last=%u\n", first_visible_event_index, last_visible_event_index);

      // Draw events
      int prev_x = 0;
      bool prev_is_start = false;
      EventInfo *event_info = &events->event_info_list[parent->event_info_index];
      painter->setPen(QPen(parent->color, 1, Qt::SolidLine));
      int y2 = y + (int)(line_h * 0.2f);
      int y3 = y + (int)(line_h * 0.4f);
      int y4 = y + (int)(line_h * 0.6f);
      int y5 = y + (int)(line_h * 0.8f);
      int range_h = y4 - y3 + 1;
      for (uint32_t i=first_visible_event_index; i<=last_visible_event_index; i++) {
        uint32_t event_index = parent->event_indices[i];
        Event *event = &events->event_buffer[event_index];
        bool is_start = event->event_id == event_info->start_id;
        // X location in visible region
        double x_percent;
        if (event->time < start_time) {
          // To the left of the visible area
          x_percent = (start_time - event->time) / -time_range;
        } else {
          // In the visible area or to the right
          x_percent = (event->time - start_time) / time_range;
        }
        int x = (int)(x_percent * (w-1));
        int top_y = is_start ? y2 : y3;
        int bottom_y = is_start ? y4 : y5;
        painter->drawLine(x, top_y, x, bottom_y);
        // Draw range
        if (!is_start && prev_is_start && x > prev_x) {
          int range_w = x - prev_x;
          painter->fillRect(QRect(prev_x,y3,range_w,range_h), parent->color);
        }
        // Remember some stuff
        prev_is_start = is_start;
        prev_x = x;
      }
      /*+ record usage to tree, to later display in profiling view, and emit signal when done drawing */
    }

    // Draw separator line
    painter->setPen(QPen(LINE_SEPARATOR_COLOR, 1, Qt::SolidLine));
    int separator_y = y + line_h - 1;
    painter->drawLine(0, separator_y, w, separator_y);
  }

  // Recurse
  if (!parent->is_open) {
    ancestor_open = false;
  }
  for (auto child: parent->children) {
    if (ancestor_open && parent->is_open) line_index++;
    drawHierarchyLine(painter, events, child, line_index, ancestor_open ? parent->is_open : false);
  }
}

void EventsView::paintEvent(QPaintEvent* /*event*/) {
  int w = width();
  int h = height();
  if (w <= 0 || h <= 0) { return; }

  // Define painter
  QPainter painter(this);

  // Fill in background
  painter.fillRect(QRect(0,0,w,h), QColor(255,255,255));

  if (G_event_tree_map.count() == 0) {
    // Draw the logo
    QRect inside_rect = getFittedRect(FitType::Inside, w/2, h, logo.width(), logo.height());
    inside_rect.moveLeft(w/4);
    painter.setRenderHint(QPainter::SmoothPixmapTransform,true);
    painter.drawPixmap(inside_rect, logo);
    painter.setRenderHint(QPainter::SmoothPixmapTransform,false);
    // Clear some stuff just in case it's active
    selected_time_range_x1 = -1;
    selected_time_range_x2 = -1;
    percent_visible = 1.0;
    percent_offset = 0.0;
    emit visibleTimeRangeChanged(0, 0); // Signal to the header
    emit selectionTimeRangeChanged(0);  // Signal to the header
    emit timeRangeSelectionChanged();   // Signal to the toolbar widgets
    return;
  }

  // Determine time range
  start_time = 0;
  end_time = 0;
  {
    bool got_time_range = false;
    QMapIterator<QString, EventTree*> i(G_event_tree_map);
    while (i.hasNext()) {
      // Get old tree info
      i.next();
      EventTree *event_tree = i.value();
      Events *events = event_tree->events;
      Event *first_event = &events->event_buffer[0];
      Event *last_event = &events->event_buffer[events->event_count-1];
      if (!got_time_range) {
        // Record initial times
        start_time = first_event->time;
        end_time = last_event->time;
      } else {
        // Update time range
        if (first_event->time < start_time) start_time = first_event->time;
        if (last_event->time < end_time) end_time = last_event->time;
      }
    }
  }
  // Zoom in as needed
  if (percent_visible < 1.0) {
    uint64_t elapsed_nanoseconds = end_time - start_time;
    uint64_t visible_nanoseconds = (uint64_t)(elapsed_nanoseconds * percent_visible);
    start_time = (uint64_t)(elapsed_nanoseconds * percent_offset);
    end_time = start_time + visible_nanoseconds;
  }
  time_range = (double)(end_time - start_time);

  // Update units header
  emit visibleTimeRangeChanged(start_time, end_time);

  // Make sure the frame buffer is the correct size
  if (frame_buffer.width() != w || frame_buffer.height() != h) {
    frame_buffer = QImage(w, h, QImage::Format_ARGB32);
    rebuild_frame_buffer = true;
  }

  // Draw event tree
  if (rebuild_frame_buffer) {
    //*+*/printf("Rebuild\n");
    rebuild_frame_buffer = false;
    QPainter painter2(&frame_buffer);
    painter2.fillRect(QRect(0,0,w,h), QColor(255,255,255));
    // Draw event tree
    int line_index = 0;
    QMapIterator<QString, EventTree*> i(G_event_tree_map);
    while (i.hasNext()) {
      // Get old tree info
      i.next();
      EventTree *event_tree = i.value();
      drawHierarchyLine(&painter2, event_tree->events, event_tree->tree, line_index, true);
    }
  }
  painter.drawImage(QRect(0,0,w,h), frame_buffer);

  // Draw selection area
  if (selected_time_range_x1 != -1 && selected_time_range_x2 != -1) {
    int x1 = std::min(selected_time_range_x1, selected_time_range_x2);
    int x2 = std::max(selected_time_range_x1, selected_time_range_x2);
    x1 = std::max(x1, 0);
    x2 = std::min(x2, w-1);
    QColor time_selection_color = TIME_SELECTION_COLOR;
    painter.setPen(QPen(time_selection_color, 1, Qt::SolidLine));
    painter.drawLine(x1, 0, x1, h-1);
    painter.drawLine(x2, 0, x2, h-1);
    time_selection_color.setAlpha(20);
    painter.fillRect(QRect(x1,0,x2-x1+1,h), time_selection_color);
    double time_range_factor = (x2 - x1)/(double)w;
    uint64_t selected_time_range = (uint64_t)(time_range_factor * time_range);
    emit selectionTimeRangeChanged(selected_time_range);
  } else {
    emit selectionTimeRangeChanged(0);
  }

  // Draw rollover info
  if (mouse_location.x() >= 0 && mouse_location.y() >= 0) {
    EventTreeNode *node_with_mouse = NULL;
    Events *events_with_mouse = NULL;
    QMapIterator<QString, EventTree*> i(G_event_tree_map);
    while (i.hasNext()) {
      // Get old tree info
      i.next();
      EventTree *event_tree = i.value();
      node_with_mouse = mouseOnEventsLine(event_tree->tree);
      if (node_with_mouse != NULL) {
        events_with_mouse = event_tree->events;
        break;
      }
    }
    if (node_with_mouse != NULL) {
      //*+ check if any of the parent hierarchy is closed. if (HierarchyClose(parent))  If so, tell user that folder must be open to see events */
      double time_range_factor = mouse_location.x()/(double)w;
      uint64_t time_at_mouse = start_time + (uint64_t)(time_range_factor * time_range);
      uint32_t closest_event_index_at_mouse = 0;
      if (node_with_mouse->tree_node_type == TREE_NODE_IS_EVENT) {
        findEventIndexAtTime(events_with_mouse, node_with_mouse, time_at_mouse, 0);
      }
      QFontMetrics fm = painter.fontMetrics();
      int th = fm.height();
      int dialog_w = fm.horizontalAdvance("This is just a test");
      int dialog_x = mouse_location.x() + 10;
      int dialog_y = mouse_location.y() + 10;
      int dialog_h = th*2;
      if (dialog_x+dialog_w > w) dialog_x -= dialog_w + 20;
      if (dialog_y+dialog_h > h) dialog_y -= dialog_h + 20;
      QString line1_text = " " + node_with_mouse->name;
      QString line2_text = " Event index: " + QString::number(closest_event_index_at_mouse);
      /*+
        Node icon & name: parent->tree_node_type == TREE_NODE_IS_EVENT
        (on range)
          Duration
          Start/End Instance
          Start/End Value
          Start/End Location
        (on gap)   
          Duration
          Prev/Next Instance
          Prev/Next Value
          Prev/Next Location
      */
      painter.setPen(QPen(ROLLOVER_TEXT_COLOR, 1, Qt::SolidLine));
      painter.setBrush(ROLLOVER_BG_COLOR);
      painter.drawRect(dialog_x, dialog_y, dialog_w, dialog_h);
      painter.drawText(dialog_x, dialog_y+th*0, dialog_w, th, Qt::AlignLeft | Qt::AlignVCenter, line1_text);
      painter.drawText(dialog_x, dialog_y+th*1, dialog_w, th, Qt::AlignLeft | Qt::AlignVCenter, line2_text);
    }
  }
}

void EventsView::rebuildAndUpdate() {
  rebuild_frame_buffer = true;
  update();
}
