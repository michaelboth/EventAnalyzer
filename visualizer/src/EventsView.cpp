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
/*+ show vertical line where mouse is along with time in header, instead of in info dialog? */

#define LINE_SEPARATOR_COLOR QColor(230, 230, 230)
#define MIN_SELECTION_PIXEL_DIFF 10
#define ROLLOVER_BG_COLOR QColor(200, 200, 200)
#define ROLLOVER_TEXT_COLOR QColor(50, 50, 50)
#define ROLLOVER_TITLE_COLOR QColor(125, 125, 125)
#define ROLLOVER_SEPARATOR_COLOR QColor(175, 175, 175)

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
  // Calculate geometry
  line_h = (int)(G_font_point_size * LINE_HEIGHT_FACTOR);

  // Update font size
  QFont font = this->font();
  font.setPointSize(G_font_point_size);
  this->setFont(font);

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
  uint32_t first = 0;
  uint32_t last = node->num_event_instances-1;

  // See if before first event
  uint32_t first_event_index = node->event_indices[first];
  Event *first_event = &events->event_buffer[first_event_index];
  if (time < first_event->time) return first;

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
      if (last_visible_event_index >= parent->num_event_instances) last_visible_event_index = parent->num_event_instances - 1; // NOTE: just outside of range
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

  /*+ histogram */
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
      drawEventInfo(painter, node_with_mouse, events_with_mouse);
    }
  }
}

static QPixmap drawEventPixmap(int height, QColor color) {
  int w = (int)(height * 1.0f);
  QPixmap pixmap(w, height);
  pixmap.fill(Qt::transparent);
  QPainter painter(&pixmap);
  int x1 = 1;
  int x2 = w-3;
  int y1 = (int)(height * 0.2f);
  int y2 = (int)(height * 0.35f);
  int y3 = (int)(height * 0.65f);
  int y4 = (int)(height * 0.8f);
  painter.fillRect(x1, y1, 2, y3-y1, color);
  painter.fillRect(x2, y2, 2, y4-y2, color);
  painter.fillRect(x1, y2, x2-x1, y3-y2, color);
  return pixmap;
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
  int instance_w = fm.horizontalAdvance(" Instance ");
  int dialog_w = fm.horizontalAdvance(" filename::function_name::line_number ");
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
    image_icon = drawEventPixmap(th, node->color);
  }
  int icon_offset = 0;
  if (!image_icon.isNull()) {
    painter.setRenderHint(QPainter::SmoothPixmapTransform,true);
    icon_offset = m+image_icon.width();
    painter.drawPixmap(dialog_x+m, dialog_y, image_icon.width(), image_icon.height(), image_icon);
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
      QString text = " -";
      if (events->includes_file_location) {
        text = " " + QString(events->file_name_list[prev_event->file_name_index]);
        text += "," + QString(events->function_name_list[prev_event->function_name_index]);
        text += "():" + QString::number(prev_event->line_number);
      }
      painter.drawText(dialog_x, dialog_y, dialog_w-m*2, th, Qt::AlignLeft | Qt::AlignVCenter, text);
    }
    dialog_y += th;
    if (has_next_event) {
      QString text = " -";
      if (events->includes_file_location) {
        text = " " + QString(events->file_name_list[next_event->file_name_index]);
        text += "," + QString(events->function_name_list[next_event->function_name_index]);
        text += "():" + QString::number(next_event->line_number);
      }
      painter.drawText(dialog_x, dialog_y, dialog_w-m*2, th, Qt::AlignLeft | Qt::AlignVCenter, text);
    }
  }
}

void EventsView::rebuildAndUpdate() {
  rebuild_frame_buffer = true;
  update();
}
