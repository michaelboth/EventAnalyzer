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

#include <QApplication>
#include <QScreen>
#include <QDesktopWidget>
#include <QStyleFactory>
#include "main.hpp"
#include "MainWindow.hpp"

QSettings *G_settings = NULL;
double G_pixels_per_point = 0;
int G_font_point_size = 0;
int G_min_font_point_size = 0;
int G_max_font_point_size = 0;
QMap<QString,EventTree*> G_event_tree_map;
QStringList G_event_filters; // List of event names to be filtered

int main(int argc, char *argv[]) {
  QApplication *app = new QApplication(argc, argv);

  G_settings = new QSettings("Michael Both", "Unikorn Viewer");

  // Get some screen stats
  G_pixels_per_point = qApp->primaryScreen()->devicePixelRatio();
  if (G_pixels_per_point > 1.0) {
    printf("Using high density display: pixels per point = %f\n", G_pixels_per_point);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
  }
  // Create home screen
  MainWindow *main_window = new MainWindow(NULL);

  // Set the common (across OSes) visual style
  app->setStyle(QStyleFactory::create("Fusion"));

  // Display home screen
  main_window->show();

  // Run main event loop until application completes
  int rc = app->exec();

  // Remember the font size
  G_settings->setValue("font_point_size", G_font_point_size);

  // Clean up
  delete main_window;
  delete G_settings;
  delete app;
  return rc;
}
