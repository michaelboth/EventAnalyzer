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
  void getTimeRange(double *percent_visible_ret, double *percent_offset_ret);
  void updateTimeOffset(double percent_offset);
  bool timeRangeSelected();

protected:
  void paintEvent(QPaintEvent *event);
  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
  void leaveEvent(QEvent *event);

signals:
  void timeRangeChanged();           // Just and percentage showing how much is visible and offset
  void timeRangeSelectionChanged();
  void visibleTimeRangeChanged(uint64_t start_time, uint64_t end_time); // Used to update the time units
  void selectionTimeRangeChanged(uint64_t elapsed_time); // Used to update the header's selection range

public slots:
  void updateVOffset(int offset);
  void zoomToAll();
  void zoomIn();
  void zoomOut();
  void zoomToRegion();
  void rebuildAndUpdate();

private:
  QPixmap logo;
  int line_h = 0;
  int v_offset = 0;
  QPoint mouse_location = QPoint(-1,-1);
  uint64_t start_time = 0;
  uint64_t end_time = 0;
  double time_range = 0;
  double percent_visible = 1.0;
  double percent_offset = 0.0;
  bool mouse_button_pressed = false;
  int selected_time_range_x1 = -1;
  int selected_time_range_x2 = -1;
  bool rebuild_frame_buffer = true;
  QImage frame_buffer;

  void drawHierarchyLine(QPainter *painter, Events *events, EventTreeNode *tree, int &line_index, int ancestor_open);
  EventTreeNode *mouseOnEventsLine(EventTreeNode *parent);
};

#endif
