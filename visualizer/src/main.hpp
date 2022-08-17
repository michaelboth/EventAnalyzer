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

#ifndef _main_hpp_
#define _main_hpp_

#include <QSettings>
#include <QStringList>
#include <QMap>
#include "EventTree.hpp"
#include "unikorn_file_loader.h"

extern QSettings *G_settings;
extern double G_pixels_per_point;
extern int G_font_point_size;
extern int G_min_font_point_size;
extern int G_max_font_point_size;
extern QStringList G_event_filters;

extern QMap<QString,EventTree*> G_event_tree_map; // QMap is always sorted by key

#define HIERARCHY_PROFILING_BG_COLOR QColor(220, 220, 220)
#define EVENTS_BG_COLOR QColor(255, 255, 255)
#define HEADER_TEXT_COLOR QColor(100, 100, 100)
#define HEADER_SEPARATOR_COLOR QColor(200, 200, 200)
#define ROW_HIGHLIGHT_COLOR QColor(0, 0, 0, 50)
#define ROW_SELECTED_COLOR QColor(0, 100, 255, 50)
#define TIME_SELECTION_COLOR QColor(200, 0, 0)

#define ARROW_ICON_COLOR QColor(0, 0, 0)
#define FOLDER_ICON_COLOR QColor(100, 0, 255)
#define THREAD_ICON_COLOR QColor(100, 0, 255)
#define EVENTS_ICON_COLOR QColor(0, 100, 255)

#endif
