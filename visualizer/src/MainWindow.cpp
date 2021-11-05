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

#define NORMAL_COLOR     QColor(50, 50, 50)
#define DISABLED_COLOR   QColor(200, 200, 200)
#define TOGGLE_ON_COLOR  QColor(0, 100, 255)
#define TOGGLE_OFF_COLOR QColor(150, 200, 255)
#define TOOLBAR_BUTTON_SIZE 38

/*+ When loading event file, build three different display trees: sorted by ID, name, time */

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  // Initial variables
  sort_type = SORT_BY_ID;
  /*+ sessions */
  /*+ filters (encapsulate in main structure) */
  /*+ show_folders */
  /*+ show_threads */
  /*+ font size */

  // Hide the status bar
  //statusBar()->hide();
  /*+ show event stats: number of files, total events */
  statusBar()->showMessage("Event files loaded: 0");

  // Margins and spacing
  /*+ tune */
  centralWidget()->layout()->setContentsMargins(0,0,0,0);
  centralWidget()->layout()->setSpacing(0);
  ui->hierarchyVLayout->setContentsMargins(0,0,0,0);
  ui->hierarchyVLayout->setSpacing(0);
  ui->hierarchyLayout->setContentsMargins(0,0,0,0);
  ui->hierarchyLayout->setSpacing(0);
  ui->eventsLayout->setContentsMargins(0,0,0,0);
  ui->eventsLayout->setSpacing(0);
  ui->profilingLayout->setContentsMargins(0,0,0,0);
  ui->profilingLayout->setSpacing(0);

  // Decorations
  this->setStyleSheet("QWidget { background: rgb(240, 240, 240); }");
  QString separator_attrs = "QWidget { background: rgb(140, 140, 140); border: none; }";
  ui->hierarchyVLine->setStyleSheet(separator_attrs);
  ui->eventsHLine->setStyleSheet(separator_attrs);
  ui->statusHLine->setStyleSheet(separator_attrs);
  // Main view splitter
  QString splitter_attrs =
    "QSplitter {"
    "  border: none;"
    "}"
    "QSplitter::handle:horizontal {"
    "  background: rgb(140, 140, 140);"
    "  width: 10px;"
    "}";
  //"QSplitter::handle { image: url(images/splitter.png); }"
  //"QSplitter::handle:vertical { height: 2px; }"
  //"QSplitter::handle:pressed { url(images/splitter_pressed.png); }";
  // Main view
  ui->viewSplitter->setStyleSheet(splitter_attrs);
  ui->hierarchyHeader->setStyleSheet("QWidget { background: rgb(220, 220, 220); border: none; }");
  ui->hierarchyView->setStyleSheet("QWidget { background: rgb(220, 220, 220); border: none; }");
  ui->eventsHeader->setStyleSheet("QWidget { background: rgb(255, 255, 255); border: none; }");
  ui->eventsView->setStyleSheet("QWidget { background: rgb(255, 255, 255); border: none; }");
  ui->profilingHeader->setStyleSheet("QWidget { background: rgb(220, 220, 220); border: none; }");
  ui->profilingView->setStyleSheet("QWidget { background: rgb(220, 220, 220); border: none; }");
  // Scrollbar
  QString hscroll_simple_attrs =
    "QScrollBar:horizontal {"
    "  border: none;"
    "  background: rgb(200, 200, 200);"
    "  height: 14px;"
    "  margin: 0px 0px 0px 0px;"
    "  padding: 2px 2px 2px 2px;"
    "}"
    "QScrollBar::handle:horizontal {"
    "  background: white;"
    "  border-radius: 5px;"
    "  min-width: 10px;"
    "}"
    "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {" // Step left/right buttons
    "  background: none;"
    "  width: 0px;"
    "}"
    "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {"
    "  background: none;"
    "}";
  QString hscroll_advanced_attrs =
    "QScrollBar:horizontal {"
    "  border: none;"
    "  background: rgb(200, 200, 200);"
    "  height: 14px;"
    "  margin: 0px 14px 0px 14px;"
    "  padding: 2px 2px 2px 2px;"
    "}"
    "QScrollBar::handle:horizontal {"
    "  background: white;"
    "  border-radius: 5px;"
    "  min-width: 10px;"
    "}"
    "QScrollBar::add-line:horizontal {" // Step right button
    "  background: rgb(200, 200, 200);"
    "  border-left: 1px solid rgb(140, 140, 140);"
    "  width: 13px;"
    "  subcontrol-position: right;"
    "  subcontrol-origin: margin;"
    "}"
    "QScrollBar::sub-line:horizontal {" // Step left button
    "  background: rgb(200, 200, 200);"
    "  border-right: 1px solid rgb(140, 140, 140);"
    "  width: 13px;"
    "  subcontrol-position: left;"
    "  subcontrol-origin: margin;"
    "}"
    "QScrollBar:left-arrow:horizontal, QScrollBar::right-arrow:horizontal {"
    "  border-radius: 3px;"
    "  width: 6px;"
    "  height: 6px;"
    "  background: white;"
    "}"
    "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {"
    "  background: none;"
    "}";
  QString vscroll_simple_attrs =
    "QScrollBar:vertical {"
    "  border: none;"
    "  background: rgb(200, 200, 200);"
    "  width: 14px;"
    "  margin: 0px 0px 0px 0px;"
    "  padding: 2px 2px 2px 2px;"
    "}"
    "QScrollBar::handle:vertical {"
    "  background: white;"
    "  border-radius: 5px;"
    "  min-height: 10px;"
    "}"
    "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {" // Step left/right buttons
    "  background: none;"
    "  height: 0px;"
    "}"
    "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
    "  background: none;"
    "}";
  ui->hierarchyVScroll->setStyleSheet(vscroll_simple_attrs);
  ui->hierarchyHScroll->setStyleSheet(hscroll_simple_attrs);
  ui->eventsHScroll->setStyleSheet(hscroll_advanced_attrs);
  ui->profilingHScroll->setStyleSheet(hscroll_simple_attrs);

  // Create the list of tool buttons
  QList<QToolButton *> tool_buttons = {
    ui->loadButton,
    ui->closeAllButton,
    ui->closeSelectedButton,
    ui->setFilterButton,
    ui->clearFilterButton,
    ui->showFoldersButton,
    ui->showThreadsButton,
    ui->openFoldersButton,
    ui->closeFoldersButton,
    ui->sortByIdButton,
    ui->sortByNameButton,
    ui->sortByTimeButton,
    ui->increaseFontSizeButton,
    ui->decreaseFontSizeButton
  };

  // Set button sizes
  QSize button_size = QSize(TOOLBAR_BUTTON_SIZE, TOOLBAR_BUTTON_SIZE);
  for (auto button: tool_buttons) button->setIconSize(button_size);

  // Set the toolbar style
  QString tool_button_style =
    "QToolButton {"
    "  margin: 0;"
    "  border: 0;"
    "  padding: 0;"
    "}"
    "QToolButton::hover {"
    "  background: rgb(200, 200, 200);"
    "}";
  for (auto button: tool_buttons) button->setStyleSheet(tool_button_style);

  // Set button icons
  ui->loadButton->setIcon(buildIcon(":/load.png",                           false, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->closeAllButton->setIcon(buildIcon(":/close.png",                      false, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->closeSelectedButton->setIcon(buildIcon(":/close_selected.png",        false, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->setFilterButton->setIcon(buildIcon(":/filter.png",                    false, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->clearFilterButton->setIcon(buildIcon(":/clear_filter.png",            false, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->showFoldersButton->setIcon(buildIcon(":/show_folders.png",            true,  NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->showThreadsButton->setIcon(buildIcon(":/show_threads.png",            true,  NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));/*+ need better icon */
  ui->openFoldersButton->setIcon(buildIcon(":/open_folders.png",            false, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->closeFoldersButton->setIcon(buildIcon(":/close_folders.png",          false, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->sortByIdButton->setIcon(buildIcon(":/sort_by_id.png",                 true,  NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->sortByNameButton->setIcon(buildIcon(":/sort_by_name.png",             true,  NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->sortByTimeButton->setIcon(buildIcon(":/sort_by_time.png",             true,  NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR)); /*+ need better icon */
  ui->increaseFontSizeButton->setIcon(buildIcon(":/increase_font_size.png", false, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->decreaseFontSizeButton->setIcon(buildIcon(":/decrease_font_size.png", false, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));

  // Set button states
  /*+
  ui->showFoldersButton->setCheckable(true);
  ui->showThreadsButton->setCheckable(true);
  */

  /*+
  ui->showFoldersButton->setChecked(true);
  */

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

QIcon MainWindow::buildIcon(QString filename, bool is_toggle, QColor normal_color, QColor disabled_color, QColor toggle_on_color, QColor toggle_off_color) {
  // Load image
  QImage image = QImage(filename);
  int tool_button_size = (int)(TOOLBAR_BUTTON_SIZE * G_pixels_per_point);
  // Resize to tool button size
  image = image.scaled(QSize(tool_button_size,tool_button_size), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
  // Build icon
  QIcon icon;
  if (is_toggle) {
    icon.addPixmap(recolorImage(image, toggle_off_color), QIcon::Mode::Normal,   QIcon::State::Off);
    icon.addPixmap(recolorImage(image, toggle_on_color),  QIcon::Mode::Normal,   QIcon::State::On);
    icon.addPixmap(recolorImage(image, disabled_color),   QIcon::Mode::Disabled, QIcon::State::Off);
  } else {
    icon.addPixmap(recolorImage(image, normal_color),   QIcon::Mode::Normal,   QIcon::State::Off);
    icon.addPixmap(recolorImage(image, disabled_color), QIcon::Mode::Disabled, QIcon::State::Off);
  }
  return icon;
}

void MainWindow::setWidgetUsability() {
  /*+*/
  bool event_files_loaded = false;
  bool event_files_selected = false;
  ui->closeAllButton->setEnabled(event_files_loaded);
  ui->closeSelectedButton->setEnabled(event_files_selected);
  ui->setFilterButton->setEnabled(event_files_loaded);
  ui->clearFilterButton->setEnabled(event_files_loaded);
  /*+
  ui->showFoldersButton->setEnabled(event_files_loaded);
  ui->showThreadsButton->setEnabled(event_files_loaded);
  ui->openFoldersButton->setEnabled(event_files_loaded);
  ui->closeFoldersButton->setEnabled(event_files_loaded);
  ui->sortByIdButton->setEnabled(event_files_loaded);
  ui->sortByNameButton->setEnabled(event_files_loaded);
  ui->sortByTimeButton->setEnabled(event_files_loaded);
  ui->increaseFontSizeButton->setEnabled(event_files_loaded);
  ui->decreaseFontSizeButton->setEnabled(event_files_loaded);
  */
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
  sort_type = SORT_BY_ID;
  ui->sortByNameButton->setChecked(false);
  ui->sortByTimeButton->setChecked(false);
  /*+ switch to ID tree */
}

void MainWindow::on_sortByNameButton_clicked() {
  sort_type = SORT_BY_NAME;
  ui->sortByIdButton->setChecked(false);
  ui->sortByTimeButton->setChecked(false);
  /*+ switch to name tree */
}

void MainWindow::on_sortByTimeButton_clicked() {
  sort_type = SORT_BY_TIME;
  ui->sortByIdButton->setChecked(false);
  ui->sortByNameButton->setChecked(false);
  /*+ switch to time tree */
}

void MainWindow::on_increaseFontSizeButton_clicked() {
  /*+*/
}

void MainWindow::on_decreaseFontSizeButton_clicked() {
  /*+*/
}
