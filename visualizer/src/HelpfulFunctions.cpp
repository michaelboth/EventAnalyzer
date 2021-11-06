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

#include "HelpfulFunctions.hpp"

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
