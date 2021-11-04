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
#define ACTIVE_COLOR QColor(0, 0, 0)
#define SELECTED_COLOR QColor(0, 125, 255)
  QSize toolbar_button_size = QSize(32,32);
  QImage image = QImage(":/open.png");
  for (int y=0; y<image.height(); y++) {
    for (int x=0; x<image.width(); x++) {
      QColor color = image.pixelColor(x, y);
      QColor new_color = SELECTED_COLOR;
      new_color.setAlpha(color.alpha());
      image.setPixelColor(x, y, new_color);
    }
  }
  QIcon icon;
  icon.addPixmap(QPixmap::fromImage(image), QIcon::Mode::Normal, QIcon::State::Off);
  ui->loadButton->setIconSize(toolbar_button_size);
  ui->loadButton->setIcon(icon);

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
