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

#include <QtWidgets>
#include "ui_MainWindow.h"
#include "MainWindow.hpp"
#include "main.hpp"

#define NORMAL_COLOR   QColor(0, 0, 0)
#define DISABLED_COLOR QColor(125, 125, 125)
#define ACTIVE_COLOR   QColor(0, 125, 255)    // Mouse over
#define SELECTED_COLOR QColor(0, 125, 255)    // Toggle is on
#define TOOLBAR_BUTTON_SIZE 32

/*+ When loading event file, build three different display trees: sorted by ID, name, time */

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  // Hide the status bar
  statusBar()->hide();

  // Margins and spacing
  /*+ tune */
  /*+
  centralWidget()->layout()->setContentsMargins(0,0,0,0);
  centralWidget()->layout()->setSpacing(0);
  */

  /*+ tool button icons */
  ui->loadButton->setIconSize(QSize(TOOLBAR_BUTTON_SIZE, TOOLBAR_BUTTON_SIZE));
  ui->loadButton->setIcon(buildIcon(":/open.png", NORMAL_COLOR, DISABLED_COLOR, ACTIVE_COLOR, SELECTED_COLOR));
  ui->showFoldersButton->setCheckable(true);
  ui->showFoldersButton->setIconSize(QSize(TOOLBAR_BUTTON_SIZE, TOOLBAR_BUTTON_SIZE));
  ui->showFoldersButton->setIcon(buildIcon(":/open.png", NORMAL_COLOR, DISABLED_COLOR, ACTIVE_COLOR, SELECTED_COLOR));

  // Set usable widgets
  setWidgetUsability();

  // Connect some signals
  //*+*/this->connect(ui->EventWindow, SIGNAL(clickAt(QPoint)), ui->EventHeader, SLOT(updateUnits(QPoint)));
}

MainWindow::~MainWindow() {
  // Nothing to do
}

void MainWindow::closeEvent(QCloseEvent *event) {
  /*+*/printf("closeEvent()\n");
  /*+ save window size? */
  /*+
    event->ignore();
  */
  event->accept();
}

QPixmap MainWindow::recolorImage(QImage &image, QColor color) {
  for (int y=0; y<image.height(); y++) {
    for (int x=0; x<image.width(); x++) {
      int alpha = image.pixelColor(x, y).alpha();
      color.setAlpha(alpha);
      image.setPixelColor(x, y, color);
    }
  }
  /*+
  QPixmap pixmap;
  pixmap.convertFromImage(image);
  return pixmap;
  */
  return QPixmap::fromImage(image);
}

QIcon MainWindow::buildIcon(QString filename, QColor normal_color, QColor disabled_color, QColor active_color, QColor selected_color) {
  // Load image
  QImage image = QImage(filename);
  int tool_button_size = (int)(TOOLBAR_BUTTON_SIZE * G_pixels_per_point);
  // Resize to tool button size
  image = image.scaled(QSize(tool_button_size,tool_button_size), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
  // Build icon
  QIcon icon;
  /*+ get this right */
  icon.addPixmap(recolorImage(image, normal_color),   QIcon::Mode::Normal,   QIcon::State::Off);
  icon.addPixmap(recolorImage(image, disabled_color), QIcon::Mode::Disabled, QIcon::State::Off);
  icon.addPixmap(recolorImage(image, active_color),   QIcon::Mode::Active,   QIcon::State::Off);
  icon.addPixmap(recolorImage(image, selected_color), QIcon::Mode::Selected, QIcon::State::Off);
  /*+ handle On state */
  icon.addPixmap(recolorImage(image, normal_color),   QIcon::Mode::Normal,   QIcon::State::On);
  icon.addPixmap(recolorImage(image, disabled_color), QIcon::Mode::Disabled, QIcon::State::On);
  icon.addPixmap(recolorImage(image, active_color),   QIcon::Mode::Active,   QIcon::State::On);
  icon.addPixmap(recolorImage(image, selected_color), QIcon::Mode::Selected, QIcon::State::On);
  return icon;
}

void MainWindow::setWidgetUsability() {
  /*+*/
}

void MainWindow::on_loadButton_clicked() {
  /*+*/
}

void MainWindow::on_closeAllButton_clicked() {
  /*+*/
}

void MainWindow::on_closeSelectedButton_clicked() {
  /*+*/
}

void MainWindow::on_setFilterButton_clicked() {
  /*+*/
}

void MainWindow::on_clearFilterButton_clicked() {
  /*+*/
}

void MainWindow::on_showFoldersButton_clicked() {
  /*+*/
}

void MainWindow::on_showThreadsButton_clicked() {
  /*+*/
}

void MainWindow::on_openFoldersButton_clicked() {
  /*+*/
}

void MainWindow::on_closeFoldersButton_clicked() {
  /*+*/
}

void MainWindow::on_sortByIdButton_clicked() {
  /*+*/
}

void MainWindow::on_sortByNameButton_clicked() {
  /*+*/
}

void MainWindow::on_sortByTimeButton_clicked() {
  /*+*/
}

void MainWindow::on_increaseFontSizeButton_clicked() {
  /*+*/
}

void MainWindow::on_decreaseFontSizeButton_clicked() {
  /*+*/
}
