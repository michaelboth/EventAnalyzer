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
#include "GenericHeader.hpp"
#include "main.hpp"

GenericHeader::GenericHeader(QWidget *parent) : QWidget(parent) {
  // Nothing to do
}

GenericHeader::~GenericHeader() {
  // Nothing to do
}

void GenericHeader::updateHeight() {
  QFont font = this->font();
  font.setPointSize(G_font_point_size);
  this->setFont(font);

  // Calculate geometry
  QFontMetrics fm = QFontMetrics(this->font());
  int height = fm.height();
  setMinimumHeight(height);

  update();
}

void GenericHeader::setTitle(QString _title) {
  title = _title;
  update();
}

void GenericHeader::paintEvent(QPaintEvent* /*event*/) {
  int w = width();
  int h = height();
  if (w <= 0 || h <= 0) { return; }

  // Define painter
  QPainter painter(this);

  // Create a nice gradient
  QLinearGradient linearGrad(QPointF(0, 0), QPointF(0, h));
  linearGrad.setColorAt(0,   QColor(50, 50, 50));
  linearGrad.setColorAt(1,   QColor(125, 125, 125));

  // Fill in background
  painter.setPen(Qt::NoPen);
  painter.setBrush(linearGrad);
  painter.drawRect(QRect(0,0,w,h));

  // Title
  painter.setPen(QPen(QColor(200, 200, 200), 1, Qt::SolidLine));
  painter.drawText(0, 0, w, h, Qt::AlignCenter, title);
}
