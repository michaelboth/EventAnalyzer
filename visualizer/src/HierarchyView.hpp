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

protected:
  void paintEvent(QPaintEvent *event);

signals:

public slots:

private:
  QMap<QString,QPixmap> icon_map;
  int line_h = 0;
  int arrow_w = 0;
  int image_w = 0;

  void drawHierarchyLine(QPainter *painter, EventTreeNode *tree, int &line_index, int level);
  void prepareIcon(QString filename, bool recolor, QColor color);
};

#endif
