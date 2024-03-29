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

#ifndef _UtilizationView_h_
#define _UtilizationView_h_
 
#include <QWidget>
#include "EventTree.hpp"

class UtilizationView : public QWidget
{
  Q_OBJECT

public:
  UtilizationView(QWidget *parent=0);
  ~UtilizationView();
  void updateLineHeight();

protected:
  void paintEvent(QPaintEvent *event);

signals:

public slots:
  void updateVOffset(int offset);

private:
  int line_h = 0;
  int v_offset = 0;

  void drawHierarchyLine(QPainter *painter, UkEvents *events, EventTreeNode *tree, int &line_index);
};

#endif
