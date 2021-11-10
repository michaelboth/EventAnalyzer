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

#ifndef _HierarchyView_h_
#define _HierarchyView_h_
 
#include <QWidget>
#include <QPainter>
#include <QMap>
#include "EventTree.hpp"

class HierarchyView : public QWidget
{
  Q_OBJECT

public:
  HierarchyView(QWidget *parent=0);
  ~HierarchyView();
  void updateLineHeight();
  void calculateGeometry(int *visible_w_ret, int *actual_w_ret, int *visible_h_ret, int *actual_h_ret);

protected:
  void paintEvent(QPaintEvent *event);
  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);

signals:

public slots:
  void updateHOffset(int offset);
  void updateVOffset(int offset);

private:
  QMap<QString,QPixmap> icon_map;
  int line_h = 0;
  int arrow_w = 0;
  int image_w = 0;
  int h_offset = 0;
  int v_offset = 0;

  void drawHierarchyLine(QPainter *painter, EventTreeNode *tree, int &line_index, int level);
  void prepareIcon(QString filename, bool recolor, QColor color);
  void calculateGeometry(EventTreeNode *parent, int &line_index, int &max_width, int level);
};

#endif
