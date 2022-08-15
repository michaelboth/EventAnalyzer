// Copyright 2022 Michael Both
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
#include "VerticalLabel.hpp"

VerticalLabel::VerticalLabel(QWidget *parent) : QWidget(parent) {
  // Nothing to do
}

VerticalLabel::~VerticalLabel() {
  // Nothing to do
}

void VerticalLabel::setText(QString _text) {
  text = _text;
}

void VerticalLabel::paintEvent(QPaintEvent* /*event*/) {
  int w = width();
  int h = height();
  if (w <= 0 || h <= 0) { return; }

  // Define painter
  QPainter painter(this);

  //painter.fillRect(QRect(0,0,w,h), Qt::red);

  // Mimic the gradient of the harizontal titles
  //  "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgb(180,180,180), stop:1 rgba(180,180,180,75));"
  QLinearGradient linearGrad(QPointF(0, 0), QPointF(w, 0));
  linearGrad.setColorAt(0, QColor(180,180,180,75));
  linearGrad.setColorAt(1, QColor(180,180,180));
  painter.setPen(Qt::NoPen);
  painter.setBrush(linearGrad);
  painter.drawRect(QRect(0,0,w,h));

  // Rotate to be vertical
  painter.translate(rect().center());
  painter.rotate(90); // Note after rotating, the rect() call switches dimensions
  painter.translate(-rect().center());

  // Draw text
  painter.setBrush(Qt::NoBrush);
  painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));
  //painter.drawText(0, 0, w, h, Qt::AlignCenter, text);
  painter.drawText(-h/2, 0, w+h, h, Qt::AlignCenter, text);
}
