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

#ifndef _HelpfulFunctions_hpp_
#define _HelpfulFunctions_hpp_

#include <QImage>

enum class FitType {
  Inside,
  Outside
};

extern QRect getFittedRect(FitType fit, int window_w, int window_h, int image_w, int image_h);
extern QRectF getFittedRectF(FitType fit, int window_w, int window_h, int image_w, int image_h);
extern void recolorImage(QImage &image, QColor color);
extern QString getTimeUnitsAndFactor(uint64_t nsecs, uint64_t max_times_to_display, uint64_t *units_factor_ret);

#endif
