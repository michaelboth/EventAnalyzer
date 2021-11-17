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
#include <cmath>
#include "EventsHeader.hpp"
#include "HelpfulFunctions.hpp"
#include "main.hpp"

EventsHeader::EventsHeader(QWidget *parent) : QWidget(parent) {
  // Nothing to do
}

EventsHeader::~EventsHeader() {
  // Nothing to do
}

void EventsHeader::updateUnits(uint64_t _start_time, uint64_t _end_time) {
  if (start_time != _start_time || end_time != _end_time) {
    start_time = _start_time;
    end_time = _end_time;
    update();
  }
}

void EventsHeader::updateSelectionRange(uint64_t _selected_time_range) {
  selected_time_range = _selected_time_range;
  update();
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
  painter.fillRect(QRect(0,0,w,h), HIERARCHY_PROFILING_BG_COLOR);

  // Separator at bottom
  painter.setPen(QPen(HEADER_SEPARATOR_COLOR, 1, Qt::SolidLine));
  painter.drawLine(0, h-1, w-1, h-1);

  // Check if any units to draw
  if (start_time == end_time) return;

  // Determine how many times can be displayed in the header
  QFontMetrics fm = painter.fontMetrics();
  int ref_w = 2 * fm.horizontalAdvance("|999 years ");
  uint64_t max_times_to_display = (uint64_t)ceilf(w / (float)ref_w);

  // Deetermine the time precision; starting with nanoseconds
  uint64_t units_factor;
  uint64_t nsecs = end_time - start_time;
  QString units = getTimeUnitsAndFactor(nsecs, max_times_to_display, &units_factor);

  // Draw units
  for (uint64_t i=0; i<max_times_to_display; i++) {
    int x = (int)i*ref_w;
    double offset_factor = x / (double)w;
    double time = start_time / (double)units_factor + (offset_factor * nsecs) / units_factor;
    if (i == 0) {
      // Time
      painter.setPen(QPen(HEADER_TEXT_COLOR, 1, Qt::SolidLine));
      char val_text[40];
      sprintf(val_text, "%0.1f", time);
      QString text = QString(val_text) + " " + units;
      painter.drawText(x+1, 0, w, h, Qt::AlignLeft | Qt::AlignBottom, text);
    } else {
      // Tick mark
      painter.setPen(QPen(HEADER_TEXT_COLOR, 1, Qt::SolidLine));
      painter.drawLine(x, 0, x, h);
      // Additional time
      painter.setPen(QPen(HEADER_TEXT_COLOR, 1, Qt::SolidLine));
      double diff = time - start_time / (double)units_factor;
      char val_text[40];
      sprintf(val_text, "%0.1f", diff);
      QString text = "+" + QString(val_text) + " " + units;
      painter.drawText(x+1, 0, w, h, Qt::AlignLeft | Qt::AlignBottom, text);
    }
  }

  // Draw selected range (only report elasped time)
  if (selected_time_range > 0) {
    int selection_times_to_display = 1;
    uint64_t selection_units_factor;
    QString selection_units = getTimeUnitsAndFactor(selected_time_range, selection_times_to_display, &selection_units_factor);
    // Highlight the background
    QColor time_selection_color = TIME_SELECTION_COLOR;
    time_selection_color.setAlpha(175);
    painter.fillRect(QRect(0,0,w,h), time_selection_color);
    // Draw the time
    QFont font = painter.font();
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(QPen(Qt::white, 1, Qt::SolidLine));
    double adjusted_selection_time = selected_time_range / (double)selection_units_factor;
    char val_text[40];
    sprintf(val_text, "%0.1f", adjusted_selection_time);
    QString text = QString(val_text) + " " + selection_units;
    painter.drawText(0, 0, w, h, Qt::AlignCenter, text);
    return;
  }
}
