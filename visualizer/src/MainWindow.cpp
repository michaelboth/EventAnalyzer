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

#include <QFileDialog>
#include <QPainter>
#include "ui_MainWindow.h"
#include "MainWindow.hpp"
#include "HelpfulFunctions.hpp"
#include "main.hpp"
#include "unikorn.h"

#define NORMAL_COLOR     QColor(50, 50, 50)
#define DISABLED_COLOR   QColor(200, 200, 200)
#define TOGGLE_ON_COLOR  QColor(0, 100, 255)
#define TOGGLE_OFF_COLOR QColor(150, 200, 255)
#define TOOLBAR_BUTTON_SIZE 32

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  // Set title
  setWindowTitle("Unikorn Viewer (version " + QString::number(UK_API_VERSION_MAJOR) + "." + QString::number(UK_API_VERSION_MINOR) + ")");

  // Hide the status bar
  //statusBar()->hide();
  statusBar()->showMessage("Event files loaded: 0, Filters Set: none, Total Events: 0");

  // Get the standard font size
  {
    QPainter painter(this);
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

  // Set the height of the headers to the font size
  ui->hierarchyHeader->updateHeight();
  //*+*/ui->eventsHeader->updateHeight();
  ui->profilingHeader->updateHeight();
  ui->hierarchyHeader->setTitle("Hierarchy");
  ui->profilingHeader->setTitle("Profiling");

  // Margins and spacing
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
  QString separator_attrs = "QWidget { background: rgb(220, 220, 220); border: none; }";
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
    "  background: rgb(240, 240, 240);"
    "  height: 14px;"
    "  margin: 0px 0px 0px 0px;"
    "  padding: 4px 2px 4px 2px;"
    "}"
    "QScrollBar::handle:horizontal {"
    "  background: rgb(140, 140, 140);"
    "  border-radius: 3px;"
    "  min-width: 6px;"
    "}"
    "QScrollBar::handle:horizontal:disabled {"
    "  background: rgb(200, 200, 200);"
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
    "  background: rgb(240, 240, 240);"
    "  height: 14px;"
    "  margin: 0px 14px 0px 14px;"
    "  padding: 4px 0px 4px 0px;"
    "}"
    "QScrollBar::handle:horizontal {"
    "  background: rgb(140, 140, 140);"
    "  border-radius: 3px;"
    "  min-width: 6px;"
    "}"
    "QScrollBar::handle:horizontal:disabled {"
    "  background: rgb(200, 200, 200);"
    "}"
    "QScrollBar::add-line:horizontal {" // Step right button
    "  background: rgb(240, 240, 240);"
    "  width: 14px;"
    "  subcontrol-position: right;"
    "  subcontrol-origin: margin;"
    "}"
    "QScrollBar::sub-line:horizontal {" // Step left button
    "  background: rgb(240, 240, 240);"
    "  width: 14px;"
    "  subcontrol-position: left;"
    "  subcontrol-origin: margin;"
    "}"
    "QScrollBar:left-arrow:horizontal, QScrollBar::right-arrow:horizontal {"
    "  border-radius: 3px;"
    "  width: 6px;"
    "  height: 6px;"
    "  background: rgb(140, 140, 140);"
    "}"
    "QScrollBar:left-arrow:horizontal:disabled, QScrollBar::right-arrow:horizontal:disabled {"
    "  background: rgb(200, 200, 200);"
    "}"
    "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {"
    "  background: none;"
    "}";
  QString vscroll_simple_attrs =
    "QScrollBar:vertical {"
    "  border: none;"
    "  background: rgb(240, 240, 240);"
    "  width: 14px;"
    "  margin: 0px 0px 0px 0px;"
    "  padding: 2px 4px 2px 4px;"
    "}"
    "QScrollBar::handle:vertical {"
    "  background: rgb(140, 140, 140);"
    "  border-radius: 3px;"
    "  min-height: 6px;"
    "}"
    "QScrollBar::handle:vertical:disabled {"
    "  background: rgb(200, 200, 200);"
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
  ui->hierarchyVScroll->setEnabled(false);
  ui->hierarchyHScroll->setEnabled(false);
  ui->eventsHScroll->setEnabled(false);
  ui->profilingHScroll->setEnabled(false);

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
  ui->showThreadsButton->setIcon(buildIcon(":/show_threads.png",            true,  NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->openFoldersButton->setIcon(buildIcon(":/open_folders.png",            false, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->closeFoldersButton->setIcon(buildIcon(":/close_folders.png",          false, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->sortByIdButton->setIcon(buildIcon(":/sort_by_id.png",                 true,  NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->sortByNameButton->setIcon(buildIcon(":/sort_by_name.png",             true,  NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->sortByTimeButton->setIcon(buildIcon(":/sort_by_time.png",             true,  NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->increaseFontSizeButton->setIcon(buildIcon(":/increase_font_size.png", false, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->decreaseFontSizeButton->setIcon(buildIcon(":/decrease_font_size.png", false, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));

  // Set initial scroll bar ranges
  ui->hierarchyVScroll->setRange(0, 0);
  ui->hierarchyHScroll->setRange(0, 0);
  ui->eventsHScroll->setRange(0, 0);
  ui->profilingHScroll->setRange(0, 0);

  // Make sure events window stretches
  ui->viewSplitter->setStretchFactor(0, 0);
  ui->viewSplitter->setStretchFactor(1, 255);
  ui->viewSplitter->setStretchFactor(2, 0);
  // Make sure the name column has a reasonable width
  int prev_name_column_width = G_settings->value("prev_name_column_width", 300).toInt();
  int event_column_width = 0; // Since the splitter widget does not allow hid, then the event colum will use all remaining width
  int prev_profiling_column_width = G_settings->value("prev_profiling_column_width", 150).toInt();
  QList<int> column_widths = {prev_name_column_width, event_column_width, prev_profiling_column_width};
  ui->viewSplitter->setSizes(column_widths);

  // Set usable widgets
  setWidgetUsability();

  // Connect some signals
  this->connect(ui->viewSplitter, SIGNAL(splitterMoved(int,int)), this, SLOT(updateColumnWidths(int,int)));
}

MainWindow::~MainWindow() {
  // Free loaded event files
  freeAllEventFiles();

  // Free main layout
  delete ui;
  ui = NULL;
}

void MainWindow::updateColumnWidths(int pos, int index) {
  if (index == 1) {
    G_settings->setValue("prev_name_column_width", pos);
  }
  if (index == 2) {
    int profiling_column_width = ui->viewSplitter->width() - pos - 2;
    G_settings->setValue("prev_profiling_column_width", profiling_column_width);
  }
}

static QPixmap recolorImageAndConvertToPixmap(QImage &image, QColor color) {
  recolorImage(image, color);
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
    icon.addPixmap(recolorImageAndConvertToPixmap(image, toggle_off_color), QIcon::Mode::Normal, QIcon::State::Off);
    icon.addPixmap(recolorImageAndConvertToPixmap(image, toggle_on_color),  QIcon::Mode::Normal, QIcon::State::On);
  } else {
    icon.addPixmap(recolorImageAndConvertToPixmap(image, normal_color), QIcon::Mode::Normal, QIcon::State::On);
    icon.addPixmap(recolorImageAndConvertToPixmap(image, normal_color), QIcon::Mode::Normal, QIcon::State::Off);
  }
  icon.addPixmap(recolorImageAndConvertToPixmap(image, disabled_color), QIcon::Mode::Disabled, QIcon::State::On);
  icon.addPixmap(recolorImageAndConvertToPixmap(image, disabled_color), QIcon::Mode::Disabled, QIcon::State::Off);
  icon.addPixmap(recolorImageAndConvertToPixmap(image, disabled_color), QIcon::Mode::Active, QIcon::State::On);
  icon.addPixmap(recolorImageAndConvertToPixmap(image, disabled_color), QIcon::Mode::Active, QIcon::State::Off);
  icon.addPixmap(recolorImageAndConvertToPixmap(image, disabled_color), QIcon::Mode::Selected, QIcon::State::On);
  icon.addPixmap(recolorImageAndConvertToPixmap(image, disabled_color), QIcon::Mode::Selected, QIcon::State::Off);
  return icon;
}

void MainWindow::setWidgetUsability() {
  bool event_files_loaded = (event_tree_map.count() > 0);
  bool event_files_selected = false;/*+*/
  bool folders_exist = eventFilesHaveFolders();
  bool threads_exist = eventFilesHaveThreads();
  bool filters_are_set = false;/*+*/
  bool font_size_can_grow = (G_font_point_size < G_max_font_point_size);
  bool font_size_can_shrink = (G_font_point_size > G_min_font_point_size);

  // Hierarchy toolbar
  ui->closeAllButton->setEnabled(event_files_loaded);
  ui->closeSelectedButton->setEnabled(event_files_selected);
  ui->setFilterButton->setEnabled(event_files_loaded);
  ui->clearFilterButton->setEnabled(filters_are_set);
  ui->showFoldersButton->setEnabled(folders_exist);
  ui->showThreadsButton->setEnabled(threads_exist);
  ui->openFoldersButton->setEnabled(event_files_loaded);
  ui->closeFoldersButton->setEnabled(event_files_loaded);
  ui->sortByIdButton->setEnabled(event_files_loaded);
  ui->sortByNameButton->setEnabled(event_files_loaded);
  ui->sortByTimeButton->setEnabled(event_files_loaded);
  ui->increaseFontSizeButton->setEnabled(event_files_loaded && font_size_can_grow);
  ui->decreaseFontSizeButton->setEnabled(event_files_loaded && font_size_can_shrink);

  // Update status bar
  QString message = "Event files loaded: " + QString::number(event_tree_map.count()) + "          Filters Set: none          Total Events: " + QString::number(totalEventInstances()); /*+ filters set */
  statusBar()->showMessage(message);
}

void MainWindow::on_loadButton_clicked() {
  QString prev_folder = G_settings->value("prev_load_folder", "").toString();
  QStringList files = QFileDialog::getOpenFileNames(this, "Load One Or More Event Files", prev_folder, "Event Files (*.events)");
  for (auto filename: files) {
    // Get folder
    QString folder = filename.section("/", -999, -2);
    //printf("folder: '%s'\n", folder.toLatin1().data());
    assert(!folder.isEmpty());
    G_settings->setValue("prev_load_folder", folder);
    QString name = filename.section("/", -1).section(".events", 0, 0);
    assert(!name.isEmpty());
    //printf("name: '%s'\n", name.toLatin1().data());

    // If events file already loaded, then delete old data first
    if (event_tree_map.contains(filename)) {
      // Remove old events file
      EventTree *old_tree = event_tree_map[filename];
      int items_removed = event_tree_map.remove(filename);
      assert(items_removed == 1);
      freeEvents(old_tree->events);
      delete old_tree;
    }

    // Load the events
    Events *events = loadEventsFile(filename.toLatin1().data());

    // Build the display tree
    EventTree *tree = new EventTree(events, name, folder, ui->showFoldersButton->isChecked(), ui->showThreadsButton->isChecked());
    SortType sort_type = ui->sortByIdButton->isChecked() ? SORT_BY_ID : ui->sortByNameButton->isChecked() ? SORT_BY_NAME : SORT_BY_TIME;
    tree->sortTree(sort_type);
    event_tree_map[filename] = tree; // NOTE: QMaps are ordered alphabetically
  }
  if (files.count() > 0) {
    setWidgetUsability();
    /*+ update display */
  }
}

void MainWindow::on_closeAllButton_clicked() {
  freeAllEventFiles();
  setWidgetUsability();
  /*+ update display */
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

void MainWindow::updateEventTreeBuild() {
  SortType sort_type = ui->sortByIdButton->isChecked() ? SORT_BY_ID : ui->sortByNameButton->isChecked() ? SORT_BY_NAME : SORT_BY_TIME;
  QMapIterator<QString, EventTree*> i(event_tree_map);
  while (i.hasNext()) {
    // Get old tree info
    i.next();
    QString filename = i.key();
    EventTree *tree = i.value();
    Events *events = tree->events;
    QString name = tree->name;
    QString folder = tree->folder;
    delete tree;
    // Build new tree
    tree = new EventTree(events, name, folder, ui->showFoldersButton->isChecked(), ui->showThreadsButton->isChecked());
    tree->sortTree(sort_type);
    event_tree_map[filename] = tree; // NOTE: QMaps are ordered alphabetically
  }
  /*+ redraw */
}

void MainWindow::freeAllEventFiles() {
  QMapIterator<QString, EventTree*> i(event_tree_map);
  while (i.hasNext()) {
    i.next();
    EventTree *tree = i.value();
    freeEvents(tree->events);
    delete tree;
  }
  event_tree_map.clear();
}

uint32_t MainWindow::totalEventInstances() {
  uint32_t count = 0;
  QMapIterator<QString, EventTree*> i(event_tree_map);
  while (i.hasNext()) {
    // Get old tree info
    i.next();
    EventTree *tree = i.value();
    Events *events = tree->events;
    count += events->event_count;
  }
  return count;
}

bool MainWindow::eventFilesHaveFolders() {
  QMapIterator<QString, EventTree*> i(event_tree_map);
  while (i.hasNext()) {
    // Get old tree info
    i.next();
    EventTree *tree = i.value();
    Events *events = tree->events;
    if (events->folder_info_count > 0) return true;
  }
  return false;
}

bool MainWindow::eventFilesHaveThreads() {
  QMapIterator<QString, EventTree*> i(event_tree_map);
  while (i.hasNext()) {
    // Get old tree info
    i.next();
    EventTree *tree = i.value();
    Events *events = tree->events;
    if (events->is_threaded) return true;
  }
  return false;
}

void MainWindow::on_showFoldersButton_clicked() {
  bool folders_exist = eventFilesHaveFolders();
  if (folders_exist) updateEventTreeBuild();
}

void MainWindow::on_showThreadsButton_clicked() {
  bool threads_exist = eventFilesHaveThreads();
  if (threads_exist) updateEventTreeBuild();
}

void MainWindow::on_openFoldersButton_clicked() {
  QMapIterator<QString, EventTree*> i(event_tree_map);
  while (i.hasNext()) {
    // Get old tree info
    i.next();
    EventTree *tree = i.value();
    tree->openAllFolders();
  }
  /*+ redraw */
}

void MainWindow::on_closeFoldersButton_clicked() {
  QMapIterator<QString, EventTree*> i(event_tree_map);
  while (i.hasNext()) {
    // Get old tree info
    i.next();
    EventTree *tree = i.value();
    tree->closeAllFolders();
  }
  /*+ redraw */
}

void MainWindow::updateEventTreeSort() {
  SortType sort_type = ui->sortByIdButton->isChecked() ? SORT_BY_ID : ui->sortByNameButton->isChecked() ? SORT_BY_NAME : SORT_BY_TIME;
  QMapIterator<QString, EventTree*> i(event_tree_map);
  while (i.hasNext()) {
    i.next();
    EventTree *tree = i.value();
    tree->sortTree(sort_type);
  }
  /*+ redraw */
}

void MainWindow::on_sortByIdButton_clicked() {
  bool is_checked = ui->sortByIdButton->isChecked();
  if (!is_checked) {
    ui->sortByIdButton->setChecked(true);
  }
  ui->sortByNameButton->setChecked(false);
  ui->sortByTimeButton->setChecked(false);
  if (is_checked) updateEventTreeSort();
}

void MainWindow::on_sortByNameButton_clicked() {
  bool is_checked = ui->sortByNameButton->isChecked();
  if (!is_checked) {
    ui->sortByNameButton->setChecked(true);
  }
  ui->sortByIdButton->setChecked(false);
  ui->sortByTimeButton->setChecked(false);
  if (is_checked) updateEventTreeSort();
}

void MainWindow::on_sortByTimeButton_clicked() {
  bool is_checked = ui->sortByTimeButton->isChecked();
  if (!is_checked) {
    ui->sortByTimeButton->setChecked(true);
  }
  ui->sortByIdButton->setChecked(false);
  ui->sortByNameButton->setChecked(false);
  if (is_checked) updateEventTreeSort();
}

void MainWindow::on_increaseFontSizeButton_clicked() {
  if (G_font_point_size < G_max_font_point_size) {
    G_font_point_size++;
    ui->hierarchyHeader->updateHeight();
    //*+*/ui->eventsHeader->updateHeight();
    ui->profilingHeader->updateHeight();
    setWidgetUsability();
    /*+ update headers heights */
    /*+ redraw */
  }
}

void MainWindow::on_decreaseFontSizeButton_clicked() {
  if (G_font_point_size > G_min_font_point_size) {
    G_font_point_size--;
    ui->hierarchyHeader->updateHeight();
    //*+*/ui->eventsHeader->updateHeight();
    ui->profilingHeader->updateHeight();
    setWidgetUsability();
    /*+ update headers heights */
    /*+ redraw */
  }
}
