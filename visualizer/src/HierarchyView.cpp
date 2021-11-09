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
#include "HierarchyView.hpp"
#include "HelpfulFunctions.hpp"
#include "main.hpp"

#define ARROW_ICON_COLOR QColor(0,0,0)
#define FOLDER_ICON_COLOR QColor(100,100,100)
#define EVENTS_ICON_COLOR QColor(0,100,255)

HierarchyView::HierarchyView(QWidget *parent) : QWidget(parent) {
  /*+
  prepareIcon("hierarchy_closed_arrow.png", true, ARROW_ICON_COLOR);
  prepareIcon("hierarchy_closed_folder.png", true, FOLDER_ICON_COLOR);
  prepareIcon("hierarchy_closed_thread.png", true, FOLDER_ICON_COLOR);
  prepareIcon("hierarchy_events.png", true, EVENTS_ICON_COLOR);
  prepareIcon("hierarchy_file.png", false, Qt::black);
  prepareIcon("hierarchy_opened_arrow.png", true, ARROW_ICON_COLOR);
  prepareIcon("hierarchy_opened_folder.png", true, FOLDER_ICON_COLOR);
  prepareIcon("hierarchy_opened_thread.png", true, FOLDER_ICON_COLOR);
  */
}

HierarchyView::~HierarchyView() {
  // Nothing to do
}

void HierarchyView::prepareIcon(QString filename, bool recolor, QColor color) {
  QImage image = QImage(":/"+filename);
  if (recolor) {
    recolorImage(image, color);
  }
  /*+
  G_pixels_per_point
  */
  /*+ test */image = image.scaledToHeight(line_h, Qt::SmoothTransformation);
  QPixmap pixmap = QPixmap::fromImage(image);
  /*+ validate on Mac
    QPixmap::devicePixelRatio()
  */
  icon_map[filename] = pixmap;
}

void HierarchyView::updateLineHeight() {
  // Calculate geometry
  line_h = (int)(G_font_point_size * 1.5f); /*+ Tune and create global constant */

  // Update font size
  QFont font = this->font();
  font.setPointSize(G_font_point_size);
  this->setFont(font);

  /*+ test */
  prepareIcon("hierarchy_closed_arrow.png", true, ARROW_ICON_COLOR);
  prepareIcon("hierarchy_closed_folder.png", true, FOLDER_ICON_COLOR);
  prepareIcon("hierarchy_closed_thread.png", true, FOLDER_ICON_COLOR);
  prepareIcon("hierarchy_events.png", true, EVENTS_ICON_COLOR);
  prepareIcon("hierarchy_file.png", false, Qt::black);
  prepareIcon("hierarchy_opened_arrow.png", true, ARROW_ICON_COLOR);
  prepareIcon("hierarchy_opened_folder.png", true, FOLDER_ICON_COLOR);
  prepareIcon("hierarchy_opened_thread.png", true, FOLDER_ICON_COLOR);

  // Update icon geometry
  QPixmap arrow_icon = icon_map["hierarchy_opened_arrow.png"];
  arrow_w = line_h * arrow_icon.width() / (float)arrow_icon.height();
  QPixmap image_icon = icon_map["hierarchy_opened_folder.png"];
  image_w = line_h * image_icon.width() / (float)image_icon.height();
}

/*+ get max width and number of lines needed for the scrollbars */

void HierarchyView::drawHierarchyLine(QPainter *painter, EventTreeNode *parent, int &line_index, int level) {
  int w = width();
  int x = level * G_font_point_size;
  int y = line_index * line_h;

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

  // Draw arrow
  if (!arrow_icon.isNull()) {
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

  // Draw name
  painter->drawText(x, y, w-x, line_h, Qt::AlignLeft | Qt::AlignVCenter, parent->name);
  line_index++;

  // Recurse
  if (parent->is_open) {
    for (auto child: parent->children) {
      drawHierarchyLine(painter, child, line_index, level+1);
    }
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
