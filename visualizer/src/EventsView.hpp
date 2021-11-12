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

#ifndef _EventsView_h_
#define _EventsView_h_
 
#include <QWidget>
#include <QPixmap>
#include "EventTree.hpp"

class EventsView : public QWidget
{
  Q_OBJECT

public:
  EventsView(QWidget *parent=0);
  ~EventsView();
  void updateLineHeight();

protected:
  void paintEvent(QPaintEvent *event);
  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
  void leaveEvent(QEvent *event);

signals:

public slots:
  void updateVOffset(int offset);

private:
  QPixmap logo;
  int line_h = 0;
  int v_offset = 0;
  QPoint mouse_location = QPoint(-1,-1);
  QRect row_with_mouse_rect;
  EventTreeNode *node_with_mouse = NULL;
  uint64_t start_time = 0;
  uint64_t end_time = 0;

  void drawHierarchyLine(QPainter *painter, Events *events, EventTreeNode *tree, int &line_index, int ancestor_open);
};

#endif
