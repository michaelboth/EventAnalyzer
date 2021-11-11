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
#include "EventsHeader.hpp"
#include "main.hpp"

EventsHeader::EventsHeader(QWidget *parent) : QWidget(parent) {
  // Nothing to do
}

EventsHeader::~EventsHeader() {
  // Nothing to do
}

void EventsHeader::updateHeight() {
  int height = (int)(G_font_point_size * 1.8f);
  setMinimumHeight(height);

  QFont font = this->font();
  font.setPointSize(G_font_point_size);
  this->setFont(font);

  update();
}

void EventsHeader::paintEvent(QPaintEvent* /*event*/) {
  int w = width();
  int h = height();
  if (w <= 0 || h <= 0) { return; }

  // Define painter
  QPainter painter(this);

  // Fill in background
  painter.fillRect(QRect(0,0,w,h), EVENTS_BG_COLOR);

  /*+ draw units and selection range
  painter.setPen(QPen(HEADER_TEXT_COLOR, 1, Qt::SolidLine));
  painter.drawText(0, 0, w, h, Qt::AlignCenter, title);
  */

  // Separator at bottom
  painter.setPen(QPen(HEADER_SEPARATOR_COLOR, 1, Qt::SolidLine));
  painter.drawLine(0, h-1, w-1, h-1);
}
