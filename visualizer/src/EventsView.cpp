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
#include <QAction>
#include <QMenu>
#include "EventsView.hpp"
#include "HelpfulFunctions.hpp"
#include "main.hpp"

#define MIN_SELECTION_PIXEL_DIFF 5
#define LINE_SEPARATOR_COLOR QColor(230, 230, 230)
#define ROLLOVER_BG_COLOR QColor(200, 200, 200, 220)
#define ROLLOVER_TEXT_COLOR QColor(50, 50, 50)
#define ROLLOVER_TITLE_COLOR QColor(125, 125, 125)
#define ROLLOVER_SEPARATOR_COLOR QColor(175, 175, 175)
#define ROW_HIGHLIGHT_COLOR2 QColor(0, 0, 0, 15)
#define HISTOGRAM_BAR_BG_COLOR QColor(230,230,230)
#define ALIGNMENT_COLOR QColor(150, 150, 150)

EventsView::EventsView(QWidget *parent) : QWidget(parent) {
  // Track mouse when not pressed
  setMouseTracking(true);

  // Allow keyboard input
  //setFocusPolicy(Qt::StrongFocus);
  // Allow for keyboard focus now
  //setFocus();

  // Allow mouse right click
  this->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(popupContextMenu(const QPoint &)));

  // Load the logo
  QImage image = QImage(":/unikorn_logo.png");
  recolorImage(image, QColor(220,220,220));
  logo = QPixmap::fromImage(image);
}

EventsView::~EventsView() {
  // Nothing to do
}

static uint32_t findEventIndexAtTime(Events *events, EventTreeNode *node, uint64_t time, int32_t index_offset) {
  uint32_t first = 0;
  uint32_t last = node->num_event_instances-1;

  // See if before first event
  uint32_t first_event_index = node->event_indices[first];
  Event *first_event = &events->event_buffer[first_event_index];
  if (time <= first_event->time) return first;

  // See if after last event
  uint32_t last_event_index = node->event_indices[last];
  Event *last_event = &events->event_buffer[last_event_index];
  if (time > last_event->time) return last+1;

  // Binary search
  while ((last-first) > 1) {
    uint32_t mid = first + (last-first)/2;
    uint32_t event_index = node->event_indices[mid];
    Event *event = &events->event_buffer[event_index];
    if (event->time <= time) {
      first = mid;
    } else {
      last = mid;
    }
  }
  first++; // NOTE: want the first event to the right of the mouse, not the left
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

void EventsView::popupContextMenu(const QPoint &mouse_location) {
  if (G_event_tree_map.count() == 0) return;

  QMenu menu("Right click menu", this);
  QString style_text =
    "QMenu {\n"
    "  icon-size: 32px;\n"
    "  background-color: #bbbbbb;\n"
    "  border: 1px solid black;\n"
    "}\n"
    "QMenu::item {\n"
    "  background-color: transparent;\n"
    "}\n"
    "QMenu::item:selected {\n"
    "  background-color: #999999;\n"
    "}\n";
  menu.setStyleSheet(style_text);

  QAction zoom_to_region_action(QIcon(":/zoom_to_selected.png"), "Zoom To Region", this);
  QAction zoom_to_all_action(QIcon(":/zoom_to_all.png"), "Zoom To All", this);
  QAction zoom_in_action(QIcon(":/zoom_in.png"), "Zoom In", this);
  QAction zoom_out_action(QIcon(":/zoom_out.png"), "Zoom Out", this);

  if (timeRangeSelected()) {
    connect(&zoom_to_region_action, SIGNAL(triggered()), this, SLOT(zoomToRegion()));
    menu.addAction(&zoom_to_region_action);
  }

  if (percent_visible < 1.0) {
    connect(&zoom_to_all_action, SIGNAL(triggered()), this, SLOT(zoomToAll()));
    menu.addAction(&zoom_to_all_action);
  }

  connect(&zoom_in_action, SIGNAL(triggered()), this, SLOT(zoomIn()));
  menu.addAction(&zoom_in_action);

  if (percent_visible < 1.0) {
    connect(&zoom_out_action, SIGNAL(triggered()), this, SLOT(zoomOut()));
    menu.addAction(&zoom_out_action);
  }

  // Display the menu
  menu.exec(mapToGlobal(mouse_location));
}

void EventsView::setMouseMode(MouseMode _mouse_mode) {
  mouse_mode = _mouse_mode;
}

void EventsView::prepareIcon(QString filename, bool recolor, QColor color) {
  QImage image = QImage(":/"+filename);
  if (recolor) {
    recolorImage(image, color);
  }
  image = image.scaledToHeight((int)(line_h*G_pixels_per_point), Qt::SmoothTransformation);
  QPixmap pixmap = QPixmap::fromImage(image);
  //pixmap.setDevicePixelRatio(G_pixels_per_point); // Does not seems to be needed probably because main.cpp calls QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
  icon_map[filename] = pixmap;
}

void EventsView::updateLineHeight() {
  // Update font size
  QFont font = this->font();
  font.setPointSize(G_font_point_size);
  this->setFont(font);

  // Calculate geometry
  QFontMetrics fm = QFontMetrics(this->font());
  line_h = fm.height();

  // Rebuild icons to make sure there are correctly sized; if not, then edges will looked aliased
  prepareIcon("hierarchy_closed_folder.png", true, FOLDER_ICON_COLOR);
  prepareIcon("hierarchy_closed_thread.png", true, THREAD_ICON_COLOR);
  prepareIcon("hierarchy_file.png", false, Qt::black);
  prepareIcon("hierarchy_opened_folder.png", true, FOLDER_ICON_COLOR);
  prepareIcon("hierarchy_opened_thread.png", true, THREAD_ICON_COLOR);
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

void EventsView::centerPrevEvent(Events *events, EventTreeNode *events_row) {
  // Check if zoomed in, otherwise all events are visible
  if (percent_visible < 1.0) {
    // IMPORTANT: Using time range that was last calculated from the paintEvent()
    // Get event currently at center
    uint64_t center_time = start_time + (end_time - start_time)/2;
    uint32_t center_event_index = findEventIndexAtTime(events, events_row, center_time, 0);
    if (center_event_index >= events_row->num_event_instances) center_event_index = events_row->num_event_instances-1;
    Event *event = &events->event_buffer[events_row->event_indices[center_event_index]];

    // See if need to skip to the prev event
    uint64_t margin = (uint64_t)(0.01*(end_time-start_time)); // 1% of visible time
    if (event->time >= (center_time-margin)) {
      if (center_event_index > 0) {
        center_event_index--;
        event = &events->event_buffer[events_row->event_indices[center_event_index]];
      }
    }

    if (event->time < center_time) {
      // Move the percent_offset to center this event
      // From:
      //   [-------|+++X+++++++++|-------------------]
      // To:
      //   [----|++++++X++++++|----------------------]
      double center_percent = (event->time - full_start_time) / (double)(full_end_time - full_start_time);
      percent_offset = std::max(center_percent - percent_visible/2.0, 0.0);
      selected_time_range_x1 = -1;
      selected_time_range_x2 = -1;
      emit timeRangeChanged();
      rebuildAndUpdate();
    }
  }
}

void EventsView::centerNextEvent(Events *events, EventTreeNode *events_row) {
  // Check if zoomed in, otherwise all events are visible
  if (percent_visible < 1.0) {
    // IMPORTANT: Using time range that was last calculated from the paintEvent()
    // Get event currently at center
    uint64_t center_time = start_time + (end_time - start_time)/2;
    uint32_t center_event_index = findEventIndexAtTime(events, events_row, center_time, 0);
    if (center_event_index >= events_row->num_event_instances) center_event_index = events_row->num_event_instances-1;
    Event *event = &events->event_buffer[events_row->event_indices[center_event_index]];

    // See if need to skip to the next event
    uint64_t margin = (uint64_t)(0.01*(end_time-start_time)); // 1% of visible time
    if (event->time <= (center_time+margin)) {
      if (center_event_index < (events_row->num_event_instances-1)) {
        center_event_index++;
        event = &events->event_buffer[events_row->event_indices[center_event_index]];
      }
    }

    if (event->time > center_time) {
      // Move the percent_offset to center this event
      // From:
      //   [-------|+++++++++X+++|-------------------]
      // To:
      //   [----------|++++++X++++++|----------------]
      double center_percent = (event->time - full_start_time) / (double)(full_end_time - full_start_time);
      percent_offset = std::max(center_percent - percent_visible/2.0, 0.0);
      selected_time_range_x1 = -1;
      selected_time_range_x2 = -1;
      emit timeRangeChanged();
      rebuildAndUpdate();
    }
  }
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
  mouse_location = QPoint(event->x(), event->y());
  if (mouse_button_pressed) {
    // Selecting time range
    selected_time_range_x2 = event->x();
    update(); // Avoids full redraw
  } else {
    // See if mouse is on an event line
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

void EventsView::hasEventsOutsideOfVisibleRegion(Events *events, EventTreeNode *events_row, bool *events_to_the_left_ret, bool *events_to_the_right_ret) {
  *events_to_the_left_ret = false;
  *events_to_the_right_ret = false;
  // Check if zoomed in, otherwise all events are visible
  if (percent_visible < 1.0) {
    // IMPORTANT: Using time range that was last calculated from the paintEvent()
    uint32_t first_visible_event_index = findEventIndexAtTime(events, events_row, start_time, 0);
    uint32_t last_visible_event_index = findEventIndexAtTime(events, events_row, end_time, 0);
    if (first_visible_event_index > 0) {
      *events_to_the_left_ret = true;
    }
    if (last_visible_event_index < events_row->num_event_instances) {
      *events_to_the_right_ret = true;
    }
  }
}

void EventsView::drawHierarchyLine(QPainter *painter, Events *events, EventTreeNode *parent, int &line_index, int ancestor_open) {
  int h = height();
  int w = width();
  int y = -v_offset + line_index * line_h;
  parent->events_row_rect = QRect();
  parent->utilization = 0.0;
  bool parent_is_filtered = (parent->tree_node_type == TREE_NODE_IS_EVENT && G_event_filters.contains(parent->name));

  // only draw if visible
  if (!parent_is_filtered && y > -line_h && y < h) {
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
      if (last_visible_event_index >= parent->num_event_instances) last_visible_event_index = parent->num_event_instances - 1; // NOTE: just outside of range

      // Draw events
      int prev_x = 0;
      int prev_prev_x = 0;
      uint64_t prev_time = 0;
      bool prev_is_start = false;
      EventInfo *event_info = &events->event_info_list[parent->event_info_index];
      painter->setPen(QPen(parent->color, 1, Qt::SolidLine));
      int y2 = y + (int)(line_h * 0.2f);
      int y3 = y + (int)(line_h * 0.4f);
      int y4 = y + (int)(line_h * 0.6f);
      int y5 = y + (int)(line_h * 0.8f);
      int range_h = y4 - y3 + 1;
      uint64_t total_time_usage = 0;
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
        if (x == prev_prev_x) {
          // Overlapped with other events, so draw tall event
          painter->drawLine(x, y+1, x, y+line_h-2);
        } else {
          int top_y = is_start ? y2 : y3;
          int bottom_y = is_start ? y4 : y5;
          painter->drawLine(x, top_y, x, bottom_y);
        }
        // Draw duration between start and end
        if (!is_start && prev_is_start) {
          int range_w = x - prev_x;
          painter->fillRect(QRect(prev_x,y3,range_w,range_h), parent->color);
	  // Accumulate time usage, to be displayed in the utilization column
	  {
	    uint64_t t1 = std::min(std::max(prev_time, start_time), end_time);
	    uint64_t t2 = std::min(std::max(event->time, start_time), end_time);
	    total_time_usage += t2-t1;
	  }
        }
        // Remember some stuff
        prev_time = event->time;
        prev_is_start = is_start;
        prev_prev_x = prev_x;
        prev_x = x;
      }
      parent->utilization = total_time_usage / (double)(end_time - start_time);
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
    bool child_is_filtered = (child->tree_node_type == TREE_NODE_IS_EVENT && G_event_filters.contains(child->name));
    if (ancestor_open && parent->is_open && !child_is_filtered) line_index++;
    drawHierarchyLine(painter, events, child, line_index, ancestor_open ? parent->is_open : false);
  }
}

uint32_t EventsView::calculateHistogram(int num_buckets, uint32_t *buckets, EventTreeNode *node, Events *events, uint64_t *min_ret, uint64_t *ave_ret, uint64_t *max_ret) {
  EventInfo *event_info = &events->event_info_list[node->event_info_index];

  // Get the first visible event
  uint32_t first_event_index = findEventIndexAtTime(events, node, start_time, 0); // NOTE: gets the first index to the right of the start time

  // Determine the min and max durations to know how to divy out the buckets
  Event *prev_event = NULL;
  uint64_t min_duration = 0;
  uint64_t max_duration = 0;
  uint64_t total_duration = 0;
  uint32_t num_durations = 0;
  uint32_t event_index = first_event_index;
  while (event_index < node->num_event_instances) {
    Event *event = &events->event_buffer[node->event_indices[event_index]];
    if (event->time > end_time) break; // Out of visible range
    if (prev_event != NULL && prev_event->event_id == event_info->start_id && event->event_id == event_info->end_id) {
      // Found a duration
      uint64_t duration = event->time - prev_event->time;
      total_duration += duration;
      if (num_durations == 0 || duration < min_duration) min_duration = duration;
      if (num_durations == 0 || duration > max_duration) max_duration = duration;
      num_durations++;
    }
    prev_event = event;
    event_index++;
  }
  if (num_durations == 0) return 0;

  // Fill in the buckets
  uint64_t duration_range = max_duration - min_duration;
  event_index = first_event_index;
  while (event_index < node->num_event_instances) {
    Event *event = &events->event_buffer[node->event_indices[event_index]];
    if (event->time > end_time) break; // Out of visible range
    if (prev_event != NULL && prev_event->event_id == event_info->start_id && event->event_id == event_info->end_id) {
      // Found a duration
      uint64_t duration = event->time - prev_event->time;
      double factor = (duration - min_duration) / (double)duration_range;
      int bucket_index = factor * (num_buckets-1);
      bucket_index = std::clamp(bucket_index, 0, num_buckets-1);
      buckets[bucket_index]++;
    }
    prev_event = event;
    event_index++;
  }

  // Return values
  *min_ret = min_duration;
  *ave_ret = total_duration / num_durations;
  *max_ret = max_duration;
  return num_durations;
}

void EventsView::drawEventHistogram(QPainter &painter, EventTreeNode *node, Events *events) {
  int w = width();
  int h = height();

  bool ancestor_collapsed = node->isAncestorCollapsed();

  // Determine dialog geometry
  QFontMetrics fm = painter.fontMetrics();
  int th = fm.height();
  int m = th / 2;
  int num_lines = 14;
  int dialog_w = fm.horizontalAdvance("9999.999 usecs      9999.999 usecs      9999.999 usecs");
  int dialog_h = th*num_lines;
  int dialog_x = mouse_location.x() + m;
  int dialog_y = mouse_location.y() + m;
  if (dialog_x+dialog_w > w) dialog_x -= dialog_w + 2*m;
  if (dialog_y+dialog_h > h) dialog_y -= dialog_h + 2*m;

  // Dialog background
  painter.setPen(QPen(ROLLOVER_TEXT_COLOR, 1, Qt::SolidLine));
  painter.setBrush(ROLLOVER_BG_COLOR);
  painter.drawRect(dialog_x, dialog_y, dialog_w, dialog_h);
  painter.setPen(QPen(ROLLOVER_SEPARATOR_COLOR, 1, Qt::SolidLine));
  // Horizontal lines
  painter.drawLine(dialog_x+1, dialog_y+th, dialog_x+dialog_w-1, dialog_y+th);
  painter.drawLine(dialog_x+1, dialog_y+th*(num_lines-2), dialog_x+dialog_w-1, dialog_y+th*(num_lines-2));
  painter.drawLine(dialog_x+1, dialog_y+th*(num_lines-1), dialog_x+dialog_w-1, dialog_y+th*(num_lines-1));
  painter.setPen(QPen(ROLLOVER_TEXT_COLOR, 1, Qt::SolidLine));

  // Draw icon
  QPixmap image_icon;
  if (node->tree_node_type == TREE_NODE_IS_FILE) {
    image_icon = icon_map["hierarchy_file.png"];
  } else if (node->tree_node_type == TREE_NODE_IS_FOLDER) {
    image_icon = node->is_open ? icon_map["hierarchy_opened_folder.png"] : icon_map["hierarchy_closed_folder.png"];
  } else if (node->tree_node_type == TREE_NODE_IS_THREAD) {
    image_icon = node->is_open ? icon_map["hierarchy_opened_thread.png"] : icon_map["hierarchy_closed_thread.png"];
  } else {
    image_icon = drawEventIcon(th, node->color);
  }
  int icon_offset = 0;
  if (!image_icon.isNull()) {
    painter.setRenderHint(QPainter::SmoothPixmapTransform,true);
    icon_offset = m+image_icon.width();
    int image_w = th * image_icon.width() / (float)image_icon.height();
    painter.drawPixmap(dialog_x+m, dialog_y, image_w, th, image_icon);
    painter.setRenderHint(QPainter::SmoothPixmapTransform,false);
  }

  // Draw name
  QString name_text = " " + node->name;
  painter.drawText(dialog_x+icon_offset, dialog_y, dialog_w-icon_offset, th, Qt::AlignLeft | Qt::AlignVCenter, name_text);

  if (ancestor_collapsed) {
    // Nothing to show
    QString message = "Open this item in left\npanel to see event histograms";
    painter.drawText(dialog_x, dialog_y+th, dialog_w, th*(num_lines-2), Qt::AlignCenter, message);
  }
  if (node->tree_node_type != TREE_NODE_IS_EVENT) {
    return;
  }

  // Calculate histogram
  int bucket_w = th / 2;
  if (bucket_w <= 1) bucket_w = 2;
  int num_buckets = (dialog_w-2) / bucket_w;
  uint32_t *buckets = (uint32_t *)malloc(num_buckets*sizeof(uint32_t));
  for (int i=0; i<num_buckets; i++) buckets[i] = 0;
  uint64_t min, ave, max;
  uint32_t num_durations = calculateHistogram(num_buckets, buckets, node, events, &min, &ave, &max);

  // Draw number of durations
  QString suffix = (num_durations == 1) ? " duration " : " durations ";
  painter.drawText(dialog_x, dialog_y, dialog_w, th, Qt::AlignRight | Qt::AlignVCenter, QString::number(num_durations) + suffix);

  // Draw histogram
  int hist_x = dialog_x + (dialog_w - bucket_w*num_buckets)/2;
  int hist_y = dialog_y + dialog_h - th*2 - 1;
  int hist_h = dialog_h - th*3 - 2;
  for (int i=0; i<num_buckets; i++) {
    int bar_x = hist_x + i*bucket_w;
    painter.fillRect(QRect(bar_x, hist_y-hist_h, bucket_w-1, hist_h), HISTOGRAM_BAR_BG_COLOR);
    if (buckets[i] > 0) {
      int bar_h = hist_h * (buckets[i] / (float)num_durations);
      if (bar_h == 0) bar_h = 1;
      int bar_y = hist_y - bar_h;
      painter.fillRect(QRect(bar_x, bar_y, bucket_w-1, bar_h), node->color);
    }
  }

  // Draw min ave and max
  { // Min
    uint64_t units_factor;
    QString time_units = getTimeUnitsAndFactor(min, 1, &units_factor);
    double adjusted_time = min / (double)units_factor;
    char val_text[40];
    sprintf(val_text, "%0.3f", adjusted_time);
    QString text = " " + QString(val_text) + " " + time_units;
    painter.drawText(dialog_x, dialog_y+th*(num_lines-2), dialog_w, th, Qt::AlignLeft | Qt::AlignVCenter, text);
    painter.drawText(dialog_x, dialog_y+th*(num_lines-1), dialog_w, th, Qt::AlignLeft | Qt::AlignVCenter, " min");
  }
  { // Ave
    uint64_t units_factor;
    QString time_units = getTimeUnitsAndFactor(ave, 1, &units_factor);
    double adjusted_time = ave / (double)units_factor;
    char val_text[40];
    sprintf(val_text, "%0.3f", adjusted_time);
    QString text = QString(val_text) + " " + time_units;
    painter.drawText(dialog_x, dialog_y+th*(num_lines-2), dialog_w, th, Qt::AlignCenter, text);
    painter.drawText(dialog_x, dialog_y+th*(num_lines-1), dialog_w, th, Qt::AlignCenter, "average");
  }
  { // Max
    uint64_t units_factor;
    QString time_units = getTimeUnitsAndFactor(max, 1, &units_factor);
    double adjusted_time = max / (double)units_factor;
    char val_text[40];
    sprintf(val_text, "%0.3f", adjusted_time);
    QString text = QString(val_text) + " " + time_units + " ";
    painter.drawText(dialog_x, dialog_y+th*(num_lines-2), dialog_w, th, Qt::AlignRight | Qt::AlignVCenter, text);
    painter.drawText(dialog_x, dialog_y+th*(num_lines-1), dialog_w, th, Qt::AlignRight | Qt::AlignVCenter, "max ");
  }

  free(buckets);
}

void EventsView::drawEventInfo(QPainter &painter, EventTreeNode *node, Events *events) {
  int w = width();
  int h = height();

  bool ancestor_collapsed = node->isAncestorCollapsed();
  double time_range_factor = mouse_location.x()/(double)w;
  uint64_t time_at_mouse = start_time + (uint64_t)(time_range_factor * time_range);

  // Determine dialog geometry
  QFontMetrics fm = painter.fontMetrics();
  int th = fm.height();
  int m = th / 2;
  int num_lines = 8;
  int instance_w = fm.horizontalAdvance("  Instance  ");
  int dialog_w = fm.horizontalAdvance(" xxxfilename::function_name::line_numberxxx ");
  int dialog_h = th*num_lines;
  int dialog_x = mouse_location.x() + m;
  int dialog_y = mouse_location.y() + m;
  if (dialog_x+dialog_w > w) dialog_x -= dialog_w + 2*m;
  if (dialog_y+dialog_h > h) dialog_y -= dialog_h + 2*m;

  // Dialog background
  painter.setPen(QPen(ROLLOVER_TEXT_COLOR, 1, Qt::SolidLine));
  painter.setBrush(ROLLOVER_BG_COLOR);
  painter.drawRect(dialog_x, dialog_y, dialog_w, dialog_h);
  painter.setPen(QPen(ROLLOVER_SEPARATOR_COLOR, 1, Qt::SolidLine));
  // Horizontal lines
  int num_lines_to_draw = ancestor_collapsed ? 3 : num_lines;
  for (int i=1; i<num_lines_to_draw; i++) {
    int line_y = i*th;
    painter.drawLine(dialog_x+1, dialog_y+line_y, dialog_x+dialog_w-1, dialog_y+line_y);
  }
  int mid_x = dialog_w/2;
  int col1_x = (dialog_w - instance_w)/2;
  int col2_x = col1_x + instance_w;
  // Veritical lines
  if (!ancestor_collapsed) {
    painter.drawLine(dialog_x+mid_x, dialog_y+th*1, dialog_x+mid_x, dialog_y+th*3);
    painter.drawLine(dialog_x+col1_x, dialog_y+th*3, dialog_x+col1_x, dialog_y+th*6);
    painter.drawLine(dialog_x+col2_x, dialog_y+th*3, dialog_x+col2_x, dialog_y+th*6);
  }
  // Titles
  painter.setPen(QPen(ROLLOVER_TITLE_COLOR, 1, Qt::SolidLine));
  painter.drawText(dialog_x, dialog_y+th*1, mid_x, th, Qt::AlignRight | Qt::AlignVCenter, "Mouse Over Time ");
  if (!ancestor_collapsed) {
    painter.drawText(dialog_x, dialog_y+th*2, mid_x, th, Qt::AlignRight | Qt::AlignVCenter, "Duration/Gap ");
    painter.drawText(dialog_x+col1_x, dialog_y+th*3, col2_x-col1_x, th, Qt::AlignCenter, "Time");
    painter.drawText(dialog_x+col1_x, dialog_y+th*4, col2_x-col1_x, th, Qt::AlignCenter, "Instance");
    painter.drawText(dialog_x+col1_x, dialog_y+th*5, col2_x-col1_x, th, Qt::AlignCenter, "Value");
  }
  painter.setPen(QPen(ROLLOVER_TEXT_COLOR, 1, Qt::SolidLine));

  // Draw icon
  QPixmap image_icon;
  if (node->tree_node_type == TREE_NODE_IS_FILE) {
    image_icon = icon_map["hierarchy_file.png"];
  } else if (node->tree_node_type == TREE_NODE_IS_FOLDER) {
    image_icon = node->is_open ? icon_map["hierarchy_opened_folder.png"] : icon_map["hierarchy_closed_folder.png"];
  } else if (node->tree_node_type == TREE_NODE_IS_THREAD) {
    image_icon = node->is_open ? icon_map["hierarchy_opened_thread.png"] : icon_map["hierarchy_closed_thread.png"];
  } else {
    image_icon = drawEventIcon(th, node->color);
  }
  int icon_offset = 0;
  if (!image_icon.isNull()) {
    painter.setRenderHint(QPainter::SmoothPixmapTransform,true);
    icon_offset = m + image_icon.width() / G_pixels_per_point;
    int image_w = th * image_icon.width() / (float)image_icon.height();
    painter.drawPixmap(dialog_x+m, dialog_y, image_w, th, image_icon);
    painter.setRenderHint(QPainter::SmoothPixmapTransform,false);
  }

  // Draw name
  QString name_text = " " + node->name;
  painter.drawText(dialog_x+icon_offset, dialog_y, dialog_w-icon_offset, th, Qt::AlignLeft | Qt::AlignVCenter, name_text);
  dialog_y += th;

  // Draw mouse time
  uint64_t mouse_time_units_factor;
  QString mouse_time_units = getTimeUnitsAndFactor(time_at_mouse, 1, &mouse_time_units_factor);
  double adjusted_mouse_time = time_at_mouse / (double)mouse_time_units_factor;
  char val_text[40];
  sprintf(val_text, "%0.3f", adjusted_mouse_time);
  QString mouse_time_text = " " + QString(val_text) + " " + mouse_time_units;
  painter.drawText(dialog_x+mid_x, dialog_y, dialog_w-mid_x, th, Qt::AlignLeft | Qt::AlignVCenter, mouse_time_text);
  dialog_y += th;

  if (ancestor_collapsed) {
    // Nothing to show
    QString message = "Open this item in left\npanel to see event details";
    painter.drawText(dialog_x, dialog_y, dialog_w, th*(num_lines-3), Qt::AlignCenter, message);
  } else if (node->tree_node_type == TREE_NODE_IS_EVENT) {
    // Show event info
    uint32_t event_index_to_right_of_mouse = findEventIndexAtTime(events, node, time_at_mouse, 0);
    bool has_next_event = event_index_to_right_of_mouse < node->num_event_instances;
    uint32_t event_index_to_left_of_mouse = (event_index_to_right_of_mouse > 0) ? event_index_to_right_of_mouse-1 : 0;
    bool has_prev_event = event_index_to_left_of_mouse < event_index_to_right_of_mouse;
    Event *prev_event = has_prev_event ? &events->event_buffer[node->event_indices[event_index_to_left_of_mouse]] : NULL;
    Event *next_event = has_next_event ? &events->event_buffer[node->event_indices[event_index_to_right_of_mouse]] : NULL;

    // Duration
    if (has_prev_event && has_next_event) {
      uint64_t duration = next_event->time - prev_event->time;
      uint64_t units_factor;
      QString time_units = getTimeUnitsAndFactor(duration, 1, &units_factor);
      double adjusted_duration = duration / (double)units_factor;
      sprintf(val_text, "%0.3f", adjusted_duration);
      QString text = " " + QString(val_text) + " " + time_units;
      painter.drawText(dialog_x+mid_x, dialog_y, dialog_w-mid_x, th, Qt::AlignLeft | Qt::AlignVCenter, text);
    }
    dialog_y += th;

    // Prev/Next Times
    if (has_prev_event) {
      uint64_t units_factor;
      QString time_units = getTimeUnitsAndFactor(prev_event->time, 1, &units_factor);
      double adjusted_time = prev_event->time / (double)units_factor;
      sprintf(val_text, "%0.3f", adjusted_time);
      QString text = QString(val_text) + " " + time_units + " ";
      painter.drawText(dialog_x, dialog_y, col1_x, th, Qt::AlignRight | Qt::AlignVCenter, text);
    }
    if (has_next_event) {
      uint64_t units_factor;
      QString time_units = getTimeUnitsAndFactor(next_event->time, 1, &units_factor);
      double adjusted_time = next_event->time / (double)units_factor;
      sprintf(val_text, "%0.3f", adjusted_time);
      QString text = " " + QString(val_text) + " " + time_units;
      painter.drawText(dialog_x+col2_x, dialog_y, col1_x, th, Qt::AlignLeft | Qt::AlignVCenter, text);
    }
    dialog_y += th;

    // Prev/Next Instances
    if (has_prev_event) {
      QString text = events->includes_instance ? QString::number(prev_event->instance) + " " : "- ";
      painter.drawText(dialog_x, dialog_y, col1_x, th, Qt::AlignRight | Qt::AlignVCenter, text);
    }
    if (has_next_event) {
      QString text = events->includes_instance ? " " + QString::number(next_event->instance) : " -";
      painter.drawText(dialog_x+col2_x, dialog_y, col1_x, th, Qt::AlignLeft | Qt::AlignVCenter, text);
    }
    dialog_y += th;

    // Prev/Next Values
    if (has_prev_event) {
      QString text = events->includes_value ? QString::number(prev_event->value) + " " : "- ";
      painter.drawText(dialog_x, dialog_y, col1_x, th, Qt::AlignRight | Qt::AlignVCenter, text);
    }
    if (has_next_event) {
      QString text = events->includes_value ? " " + QString::number(next_event->value) : " -";
      painter.drawText(dialog_x+col2_x, dialog_y, col1_x, th, Qt::AlignLeft | Qt::AlignVCenter, text);
    }
    dialog_y += th;

    // Prev/Next Locations
    if (has_prev_event) {
      QString text = " -";   /*+ maybe use "Location not recorded" instead */
      if (events->includes_file_location) {
        text = " " + QString(events->file_name_list[prev_event->file_name_index]);
        text += "," + QString(events->function_name_list[prev_event->function_name_index]);
        text += "():" + QString::number(prev_event->line_number);
      }
      painter.drawText(dialog_x, dialog_y, dialog_w-m, th, Qt::AlignRight | Qt::AlignVCenter, text);
    }
    dialog_y += th;
    if (has_next_event) {
      QString text = " -";
      if (events->includes_file_location) {
        text = " " + QString(events->file_name_list[next_event->file_name_index]);
        text += "," + QString(events->function_name_list[next_event->function_name_index]);
        text += "():" + QString::number(next_event->line_number);
      }
      painter.drawText(dialog_x, dialog_y, dialog_w-m, th, Qt::AlignRight | Qt::AlignVCenter, text);
    }
  }
}

void EventsView::updateTimeAlignment() {
  QString alignment_mode = G_settings->value("alignment_mode", "Native").toString();  // One of "Native", "TimeZero", "EventId"

  if (alignment_mode == "Native") {
    QMapIterator<QString, EventTree*> i(G_event_tree_map);
    while (i.hasNext()) {
      i.next();
      EventTree *event_tree = i.value();
      Events *events = event_tree->events;
      uint64_t first_time = events->event_buffer[0].time;
      bool increase_time = (first_time < event_tree->native_start_time);
      uint64_t delta = increase_time ? event_tree->native_start_time - first_time : first_time - event_tree->native_start_time;
      if (delta > 0) {
        if (increase_time) {
          for (uint32_t j=0; j<events->event_count; j++) events->event_buffer[j].time += delta;
        } else {
          for (uint32_t j=0; j<events->event_count; j++) events->event_buffer[j].time -= delta;
        }
      }
    }

  } else if (alignment_mode == "TimeZero") {
    QMapIterator<QString, EventTree*> i(G_event_tree_map);
    while (i.hasNext()) {
      i.next();
      EventTree *event_tree = i.value();
      Events *events = event_tree->events;
      uint64_t first_time = events->event_buffer[0].time;
      if (first_time > 0) {
        for (uint32_t j=0; j<events->event_count; j++) {
          events->event_buffer[j].time -= first_time;
        }
      }
    }

  } else { // "EventId"
    // Move all event files to time zero
    {
      QMapIterator<QString, EventTree*> i(G_event_tree_map);
      while (i.hasNext()) {
        i.next();
        EventTree *event_tree = i.value();
        Events *events = event_tree->events;
        uint64_t first_time = events->event_buffer[0].time;
        if (first_time > 0) {
          for (uint32_t j=0; j<events->event_count; j++) {
            events->event_buffer[j].time -= first_time;
          }
        }
      }
    }

    // Get the alignment values
    bool is_start = G_settings->value("alignment_event_is_start", false).toBool();
    QString event_name = G_settings->value("alignment_event_name", "unset").toString();
    uint32_t instance_index = (uint32_t)G_settings->value("alignment_instance_index", 0).toInt();

    // Find the greatest time across files
    uint64_t max_time = 0;
    {
      QMapIterator<QString, EventTree*> i(G_event_tree_map);
      while (i.hasNext()) {
        i.next();
        EventTree *event_tree = i.value();
        Events *events = event_tree->events;

        // Get the event ID
        uint16_t event_id = 0;
        for (uint16_t j=0; j<events->event_info_count; j++) {
          EventInfo *event_info = &events->event_info_list[j];
          if (QString(event_info->name) == event_name) {
            event_id = is_start ? event_info->start_id : event_info->end_id;
            break;
          }
        }

        // Find the event time with event_id and instance
        for (uint32_t j=0; j<events->event_count; j++) {
          Event *event = &events->event_buffer[j];
          if (event->event_id == event_id && event->instance == instance_index) {
            if (event->time > max_time) max_time = event->time;
            break;
          }
        }
      }
    }

    // Adjust times
    alignment_time = max_time;
    if (max_time > 0) {
      QMapIterator<QString, EventTree*> i(G_event_tree_map);
      while (i.hasNext()) {
        i.next();
        EventTree *event_tree = i.value();
        Events *events = event_tree->events;

        // Get the event ID
        uint16_t event_id = 0;
        for (uint16_t j=0; j<events->event_info_count; j++) {
          EventInfo *event_info = &events->event_info_list[j];
          if (QString(event_info->name) == event_name) {
            event_id = is_start ? event_info->start_id : event_info->end_id;
            break;
          }
        }

        // Find the event time with event_id and instance
        uint64_t max_time2 = 0;
        for (uint32_t j=0; j<events->event_count; j++) {
          Event *event = &events->event_buffer[j];
          if (event->event_id == event_id && event->instance == instance_index) {
            max_time2 = event->time;
            break;
          }
        }

        // Adjust all times
        bool increase_time = (max_time2 < max_time);
        uint64_t delta = increase_time ? max_time - max_time2 : max_time2 - max_time;
        if (delta > 0) {
          if (increase_time) {
            for (uint32_t j=0; j<events->event_count; j++) events->event_buffer[j].time += delta;
          } else {
            for (uint32_t j=0; j<events->event_count; j++) events->event_buffer[j].time -= delta;
          }
        }
      }
    }
  }

  // Update display
  percent_visible = 1.0;
  percent_offset = 0.0;
  selected_time_range_x1 = -1;
  selected_time_range_x2 = -1;
  emit timeRangeChanged();
  rebuildAndUpdate();
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
    emit utilizationRecalculated();     // Signal to utilization column
    return;
  }

  // Determine time range
  full_start_time = 0;
  full_end_time = 0;
  start_time = 0;
  end_time = 0;
  {
    bool got_first_time_range = false;
    QMapIterator<QString, EventTree*> i(G_event_tree_map);
    while (i.hasNext()) {
      // Get old tree info
      i.next();
      EventTree *event_tree = i.value();
      Events *events = event_tree->events;
      Event *first_event = &events->event_buffer[0];
      Event *last_event = &events->event_buffer[events->event_count-1];
      if (!got_first_time_range) {
        // Record initial times
        full_start_time = first_event->time;
        full_end_time = last_event->time;
        got_first_time_range = true;
      } else {
        // Update time range
        if (first_event->time < full_start_time) full_start_time = first_event->time;
        if (last_event->time > full_end_time) full_end_time = last_event->time;
      }
    }
  }

  // Zoom in as needed
  start_time = full_start_time;
  end_time = full_end_time;
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
    rebuild_frame_buffer = false;
    QPainter painter2(&frame_buffer);
    painter2.fillRect(QRect(0,0,w,h), QColor(255,255,255));
    // Draw alignment time
    if (G_event_tree_map.count() > 1) {
      QString alignment_mode = G_settings->value("alignment_mode", "Native").toString();  // One of "Native", "TimeZero", "EventId"
      if (alignment_mode == "EventId") {
        if (alignment_time > start_time && alignment_time < end_time) {
          painter2.setPen(QPen(ALIGNMENT_COLOR, 1, Qt::DashLine));
          int x = (int)(w * (alignment_time-start_time) / time_range);
          painter2.drawLine(x, 0, x, h);
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
      drawHierarchyLine(&painter2, event_tree->events, event_tree->tree, line_index, true);
      line_index++;
    }
    emit utilizationRecalculated();
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

  if (mouse_button_pressed) return;

  // Draw rollover info or histogram
  if (mouse_location.x() >= 0 && mouse_location.y() >= 0) {
    EventTreeNode *node_with_mouse = NULL;
    Events *events_with_mouse = NULL;
    QMapIterator<QString, EventTree*> i(G_event_tree_map);
    while (i.hasNext()) {
      // Get old tree info
      i.next();
      EventTree *event_tree = i.value();
      node_with_mouse = mouseOnEventsLine(event_tree->tree); /*+ this is not correct if some folders collapsed */
      if (node_with_mouse != NULL) {
        events_with_mouse = event_tree->events;
        break;
      }
    }
    if (node_with_mouse != NULL) {
      if (mouse_mode == MOUSE_MODE_EVENT_HISTOGRAM) {
	painter.fillRect(node_with_mouse->events_row_rect, ROW_HIGHLIGHT_COLOR2);
	drawEventHistogram(painter, node_with_mouse, events_with_mouse);
      } else if (mouse_mode == MOUSE_MODE_EVENT_INFO) {
	painter.fillRect(node_with_mouse->events_row_rect, ROW_HIGHLIGHT_COLOR2);
	drawEventInfo(painter, node_with_mouse, events_with_mouse);
      }
    }
  }
}

void EventsView::rebuildAndUpdate() {
  rebuild_frame_buffer = true;
  update();
}
