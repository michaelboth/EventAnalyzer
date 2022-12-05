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
#include <QBitmap>
#include "HelpfulFunctions.hpp"
#include "main.hpp"

#define CURSOR_SIZE 32   // 32 is more portable
#define GHOST_CURSOR_HOT_SPOT_X 3
#define GHOST_CURSOR_HOT_SPOT_Y 4

QCursor createGhostingCursor() {
  // Usage
  //   Custom:
  //     QCursor cursor = createCrossHairsCursor();
  //   Built in cursors
  //     QCursor cursor = Qt::ArrowCursor;
  //     QCursor cursor = Qt::CrossCursor;
  //     etc
  //   Setting
  //     this->setCursor(cursor);
  //   Reverting to parent cursor
  //     this->unsetCursor();

  int cursor_size = CURSOR_SIZE; // IMPORTANT: Required for portability
  QBitmap bitmap(cursor_size, cursor_size);
  QBitmap mask(cursor_size, cursor_size);

  // Load the reference image
  QImage image(":/ghost_pointer_32x32.png");
  if (CURSOR_SIZE != 32) {
    image = image.scaled(CURSOR_SIZE, CURSOR_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation /* Qt::FastTransformation */);
  }

  // Create the two bit masks to define black, white, and transparent
  {
    bitmap.clear();
    mask.clear();

    /*+ set pixel density? */

    QPainter bpainter(&bitmap);
    QPainter mpainter(&mask);

    // The cursor bitmap (B) and mask (M) bits are combined like this:
    //   B=1 and M=1 gives black.
    //   B=0 and M=1 gives white.
    //   B=0 and M=0 gives transparent.
    //   B=1 and M=0 gives an XOR'd result under Windows, undefined results on all other platforms.

    bpainter.setBrush(Qt::NoBrush);
    bpainter.setPen(QPen(Qt::color1, 1.0, Qt::SolidLine));

    for (int x=0; x<cursor_size; x++) {
      for (int y=0; y<cursor_size; y++) {
	int gray = image.pixelColor(x,y).red();
	bool is_black = (gray < 100);
	if (is_black) {
	  bpainter.drawPoint(x, y);
	  mpainter.drawPoint(x, y);
	}
      }
    }
  }

  QCursor cursor = QCursor(bitmap, mask, GHOST_CURSOR_HOT_SPOT_X, GHOST_CURSOR_HOT_SPOT_Y);

  return cursor;
}

QRect getFittedRect(FitType fit, int window_w, int window_h, int image_w, int image_h) {
  if (fit == FitType::Inside) {
    int new_image_w = window_w;
    int new_image_h = new_image_w * image_h / (float)image_w;
    if (new_image_h > window_h) {
      new_image_h = window_h;
      new_image_w = new_image_h * image_w / (float)image_h;
    }
    int x_offset = (window_w - new_image_w) / 2;
    int y_offset = (window_h - new_image_h) / 2;
    return QRect(x_offset, y_offset, new_image_w, new_image_h);
  } else {
    int new_image_w = window_w;
    int new_image_h = new_image_w * image_h / (float)image_w;
    if (new_image_h < window_h) {
      new_image_h = window_h;
      new_image_w = new_image_h * image_w / (float)image_h;
    }
    int x_offset = (window_w - new_image_w) / 2;
    int y_offset = (window_h - new_image_h) / 2;
    return QRect(x_offset, y_offset, new_image_w, new_image_h);
  }
}

QRectF getFittedRectF(FitType fit, int window_w, int window_h, int image_w, int image_h) {
  if (fit == FitType::Inside) {
    float new_image_w = window_w;
    float new_image_h = new_image_w * image_h / (float)image_w;
    if (new_image_h > window_h) {
      new_image_h = window_h;
      new_image_w = new_image_h * image_w / (float)image_h;
    }
    float x_offset = (window_w - new_image_w) / 2.0f;
    float y_offset = (window_h - new_image_h) / 2.0f;
    return QRectF(x_offset, y_offset, new_image_w, new_image_h);
  } else {
    float new_image_w = window_w;
    float new_image_h = new_image_w * image_h / (float)image_w;
    if (new_image_h < window_h) {
      new_image_h = window_h;
      new_image_w = new_image_h * image_w / (float)image_h;
    }
    float x_offset = (window_w - new_image_w) / 2.0f;
    float y_offset = (window_h - new_image_h) / 2.0f;
    return QRectF(x_offset, y_offset, new_image_w, new_image_h);
  }
}

void recolorImage(QImage &image, QColor color) {
  for (int y=0; y<image.height(); y++) {
    for (int x=0; x<image.width(); x++) {
      int alpha = image.pixelColor(x, y).alpha();
      color.setAlpha(alpha);
      image.setPixelColor(x, y, color);
    }
  }
}

QString niceValueText(double value) {
  // Don't show too much of the fraction if the integer part is big
  char val_text[40];
  if (value >= 100) {
    sprintf(val_text, "%d", (int)value);
  } else if (value >= 10) {
    sprintf(val_text, "%0.1f", value);
  } else if (value >= 1) {
    sprintf(val_text, "%0.2f", value);
  } else {
    sprintf(val_text, "%0.3f", value);
  }
  return QString(val_text);
}

QString getTimeUnitsAndFactor(uint64_t nsecs, uint64_t max_times_to_display, uint64_t *units_factor_ret) {
  uint64_t usecs = nsecs / 1000;
  uint64_t msecs = nsecs / 1000000;
  uint64_t secs = nsecs / 1000000000;
  uint64_t mins = nsecs / 1000000000 / 60;
  uint64_t hours = nsecs / 1000000000 / 60 / 60;
  uint64_t days = nsecs / 1000000000 / 60 / 60 / 24;

  QString units;
  uint64_t units_factor;
  if (days > max_times_to_display) {
    units = "days";
    units_factor = 1000000000LLU * 60 * 60 * 24;
  } else if (hours > max_times_to_display) {
    units = "hours";
    units_factor = 1000000000LLU * 60 * 60;
  } else if (mins > max_times_to_display) {
    units = "mins";
    units_factor = 1000000000LLU * 60;
  } else if (secs > max_times_to_display) {
    units = "secs";
    units_factor = 1000000000;
  } else if (msecs > max_times_to_display) {
    units = "msecs";
    units_factor = 1000000;
  } else if (usecs > max_times_to_display) {
    units = "usecs";
    units_factor = 1000;
  } else {
    units = "nsecs";
    units_factor = 1;
  }

  *units_factor_ret = units_factor;
  return units;
}

QPixmap drawEventIcon(int height, QColor color) {
  int icon_h = height * G_pixels_per_point;
  int icon_w = (int)(icon_h * 1.0f);
  QPixmap pixmap(icon_w, icon_h);
  pixmap.setDevicePixelRatio(G_pixels_per_point);
  icon_h /= G_pixels_per_point;
  icon_w /= G_pixels_per_point;
  pixmap.fill(Qt::transparent);
  QPainter painter(&pixmap);

  int x1 = 1;
  int x2 = icon_w-3;
  int y1 = (int)(icon_h * 0.2f);
  int y2 = (int)(icon_h * 0.35f);
  int y3 = (int)(icon_h * 0.65f);
  int y4 = (int)(icon_h * 0.8f);
  painter.fillRect(x1, y1, 2, y3-y1, color);
  painter.fillRect(x2, y2, 2, y4-y2, color);
  painter.fillRect(x1, y2, x2-x1, y3-y2, color);

  return pixmap;
}

QPixmap drawUpArrow(int width, int height, QColor color) {
  QPixmap pixmap(width, height);
  pixmap.setDevicePixelRatio(G_pixels_per_point);
  width /= G_pixels_per_point;
  height /= G_pixels_per_point;
  pixmap.fill(Qt::transparent);
  QPainter painter(&pixmap);

  float mid_x = width / 2.0f;
  float left = 0;
  float right = width;
  float top = 0;
  float head_y = right - left;
  float bottom = height - 1;
  float x1 = width * 0.4f;
  float x2 = width * 0.6f;
  const QPointF points[7] = { QPointF(x1, head_y),
                              QPointF(left, head_y),
                              QPointF(mid_x, top),
                              QPointF(right, head_y),
                              QPointF(x2, head_y),
                              QPointF(x2, bottom),
                              QPointF(x1, bottom)
  };
  painter.setPen(Qt::NoPen);
  painter.setBrush(color);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.drawPolygon(points, 7);

  return pixmap;
}
