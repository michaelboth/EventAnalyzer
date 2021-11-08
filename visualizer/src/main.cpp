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
#include <QPainter>
#include "main.hpp"
#include "MainWindow.hpp"

QSettings *G_settings = NULL;
double G_pixels_per_point = 0;
int G_default_font_point_size = 0;
int G_font_point_size = 0;
int G_min_font_point_size = 0;
int G_max_font_point_size = 0;

int main(int argc, char *argv[]) {
  QApplication *app = new QApplication(argc, argv);

  G_settings = new QSettings("Michael Both", "Unikorn Viewer");

  // Get some screen stats
  G_pixels_per_point = qApp->primaryScreen()->devicePixelRatio();
  int dpi_x = app->desktop()->physicalDpiX();
  int dpi_y = app->desktop()->physicalDpiY();
  printf("Screen info: pixels_per_point=%f, dpi_x=%d, dpi_y=%d\n", G_pixels_per_point, dpi_x, dpi_y);
  if (G_pixels_per_point > 1.0) {
    printf("Using high density display\n");
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
  }
  // Create home screen
  MainWindow *main_window = new MainWindow(NULL);

  // Set the common (across OSes) visual style
  app->setStyle(QStyleFactory::create("Fusion"));

  // Get the default font size
  // Get the standard font size
  {
    QPainter painter(main_window);
    QFont font = painter.font();
    int pixel_size = font.pixelSize();
    int point_size = font.pointSize();
    qreal point_size_real = font.pointSizeF();
    G_default_font_point_size = point_size;
    G_min_font_point_size = point_size / 2;
    G_max_font_point_size = point_size * 2;
    G_font_point_size = G_settings->value("font_point_size", point_size).toInt();
    if (G_font_point_size < G_min_font_point_size) G_font_point_size = G_min_font_point_size;
    if (G_font_point_size > G_max_font_point_size) G_font_point_size = G_max_font_point_size;
    /*+*/printf("pixel_size=%d, point_size=%d, point_size_real=%f, G_font_point_size=%d\n", pixel_size, point_size, point_size_real, G_font_point_size);
  }

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
