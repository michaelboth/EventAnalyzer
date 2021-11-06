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

#include <QMouseEvent>
#include <QPainter>
#include "EventsView.hpp"
#include "HelpfulFunctions.hpp"

EventsView::EventsView(QWidget *parent) : QWidget(parent) {
  // Track mouse when not pressed
  //setMouseTracking(true);

  // Allow keyboard input
  //setFocusPolicy(Qt::StrongFocus);
  // Allow for keyboard focus now
  //setFocus();

  // Load the logo
  QImage image = QImage(":/unikorn_logo.png");
  recolorImage(image, QColor(220,220,220));
  logo = QPixmap::fromImage(image);
}

EventsView::~EventsView() {
  // Nothing to do
}

void EventsView::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    // Prepare for mouse motion
    prev_mouse_location = QPoint(event->x(), event->y());
  }
  update();
}

void EventsView::mouseMoveEvent(QMouseEvent *event) {
  // Determine the change in motion
  /*+
  int dx = event->x() - prev_mouse_location.x();
  int dy = event->y() - prev_mouse_location.y();
  */
  prev_mouse_location = QPoint(event->x(), event->y());
}

void EventsView::mouseReleaseEvent(QMouseEvent * /*event*/) {
  // Nothing to do
}

void EventsView::paintEvent(QPaintEvent* /*event*/) {
  int w = width();
  int h = height();
  if (w <= 0 || h <= 0) { return; }

  // Define painter
  QPainter painter(this);

  // Fill in background
  painter.fillRect(QRect(0,0,w,h), QColor(255,255,255));

  // Draw the logo
  QRect inside_rect = getFittedRect(FitType::Inside, w/2, h, logo.width(), logo.height());
  inside_rect.moveLeft(w/4);
  painter.setRenderHint(QPainter::SmoothPixmapTransform,true);
  painter.drawPixmap(inside_rect, logo);
  painter.setRenderHint(QPainter::SmoothPixmapTransform,false);
}
