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
#include "UtilizationView.hpp"
#include "HelpfulFunctions.hpp"
#include "main.hpp"

UtilizationView::UtilizationView(QWidget *parent) : QWidget(parent) {
  // Nothing to do
}

UtilizationView::~UtilizationView() {
  // Nothing to do
}

void UtilizationView::updateLineHeight() {
  // Update font size
  QFont font = this->font();
  font.setPointSize(G_font_point_size);
  this->setFont(font);

  // Calculate geometry
  QFontMetrics fm = QFontMetrics(this->font());
  line_h = fm.height();
}

void UtilizationView::updateVOffset(int offset) {
  v_offset = offset;
  update();
}

void UtilizationView::drawHierarchyLine(QPainter *painter, UkEvents *events, EventTreeNode *parent, int &line_index) {
  int h = height();
  int w = width();
  int y = -v_offset + line_index * line_h;
  bool is_filtered = (parent->tree_node_type == TREE_NODE_IS_EVENT && G_event_filters.contains(parent->name));
  if (is_filtered) return;

  // only draw if visible
  if (y > -line_h && y < h) {
    // Row geometry
    QRect row_rect = QRect(0,y,w,line_h);

    // Highlight row if selected (selected in hierarchy view)
    if (parent->row_selected) {
      painter->fillRect(row_rect, ROW_SELECTED_COLOR);
    }

    if (parent->tree_node_type == TREE_NODE_IS_EVENT) {
      // Draw bar
      int bar_w = (int)(parent->utilization * w);
      if (bar_w > 0) {
	painter->fillRect(QRect(0,y+1,bar_w,line_h-2), parent->color);
      }

      // Draw Utilization
      char val_text[40];
      sprintf(val_text, "%0.1f", 100.0*parent->utilization);
      QString text = QString(val_text) + "%";
      if (parent->utilization >= 0.5) {
	painter->setPen(QPen(Qt::white, 1, Qt::SolidLine));
	painter->drawText(0, y, w, line_h, Qt::AlignLeft | Qt::AlignVCenter, " " + text);
      } else {
	painter->setPen(QPen(Qt::black, 1, Qt::SolidLine));
	painter->drawText(0, y, w, line_h, Qt::AlignRight | Qt::AlignVCenter, text + " ");
      }
    }
  }
  line_index++;

  // Recurse
  if (parent->is_open) {
    for (auto child: parent->children) {
      drawHierarchyLine(painter, events, child, line_index);
    }
  }
}

void UtilizationView::paintEvent(QPaintEvent* /*event*/) {
  int w = width();
  int h = height();
  if (w <= 0 || h <= 0) { return; }

  // Define painter
  QPainter painter(this);

  // Fill in background
  painter.fillRect(QRect(0,0,w,h), HIERARCHY_PROFILING_BG_COLOR);

  if (G_event_tree_map.count() == 0) {
    return;
  }

  // Draw event tree
  int line_index = 0;
  QMapIterator<QString, EventTree*> i(G_event_tree_map);
  while (i.hasNext()) {
    // Get old tree info
    i.next();
    EventTree *event_tree = i.value();
    drawHierarchyLine(&painter, event_tree->events, event_tree->tree, line_index);
  }
}
