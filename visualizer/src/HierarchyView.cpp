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
#include "main.hpp"

HierarchyView::HierarchyView(QWidget *parent) : QWidget(parent) {
  // Nothing to do
}

HierarchyView::~HierarchyView() {
  // Nothing to do
}

/*+ get max width and number of lines needed for the scrollbars */

void HierarchyView::drawHierarchyLine(QPainter *painter, EventTreeNode *parent, int &line_index, int level) {
  int w = width();
  int x = level * G_font_point_size;
  int line_height = (int)(G_font_point_size * 1.5f); /*+ pre - calculate and store in global */
  int y = line_index * line_height;
  painter->drawText(x, y, w-x, line_height, Qt::AlignLeft | Qt::AlignVCenter, parent->name);
  line_index++;
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

  /*+
  // Title
  painter.setPen(QPen(HEADER_TEXT_COLOR, 1, Qt::SolidLine));
  painter.drawText(0, 0, w, h, Qt::AlignCenter, title);

  // Separator at bottom
  painter.setPen(QPen(HEADER_SEPARATOR_COLOR, 1, Qt::SolidLine));
  painter.drawLine(0, h-1, w-1, h-1);
  */

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
