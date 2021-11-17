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

#include <QPainter>
#include <QMouseEvent>
#include "HierarchyView.hpp"
#include "HelpfulFunctions.hpp"
#include "main.hpp"

/*+ when folder collapsed, scrollbar not adjusted */

#define EXTRA_MARGIN_FACTOR 0.5f

HierarchyView::HierarchyView(QWidget *parent) : QWidget(parent) {
  // Track mouse when not pressed
  setMouseTracking(true);
}

HierarchyView::~HierarchyView() {
  // Nothing to do
}

void HierarchyView::updateHOffset(int offset) {
  h_offset = offset;
  update();
}

void HierarchyView::updateVOffset(int offset) {
  v_offset = offset;
  update();
}

void HierarchyView::prepareIcon(QString filename, bool recolor, QColor color) {
  QImage image = QImage(":/"+filename);
  if (recolor) {
    recolorImage(image, color);
  }
  image = image.scaledToHeight((int)(line_h*G_pixels_per_point), Qt::SmoothTransformation);
  QPixmap pixmap = QPixmap::fromImage(image);
  //pixmap.setDevicePixelRatio(G_pixels_per_point); // Does not seems to be needed probably because main.cpp calls QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
  icon_map[filename] = pixmap;
}

void HierarchyView::updateLineHeight() {
  // Calculate geometry
  line_h = (int)(G_font_point_size * LINE_HEIGHT_FACTOR);

  // Update font size
  QFont font = this->font();
  font.setPointSize(G_font_point_size);
  this->setFont(font);

  // Rebuild icons to make sure there are correctly sized; if not, then edges will looked aliased
  prepareIcon("hierarchy_closed_arrow.png", true, ARROW_ICON_COLOR);
  prepareIcon("hierarchy_closed_folder.png", true, FOLDER_ICON_COLOR);
  prepareIcon("hierarchy_closed_thread.png", true, THREAD_ICON_COLOR);
  prepareIcon("hierarchy_events.png", true, EVENTS_ICON_COLOR);
  prepareIcon("hierarchy_file.png", false, Qt::black);
  prepareIcon("hierarchy_opened_arrow.png", true, ARROW_ICON_COLOR);
  prepareIcon("hierarchy_opened_folder.png", true, FOLDER_ICON_COLOR);
  prepareIcon("hierarchy_opened_thread.png", true, THREAD_ICON_COLOR);

  // Update icon geometry
  QPixmap arrow_icon = icon_map["hierarchy_opened_arrow.png"];
  arrow_w = line_h * arrow_icon.width() / (float)arrow_icon.height();
  QPixmap image_icon = icon_map["hierarchy_opened_folder.png"];
  image_w = line_h * image_icon.width() / (float)image_icon.height();
}

void HierarchyView::calculateGeometry(EventTreeNode *parent, int &line_index, int &max_width, int level) {
  QFontMetrics fm = fontMetrics();
  int x = level * G_font_point_size + arrow_w + image_w + (int)(line_h * EXTRA_MARGIN_FACTOR);
  int w = x + fm.horizontalAdvance(parent->name);
  if (w > max_width) max_width = w;
  line_index++;
  if (parent->is_open) {
    for (auto child: parent->children) {
      calculateGeometry(child, line_index, max_width, level+1);
    }
  }
}

void HierarchyView::calculateGeometry(int *visible_w_ret, int *actual_w_ret, int *visible_h_ret, int *actual_h_ret) {
  int line_index = 0;
  int max_width = 0;
  QMapIterator<QString, EventTree*> i(G_event_tree_map);
  while (i.hasNext()) {
    // Get old tree info
    i.next();
    EventTree *event_tree = i.value();
    calculateGeometry(event_tree->tree, line_index, max_width, 0);
  }

  *visible_w_ret = width();
  *visible_h_ret = height();
  *actual_w_ret = max_width;
  *actual_h_ret = line_index * line_h;
}

void HierarchyView::drawHierarchyLine(QPainter *painter, EventTreeNode *parent, int &line_index, int level) {
  int h = height();
  int w = width();
  int x = -h_offset + level * G_font_point_size;
  int y = -v_offset + line_index * line_h;
  if (y > h) return;

  // Determine icons to draw
  QPixmap arrow_icon;
  QPixmap image_icon;
  if (parent->tree_node_type == TREE_NODE_IS_FILE) {
    arrow_icon = parent->is_open ? icon_map["hierarchy_opened_arrow.png"] : icon_map["hierarchy_closed_arrow.png"];
    image_icon = icon_map["hierarchy_file.png"];
  } else if (parent->tree_node_type == TREE_NODE_IS_FOLDER) {
    arrow_icon = parent->is_open ? icon_map["hierarchy_opened_arrow.png"] : icon_map["hierarchy_closed_arrow.png"];
    image_icon = parent->is_open ? icon_map["hierarchy_opened_folder.png"] : icon_map["hierarchy_closed_folder.png"];
  } else if (parent->tree_node_type == TREE_NODE_IS_THREAD) {
    arrow_icon = parent->is_open ? icon_map["hierarchy_opened_arrow.png"] : icon_map["hierarchy_closed_arrow.png"];
    image_icon = parent->is_open ? icon_map["hierarchy_opened_thread.png"] : icon_map["hierarchy_closed_thread.png"];
  } else {
    image_icon = icon_map["hierarchy_events.png"];
  }

  // Reset some visual info
  parent->hierarchy_row_rect = QRect();
  parent->folder_rect = QRect();
  content_bottom_y = y + line_h;

  // only draw if visible
  if (y > -line_h) {
    // Remember the row geometry
    QRect row_rect = QRect(0,y,w,line_h);
    parent->hierarchy_row_rect = QRect(0,y,w,line_h);

    // If mouse is on the current row, then highlight it
    if (row_rect.contains(mouse_location)) {
      node_with_mouse = parent;
      row_with_mouse_rect = row_rect;
      painter->fillRect(row_rect, ROW_HIGHLIGHT_COLOR);
    }

    // Highlight row if selected
    if (parent->row_selected) {
      painter->fillRect(row_rect, ROW_SELECTED_COLOR);
    }

    // Draw arrow
    if (!arrow_icon.isNull()) {
      parent->folder_rect = QRect(x, y, arrow_w+image_w, line_h);
      painter->setRenderHint(QPainter::SmoothPixmapTransform,true);
      painter->drawPixmap(x, y, arrow_w, line_h, arrow_icon);
      painter->setRenderHint(QPainter::SmoothPixmapTransform,false);
    }
    x += arrow_w;

    // Draw image
    if (!image_icon.isNull()) {
      painter->setRenderHint(QPainter::SmoothPixmapTransform,true);
      painter->drawPixmap(x, y, image_w, line_h, image_icon);
      painter->setRenderHint(QPainter::SmoothPixmapTransform,false);
    }
    x += image_w;
    x += (int)(line_h * EXTRA_MARGIN_FACTOR);

    // Draw name
    painter->drawText(x, y, w-x, line_h, Qt::AlignLeft | Qt::AlignVCenter, parent->name);
  }

  // Recurse
  line_index++;
  if (parent->is_open) {
    for (auto child: parent->children) {
      drawHierarchyLine(painter, child, line_index, level+1);
    }
  }
}

void HierarchyView::mousePressEvent(QMouseEvent *event) {
  clearSelection();
  if (node_with_mouse != NULL) {
    node_with_mouse->row_selected = true;
    // Check if on folder to open/close
    if (node_with_mouse->folder_rect.contains(event->x(), event->y())) {
      node_with_mouse->is_open = !node_with_mouse->is_open;
    }
  }
  emit hierarchyChanged(); // Tool button, to close selected files, may need to be enabled
  update();
}

void HierarchyView::mouseMoveEvent(QMouseEvent *event) {
  mouse_location = QPoint(event->x(), event->y());
  if ((node_with_mouse != NULL && !row_with_mouse_rect.contains(mouse_location)) || (node_with_mouse == NULL && mouse_location.y() < content_bottom_y)) {
    // Need to update since a different row should be highlighted
    update();
  }
}

void HierarchyView::mouseReleaseEvent(QMouseEvent * /*event*/) {
  // Nothing to do
}

void HierarchyView::leaveEvent(QEvent * /*event*/) {
  mouse_location = QPoint(-1,-1);
  update();
}

void HierarchyView::clearSelection(EventTreeNode *parent) {
  parent->row_selected = false;
  for (auto child: parent->children) {
    clearSelection(child);
  }
}

void HierarchyView::clearSelection() {
  QMapIterator<QString, EventTree*> i(G_event_tree_map);
  while (i.hasNext()) {
    // Get old tree info
    i.next();
    EventTree *event_tree = i.value();
    clearSelection(event_tree->tree);
  }
}

void HierarchyView::paintEvent(QPaintEvent* /*event*/) {
  int w = width();
  int h = height();
  if (w <= 0 || h <= 0) { return; }

  // Define painter
  QPainter painter(this);

  // Fill in background
  painter.fillRect(QRect(0,0,w,h), HIERARCHY_PROFILING_BG_COLOR);

  // Update some high level geometry
  content_bottom_y = 0;
  row_with_mouse_rect = QRect();
  node_with_mouse = NULL;

  // Draw hierarchy tree
  painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));
  int line_index = 0;
  QMapIterator<QString, EventTree*> i(G_event_tree_map);
  while (i.hasNext()) {
    // Get old tree info
    i.next();
    EventTree *event_tree = i.value();
    drawHierarchyLine(&painter, event_tree->tree, line_index, 0);
  }
}
