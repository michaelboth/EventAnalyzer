// Copyright 2021,2023 Michael Both
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
#include <QMap>
#include <QCursor>
#include "EventTree.hpp"

class EventsView : public QWidget
{
  Q_OBJECT

public:
  enum MouseMode {
    MOUSE_MODE_EVENT_NONE,
    MOUSE_MODE_EVENT_INFO,
    MOUSE_MODE_EVENT_HISTOGRAM,
    MOUSE_MODE_EVENT_GHOSTING,
    MOUSE_MODE_TIME_SHIFT,
  };

  EventsView(QWidget *parent=0);
  ~EventsView();
  void updateLineHeight();
  void getTimeRange(double *percent_visible_ret, double *percent_offset_ret);
  void updateTimeOffset(double percent_offset);
  bool timeRangeSelected();
  void setMouseMode(MouseMode mouse_mode);
  void hasEventsOutsideOfVisibleRegion(UkEvents *events, EventTreeNode *events_row, bool *events_to_the_left_ret, bool *events_to_the_right_ret);
  void centerPrevEvent(UkEvents *events, EventTreeNode *events_row);
  void centerNextEvent(UkEvents *events, EventTreeNode *events_row);
  //* No longer used */void centerLargestEvent(UkEvents *events, EventTreeNode *events_row);
  void setShowMinDurationsMode(bool _enabled);
  void setShowMaxDurationsMode(bool _enabled);
  bool hasGhostedEvents();
  void clearEventGhosting();

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
  void selectionTimeRangeChanged(int x1, int x2, uint64_t elapsed_time); // Used to update the header's selection range
  void utilizationRecalculated();

public slots:
  void updateVOffset(int offset);
  void zoomToAll();
  void zoomIn();
  void zoomOut();
  void zoomToRegion();
  void rebuildAndUpdate();
  void popupContextMenu(const QPoint &mouse_location);
  void updateTimeAlignment();

private:
  QPixmap logo;
  QMap<QString,QPixmap> icon_map;
  int line_h = 0;
  int v_offset = 0;
  QPoint mouse_location = QPoint(-1,-1);
  uint64_t full_start_time = 0;
  uint64_t full_end_time = 0;
  uint64_t start_time = 0; // In the zoomed in view
  uint64_t end_time = 0;   // In the zoomed in view
  double time_range = 0;   // Of the zoomed in view
  double percent_visible = 1.0;
  double percent_offset = 0.0;
  bool mouse_button_pressed = false;
  int selected_time_range_x1 = -1;
  int selected_time_range_x2 = -1;
  bool rebuild_frame_buffer = true;
  QImage frame_buffer;
  MouseMode mouse_mode = MOUSE_MODE_EVENT_INFO;
  uint64_t alignment_time = 0;
  QCursor ghosting_cursor;
  bool show_min_durations = false;
  bool show_max_durations = false;

  void prepareIcon(QString filename, bool recolor, QColor color);
  void drawHierarchyLine(QPainter *painter, UkEvents *events, EventTreeNode *tree, int &line_index, int ancestor_open);
  EventTreeNode *mouseOnEventsLine(EventTreeNode *parent);
  void alternateEventGhosting(EventTreeNode *node, EventTree *event_tree);
  void drawEventInfo(QPainter &painter, EventTreeNode *node, UkEvents *events);
  void drawEventHistogram(QPainter &painter, EventTreeNode *node, UkEvents *events);
  uint32_t calculateHistogram(int num_buckets, uint32_t *buckets, EventTreeNode *node, UkEvents *events, bool get_gap_durations, uint64_t *min_ret, uint64_t *avg_ret, uint64_t *max_ret);
};

#endif
