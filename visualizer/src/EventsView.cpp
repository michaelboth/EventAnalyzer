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

void EventsView::drawHierarchyLine(QPainter *painter, EventTreeNode *parent, int &line_index, int level) {
  int h = height();
  int w = width();
  //*+*/int x = 0; /*+ adjust by time offset */
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
      node_with_mouse = parent;
      row_with_mouse_rect = row_rect;
      painter->fillRect(row_rect, ROW_HIGHLIGHT_COLOR);
    }

    // Highlight row if selected
    if (parent->row_selected) {
      painter->fillRect(row_rect, ROW_SELECTED_COLOR);
    }

    // Draw events
    /*+
      painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));
    */

    /*+ draw to profiling view here? */
  }

  // Recurse
  line_index++;
  if (parent->is_open) {
    for (auto child: parent->children) {
      drawHierarchyLine(painter, child, line_index, level+1);
    }
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
  uint64_t start_time = 0;
  uint64_t end_time = 0;
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
    drawHierarchyLine(&painter, event_tree->tree, line_index, 0);
  }
}
