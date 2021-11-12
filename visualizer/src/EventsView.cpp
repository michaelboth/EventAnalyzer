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

#define LINE_SEPARATOR_COLOR QColor(230, 230, 230)

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
  update();
}

void EventsView::mousePressEvent(QMouseEvent * /*event*/) {
  /*+
  if (event->button() == Qt::LeftButton) {
    // Prepare for mouse motion
    prev_mouse_location = QPoint(event->x(), event->y());
  }
  update();
  */
}

void EventsView::mouseMoveEvent(QMouseEvent * /*event*/) {
  /*+
  // Determine the change in motion
  int dx = event->x() - prev_mouse_location.x();
  int dy = event->y() - prev_mouse_location.y();
  prev_mouse_location = QPoint(event->x(), event->y());
  */
}

void EventsView::mouseReleaseEvent(QMouseEvent * /*event*/) {
  // Nothing to do
}

void EventsView::leaveEvent(QEvent * /*event*/) {
  mouse_location = QPoint(-1,-1);
  update();
}

void EventsView::drawHierarchyLine(QPainter *painter, Events *events, EventTreeNode *parent, int &line_index, int ancestor_open) {
  int h = height();
  int w = width();
  int y = -v_offset + line_index * line_h;
  if (y > h) return;

  // Reset some visual info
  //*+*/parent->row_rect = QRect();
  //*+*/content_bottom_y = y + line_h;

  // only draw if visible
  if (y > -line_h) {
    // Remember the row geometry
    QRect row_rect = QRect(0,y,w,line_h);
    //*+*/parent->row_rect = row_rect;

    // If mouse is on row, then highlight it
    if (row_rect.contains(mouse_location)) {
      /*+ if these are only use to display rollover, then don't need class variables */
      node_with_mouse = parent;
      row_with_mouse_rect = row_rect;
      painter->fillRect(row_rect, ROW_HIGHLIGHT_COLOR);
    }

    // Highlight row if selected (selected in hierarchy view)
    if (parent->row_selected) {
      painter->fillRect(row_rect, ROW_SELECTED_COLOR);
    }

    // Draw events
    /*+ binary search for first visible event */
    /*+ binary search for last visible event */
    int prev_x = 0;
    bool prev_is_start = false;
    //*+*/uint64_t prev_start_time = 0; /*+ determine based on prev event */
    //*+*/uint64_t prev_prev_start_time = 0; /*+ determine based on prev prev event */
    /*+ pre calculate color and store in tree */
    EventInfo *event_info = &events->event_info_list[parent->event_info_index];
    int red   = (int)(255 * (((event_info->rgb & 0xf00) >> 8) / (float)0xf));
    int green = (int)(255 * (((event_info->rgb & 0xf0) >> 4) / (float)0xf));
    int blue  = (int)(255 * ((event_info->rgb & 0xf) / (float)0xf));
    QColor color = QColor(red, green, blue);
    painter->setPen(QPen(color, 1, Qt::SolidLine));
    int y2 = y + (int)(line_h * 0.2f);
    int y3 = y + (int)(line_h * 0.4f);
    int y4 = y + (int)(line_h * 0.6f);
    int y5 = y + (int)(line_h * 0.8f);
    int range_h = y4 - y3 + 1;
    int x_offset = 0; /*+ adjust by time offset */
    double time_range = (double)(end_time - start_time);/*+ pre calculate as a class var */
    for (uint32_t i=0; i<parent->num_event_instances; i++) {
      uint32_t event_index = parent->event_indices[i];
      Event *event = &events->event_buffer[event_index];
      bool is_start = event->event_id == event_info->start_id;
      // X location in visible region
      double x_percent = (event->time - start_time) / time_range;
      int x = x_offset + (int)(x_percent * (w-1));
      int top_y = is_start ? y2 : y3;
      int bottom_y = is_start ? y4 : y5;
      painter->drawLine(x, top_y, x, bottom_y);
      // Draw range
      if (!is_start && prev_is_start && x > prev_x) {
        int range_w = x - prev_x;
        painter->fillRect(QRect(prev_x,y3,range_w,range_h), color);
      }
      // Remember some stuff
      prev_is_start = is_start;
      prev_x = x;
    }
    /*+ record usage to tree, to later display in profiling view, and emit signal when done drawing */

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

  // Update some high level geometry
  //*+*/content_bottom_y = 0;
  row_with_mouse_rect = QRect();
  node_with_mouse = NULL;

  if (G_event_tree_map.count() == 0) {
    // Draw the logo
    QRect inside_rect = getFittedRect(FitType::Inside, w/2, h, logo.width(), logo.height());
    inside_rect.moveLeft(w/4);
    painter.setRenderHint(QPainter::SmoothPixmapTransform,true);
    painter.drawPixmap(inside_rect, logo);
    painter.setRenderHint(QPainter::SmoothPixmapTransform,false);
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

  // Draw event tree
  int line_index = 0;
  QMapIterator<QString, EventTree*> i(G_event_tree_map);
  while (i.hasNext()) {
    // Get old tree info
    i.next();
    EventTree *event_tree = i.value();
    drawHierarchyLine(&painter, event_tree->events, event_tree->tree, line_index, true);
  }

  /*+ draw rollover info */
}
