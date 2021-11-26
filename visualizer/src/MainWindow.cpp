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
#include <QMessageBox>
#include <QScreen>
#include "ui_MainWindow.h"
#include "MainWindow.hpp"
#include "HelpfulFunctions.hpp"
#include "TimeAlignDialog.hpp"
#include "EventFilterDialog.hpp"
#include "main.hpp"
#include "unikorn.h"

#define NORMAL_COLOR     QColor(50, 50, 50)
#define IMPORTANT_COLOR  QColor(200, 0, 0)
#define DISABLED_COLOR   QColor(200, 200, 200)
#define TOGGLE_ON_COLOR  QColor(0, 100, 255)
#define TOGGLE_OFF_COLOR QColor(150, 200, 255)

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  // Set title
  setWindowIcon(QIcon(":/hierarchy_file.png")); // Icon shown at top left, but not file based icon
  setWindowTitle("Unikorn Viewer (version " + QString::number(UK_API_VERSION_MAJOR) + "." + QString::number(UK_API_VERSION_MINOR) + ")");

  // Hide the status bar
  //statusBar()->hide();
  statusBar()->showMessage("Event files loaded: 0, Filters Set: none, Total Events: 0");

  // Define a good window size
  QSize screen_size = screen()->availableSize();
  int screen_w = (int)(screen_size.width() * 0.7f);
  int screen_h = (int)(screen_size.height() * 0.7f);
  int screen_x = (screen_size.width() - screen_w)/2;
  int screen_y = (screen_size.height() - screen_h)/2;
  setGeometry(screen_x, screen_y, screen_w, screen_h);

  // Get the standard font size
  {
    QFont font = this->font();
#if defined(_WIN32)
    int point_size = font.pointSize();
#elif defined(__APPLE__)
    int point_size = font.pointSize();
#else
    int point_size = (int)(font.pointSize() * 1.2f);
#endif
    G_min_font_point_size = point_size / 2;
    G_max_font_point_size = point_size * 2;
    G_font_point_size = G_settings->value("font_point_size", point_size).toInt();
    if (G_font_point_size < G_min_font_point_size) G_font_point_size = G_min_font_point_size;
    if (G_font_point_size > G_max_font_point_size) G_font_point_size = G_max_font_point_size;
  }

  // Set the default time alignment
  G_settings->setValue("alignment_mode", "Native");  // One of "Native", "TimeZero", "EventId"

  // Set the height of the headers to the font size
  ui->hierarchyHeader->updateHeight();
  ui->eventsHeader->updateHeight();
  ui->profilingHeader->updateHeight();
  ui->hierarchyView->updateLineHeight();
  ui->eventsView->updateLineHeight();
  ui->profilingView->updateLineHeight();
  ui->hierarchyHeader->setTitle("Event Files");
  ui->profilingHeader->setTitle("Utilization");

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
  //this->setStyleSheet("QWidget { background: rgb(240, 240, 240); }");
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
    "  padding: 2px 2px 2px 2px;"
    "}"
    "QScrollBar::handle:horizontal {"
    "  background: rgb(100, 100, 100);"
    "  border-radius: 5px;"
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
    "  padding: 2px 0px 2px 0px;"
    "}"
    "QScrollBar::handle:horizontal {"
    "  background: rgb(100, 100, 100);"
    "  border-radius: 5px;"
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
    "  background: rgb(100, 100, 100);"
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
    "  padding: 2px 2px 2px 2px;"
    "}"
    "QScrollBar::handle:vertical {"
    "  background: rgb(100, 100, 100);"
    "  border-radius: 5px;"
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

  // Hide the profiling scroll but maintain the geometry
  ui->profilingHScroll->setRange(0, 0);
  ui->profilingHScroll->setEnabled(false);

  // Create the list of tool buttons
  QList<QToolButton *> tool_buttons = {
    // Hierarcy toolbar
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
    ui->decreaseFontSizeButton,
    // Events toolbar
    ui->timeAlignButton,
    ui->mouseModeInfoButton,
    ui->mouseModeHistogramButton,
    ui->mouseModeTimeShiftButton,
    ui->zoomToAllButton,
    ui->zoomInButton,
    ui->zoomOutButton,
    ui->zoomToSelectedButton,
    ui->prevEventButton,
    ui->nextEventButton
  };

  // Calculate standard tool button icon size
#if defined(_WIN32)
  int toolbar_icon_size = (int)(iconSize().height() * 1.4f);
#elif defined(__APPLE__)
  int toolbar_icon_size = 32;
#else
  int toolbar_icon_size = (int)(iconSize().height() * 1.6f);
#endif
  QSize button_size = QSize(toolbar_icon_size, toolbar_icon_size);

  // Set button sizes
  for (auto button: tool_buttons) button->setIconSize(button_size);

  // Set the logo
  QPixmap logo_pixmap = QPixmap(":/unikorn_logo.png");
  logo_pixmap = logo_pixmap.scaledToHeight(toolbar_icon_size, Qt::SmoothTransformation);
  ui->logoLabel->setPixmap(logo_pixmap);

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

  // Set hierarchy toolbar button icons
  ui->loadButton->setIcon(buildIcon(":/load.png",                           false, toolbar_icon_size, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->closeAllButton->setIcon(buildIcon(":/close.png",                      false, toolbar_icon_size, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->closeSelectedButton->setIcon(buildIcon(":/close_selected.png",        false, toolbar_icon_size, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->setFilterButton->setIcon(buildIcon(":/filter.png",                    false, toolbar_icon_size, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->clearFilterButton->setIcon(buildIcon(":/clear_filter.png",            false, toolbar_icon_size, IMPORTANT_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->showFoldersButton->setIcon(buildIcon(":/show_folders.png",            true,  toolbar_icon_size, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->showThreadsButton->setIcon(buildIcon(":/show_threads.png",            true,  toolbar_icon_size, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->openFoldersButton->setIcon(buildIcon(":/open_folders.png",            false, toolbar_icon_size, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->closeFoldersButton->setIcon(buildIcon(":/close_folders.png",          false, toolbar_icon_size, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->sortByIdButton->setIcon(buildIcon(":/sort_by_id.png",                 true,  toolbar_icon_size, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->sortByNameButton->setIcon(buildIcon(":/sort_by_name.png",             true,  toolbar_icon_size, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->sortByTimeButton->setIcon(buildIcon(":/sort_by_time.png",             true,  toolbar_icon_size, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->increaseFontSizeButton->setIcon(buildIcon(":/increase_font_size.png", false, toolbar_icon_size, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->decreaseFontSizeButton->setIcon(buildIcon(":/decrease_font_size.png", false, toolbar_icon_size, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));

  // Set events toolbar button icons
  ui->timeAlignButton->setIcon(buildIcon(":/time_align.png",                     false, toolbar_icon_size, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->mouseModeInfoButton->setIcon(buildIcon(":/mouse_mode_info.png",            true,  toolbar_icon_size, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->mouseModeHistogramButton->setIcon(buildIcon(":/mouse_mode_histogram.png",  true,  toolbar_icon_size, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->mouseModeTimeShiftButton->setIcon(buildIcon(":/mouse_mode_time_shift.png", true,  toolbar_icon_size, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->zoomToAllButton->setIcon(buildIcon(":/zoom_to_all.png",                    false, toolbar_icon_size, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->zoomInButton->setIcon(buildIcon(":/zoom_in.png",                           false, toolbar_icon_size, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->zoomOutButton->setIcon(buildIcon(":/zoom_out.png",                         false, toolbar_icon_size, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->zoomToSelectedButton->setIcon(buildIcon(":/zoom_to_selected.png",          false, toolbar_icon_size, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->prevEventButton->setIcon(buildIcon(":/prev_event.png",                     false, toolbar_icon_size, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));
  ui->nextEventButton->setIcon(buildIcon(":/next_event.png",                     false, toolbar_icon_size, NORMAL_COLOR, DISABLED_COLOR, TOGGLE_ON_COLOR, TOGGLE_OFF_COLOR));

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
  // Update the events toolbar spacer width
  ui->eventsToolbarSpacer->changeSize(prev_name_column_width, 5, QSizePolicy::Fixed, QSizePolicy::Minimum);
  ui->eventsToolbarLayout->invalidate();

  // Havn't decied on if this feature should be implemented
  ui->mouseModeTimeShiftButton->setHidden(true);

  // Set usable widgets
  setWidgetUsability();

  // Update scrollbars
  updateHierarchyScrollbars();
  updateEventsScrollRange();

  // Update views
  updateViews();

  // Connect some signals
  this->connect(ui->viewSplitter, SIGNAL(splitterMoved(int,int)), this, SLOT(updateColumnWidths(int,int)));
  this->connect(ui->hierarchyHScroll, SIGNAL(valueChanged(int)), ui->hierarchyView, SLOT(updateHOffset(int)));
  this->connect(ui->hierarchyVScroll, SIGNAL(valueChanged(int)), ui->hierarchyView, SLOT(updateVOffset(int)));
  this->connect(ui->hierarchyVScroll, SIGNAL(valueChanged(int)), ui->eventsView, SLOT(updateVOffset(int)));
  this->connect(ui->hierarchyVScroll, SIGNAL(valueChanged(int)), ui->profilingView, SLOT(updateVOffset(int)));
  this->connect(ui->hierarchyView, SIGNAL(hierarchySelectionChanged()), this, SLOT(setWidgetUsability()));
  this->connect(ui->hierarchyView, SIGNAL(hierarchySelectionChanged()), ui->eventsView, SLOT(rebuildAndUpdate()));
  this->connect(ui->hierarchyView, SIGNAL(hierarchyHeightChanged()), this, SLOT(updateHierarchyScrollbars()));
  this->connect(ui->eventsView, SIGNAL(timeRangeChanged()), this, SLOT(updateEventsScrollRange()));
  this->connect(ui->eventsView, SIGNAL(timeRangeSelectionChanged()), this, SLOT(setWidgetUsability()));
  this->connect(ui->eventsView, SIGNAL(visibleTimeRangeChanged(uint64_t,uint64_t)), ui->eventsHeader, SLOT(updateUnits(uint64_t,uint64_t)));
  this->connect(ui->eventsView, SIGNAL(selectionTimeRangeChanged(uint64_t)), ui->eventsHeader, SLOT(updateSelectionRange(uint64_t)));
  this->connect(ui->eventsView, SIGNAL(utilizationRecalculated()), ui->profilingView, SLOT(update()));
  this->connect(ui->eventsView, SIGNAL(utilizationRecalculated()), this, SLOT(setWidgetUsability()));
  this->connect(ui->zoomToAllButton, SIGNAL(clicked()), ui->eventsView, SLOT(zoomToAll()));
  this->connect(ui->zoomInButton, SIGNAL(clicked()), ui->eventsView, SLOT(zoomIn()));
  this->connect(ui->zoomOutButton, SIGNAL(clicked()), ui->eventsView, SLOT(zoomOut()));
  this->connect(ui->zoomToSelectedButton, SIGNAL(clicked()), ui->eventsView, SLOT(zoomToRegion()));
  this->connect(ui->eventsHScroll, SIGNAL(valueChanged(int)), this, SLOT(updateEventsTimeOffset(int)));
}

MainWindow::~MainWindow() {
  // Free loaded event files
  freeAllEventFiles();

  // Free main layout
  delete ui;
  ui = NULL;
}

void MainWindow::resizeEvent(QResizeEvent * /*event*/) {
  //printf("new window size = %d x %d\n", event->size().width(), event->size().height());
  updateHierarchyScrollbars();
}

void MainWindow::updateColumnWidths(int pos, int index) {
  if (index == 1) {
    G_settings->setValue("prev_name_column_width", pos);
    // Update the events toolbar spacer width
    ui->eventsToolbarSpacer->changeSize(pos, 5, QSizePolicy::Fixed, QSizePolicy::Minimum);
    ui->eventsToolbarLayout->invalidate();
  }
  if (index == 2) {
    int profiling_column_width = ui->viewSplitter->width() - pos - 2;
    G_settings->setValue("prev_profiling_column_width", profiling_column_width);
  }
  updateHierarchyScrollbars();
}

static QPixmap recolorImageAndConvertToPixmap(QImage &image, QColor color) {
  recolorImage(image, color);
  return QPixmap::fromImage(image);
}

QIcon MainWindow::buildIcon(QString filename, bool is_toggle, int toolbar_icon_size, QColor normal_color, QColor disabled_color, QColor toggle_on_color, QColor toggle_off_color) {
  // Load image
  QImage image = QImage(filename);
  int tool_button_size = (int)(toolbar_icon_size * G_pixels_per_point);
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
  int num_files_loaded = G_event_tree_map.count();
  bool event_files_loaded = (num_files_loaded > 0);
  bool event_file_selected = eventFileSelected();
  Events *selected_events = NULL;
  EventTreeNode *events_row = eventRowSelected(&selected_events);
  bool events_to_the_left = false;
  bool events_to_the_right = false;
  if (events_row != NULL) {
    ui->eventsView->hasEventsOutsideOfVisibleRegion(selected_events, events_row, &events_to_the_left, &events_to_the_right);
  }
  bool folders_exist = eventFilesHaveFolders();
  bool threads_exist = eventFilesHaveThreads();
  int num_events_filtered = G_event_filters.count();
  bool font_size_can_grow = (G_font_point_size < G_max_font_point_size);
  bool font_size_can_shrink = (G_font_point_size > G_min_font_point_size);
  double percent_visible, percent_offset;
  ui->eventsView->getTimeRange(&percent_visible, &percent_offset);
  bool zoomed_to_all = (percent_visible == 1.0);

  // Hierarchy toolbar
  ui->closeAllButton->setEnabled(event_files_loaded);
  ui->closeSelectedButton->setEnabled(event_file_selected);
  ui->setFilterButton->setEnabled(event_files_loaded);
  ui->clearFilterButton->setEnabled(num_events_filtered > 0);
  ui->showFoldersButton->setEnabled(folders_exist);
  ui->showThreadsButton->setEnabled(threads_exist);
  ui->openFoldersButton->setEnabled(event_files_loaded);
  ui->closeFoldersButton->setEnabled(event_files_loaded);
  ui->sortByIdButton->setEnabled(event_files_loaded);
  ui->sortByNameButton->setEnabled(event_files_loaded);
  ui->sortByTimeButton->setEnabled(event_files_loaded);
  ui->increaseFontSizeButton->setEnabled(event_files_loaded && font_size_can_grow);
  ui->decreaseFontSizeButton->setEnabled(event_files_loaded && font_size_can_shrink);

  // Events toolbar
  ui->timeAlignButton->setEnabled(event_files_loaded && num_files_loaded > 1);
  ui->mouseModeInfoButton->setEnabled(event_files_loaded);
  ui->mouseModeHistogramButton->setEnabled(event_files_loaded);
  ui->mouseModeTimeShiftButton->setEnabled(event_files_loaded);
  ui->zoomToAllButton->setEnabled(event_files_loaded && !zoomed_to_all);
  ui->zoomInButton->setEnabled(event_files_loaded);
  ui->zoomOutButton->setEnabled(event_files_loaded && !zoomed_to_all);
  ui->zoomToSelectedButton->setEnabled(event_files_loaded && ui->eventsView->timeRangeSelected());
  ui->prevEventButton->setEnabled(event_files_loaded && events_to_the_left);
  ui->nextEventButton->setEnabled(event_files_loaded && events_to_the_right);

  // Update status bar
  QString alignment_mode = G_settings->value("alignment_mode", "Native").toString();  // One of "Native", "TimeZero", "EventId"
  QString message = "Event files loaded: " + QString::number(G_event_tree_map.count());
  message += "          Total Events: " + QString::number(totalEventInstances());
  if (alignment_mode == "Native") {
    message += "          Time Alignment: unmodified";
  } else if (alignment_mode == "TimeZero") {
    message += "          Time Alignment: start at zero";
  } else { // "EventId"
    bool is_start = G_settings->value("alignment_event_is_start", false).toBool();
    QString event_name = G_settings->value("alignment_event_name", "unset").toString();
    uint32_t instance_index = (uint32_t)G_settings->value("alignment_instance_index", 0).toInt();
    message += "          Time Alignment: '" + event_name + "' ";
    message += is_start ? "start ID, " : "end ID, ";
    message += "instance " + QString::number(instance_index);
  }
  if (num_events_filtered == 0) {
    message += "          Filters Set: none";
  } else if (num_events_filtered == 1) {
    message += "          Filters Set: " + QString::number(num_events_filtered) + " event";
  } else if (num_events_filtered > 1) {
    message += "          Filters Set: " + QString::number(num_events_filtered) + " events";
  }
  statusBar()->showMessage(message);
}

void MainWindow::on_loadButton_clicked() {
  QString prev_folder = G_settings->value("prev_load_folder", "").toString();
  QStringList files = QFileDialog::getOpenFileNames(this, "Load One Or More Event Files", prev_folder, "Event Files (*.events)");
  for (auto filename: files) {
    // Get folder
    QString folder = filename.section("/", -999, -2);
    //printf("folder: '%s'\n", folder.toLatin1().data());
    if (folder.isEmpty()) {
      QMessageBox::critical(this, "File Error", "No folder.");
      return;
    }
    // Get name of file
    QString name = filename.section("/", -1).section(".events", 0, 0);
    if (name.isEmpty()) {
      QMessageBox::critical(this, "File Error", "Empty filename.");
      return;
    }
    //printf("name: '%s'\n", name.toLatin1().data());
    G_settings->setValue("prev_load_folder", folder);

    // If events file already loaded, then delete old data first
    if (G_event_tree_map.contains(filename)) {
      // Remove old events file
      EventTree *old_tree = G_event_tree_map[filename];
      (void)/*int items_removed =*/G_event_tree_map.remove(filename);
      //assert(items_removed == 1);
      freeEvents(old_tree->events);
      delete old_tree;
    }

    // Load the events
    Events *events = loadEventsFile(filename.toLatin1().data());

    // Build the display tree
    EventTree *tree = new EventTree(events, name, folder, ui->showFoldersButton->isChecked(), ui->showThreadsButton->isChecked());
    tree->native_start_time = events->event_buffer[0].time;
    SortType sort_type = ui->sortByIdButton->isChecked() ? SORT_BY_ID : ui->sortByNameButton->isChecked() ? SORT_BY_NAME : SORT_BY_TIME;
    tree->sortTree(sort_type);
    G_event_tree_map[filename] = tree; // NOTE: QMaps are ordered alphabetically
  }
  if (files.count() > 0) {
    setWidgetUsability();
    updateHierarchyScrollbars();
    ui->eventsView->zoomToAll();
    updateViews();
    if (G_event_tree_map.count() > 1) {
      // Multiple files loaded, ask to time align
      G_settings->setValue("alignment_mode", "Native");  // One of "Native", "TimeZero", "EventId"
      on_timeAlignButton_clicked();
    }
  }
}

void MainWindow::on_timeAlignButton_clicked() {
  TimeAlignDialog dialog(this);
  dialog.adjustSize(); // Shrink to fit content

  // Setup signals
  this->connect(&dialog, SIGNAL(timeAlignmentChanged()), ui->eventsView, SLOT(updateTimeAlignment()));

  //if (dialog.exec() == QDialog::Accepted) {
  dialog.exec();
}

void MainWindow::on_closeAllButton_clicked() {
  freeAllEventFiles();
  setWidgetUsability();
  updateHierarchyScrollbars();
  ui->eventsView->zoomToAll();
  updateViews();
}

void MainWindow::on_closeSelectedButton_clicked() {
  QStringList delete_filenames;
  QMapIterator<QString, EventTree*> i(G_event_tree_map);
  while (i.hasNext()) {
    i.next();
    EventTree *tree = i.value();
    if (tree->tree->row_selected) {
      freeEvents(tree->events);
      delete tree;
      delete_filenames += i.key();
    }
  }
  for (auto filename: delete_filenames) {
    G_event_tree_map.remove(filename);
  }
  setWidgetUsability();
  updateHierarchyScrollbars();
  ui->eventsView->zoomToAll();
  updateViews();
  ui->eventsView->updateTimeAlignment();
}

void MainWindow::on_setFilterButton_clicked() {
  EventFilterDialog dialog(this);
  dialog.adjustSize(); // Shrink to fit content

  // Setup signals
  this->connect(&dialog, SIGNAL(eventFilterChanged()), this, SLOT(eventFiltersModified()));

  //if (dialog.exec() == QDialog::Accepted) {
  dialog.exec();
}

void MainWindow::on_clearFilterButton_clicked() {
  G_event_filters.clear();
  eventFiltersModified();
}

void MainWindow::eventFiltersModified() {
  updateViews();
  updateHierarchyScrollbars();
}

void MainWindow::updateEventTreeBuild() {
  SortType sort_type = ui->sortByIdButton->isChecked() ? SORT_BY_ID : ui->sortByNameButton->isChecked() ? SORT_BY_NAME : SORT_BY_TIME;
  QMapIterator<QString, EventTree*> i(G_event_tree_map);
  while (i.hasNext()) {
    // Get old tree info
    i.next();
    QString filename = i.key();
    EventTree *tree = i.value();
    Events *events = tree->events;
    uint64_t native_start_time = tree->native_start_time;
    QString name = tree->name;
    QString folder = tree->folder;
    delete tree;
    // Build new tree
    tree = new EventTree(events, name, folder, ui->showFoldersButton->isChecked(), ui->showThreadsButton->isChecked());
    tree->native_start_time = native_start_time;
    tree->sortTree(sort_type);
    G_event_tree_map[filename] = tree; // NOTE: QMaps are ordered alphabetically
  }
  updateHierarchyScrollbars();
  updateViews();
}

void MainWindow::freeAllEventFiles() {
  QMapIterator<QString, EventTree*> i(G_event_tree_map);
  while (i.hasNext()) {
    i.next();
    EventTree *tree = i.value();
    freeEvents(tree->events);
    delete tree;
  }
  G_event_tree_map.clear();
}

uint32_t MainWindow::totalEventInstances() {
  uint32_t count = 0;
  QMapIterator<QString, EventTree*> i(G_event_tree_map);
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
  QMapIterator<QString, EventTree*> i(G_event_tree_map);
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
  QMapIterator<QString, EventTree*> i(G_event_tree_map);
  while (i.hasNext()) {
    // Get old tree info
    i.next();
    EventTree *tree = i.value();
    Events *events = tree->events;
    if (events->is_threaded) return true;
  }
  return false;
}

bool MainWindow::eventFileSelected() {
  QMapIterator<QString, EventTree*> i(G_event_tree_map);
  while (i.hasNext()) {
    // Get old tree info
    i.next();
    EventTree *tree = i.value();
    if (tree->tree->row_selected) return true;
  }
  return false;
}

EventTreeNode *MainWindow::eventRowSelected(EventTreeNode *parent) {
  if (parent->row_selected && parent->tree_node_type == TREE_NODE_IS_EVENT) {
    return parent;
  }
  for (auto child: parent->children) {
    EventTreeNode *event_row = eventRowSelected(child);
    if (event_row != NULL) return event_row;
  }
  return NULL;
}

EventTreeNode *MainWindow::eventRowSelected(Events **selected_events_ret) {
  *selected_events_ret = NULL;
  QMapIterator<QString, EventTree*> i(G_event_tree_map);
  while (i.hasNext()) {
    // Get old tree info
    i.next();
    EventTree *tree = i.value();
    EventTreeNode *event_row = eventRowSelected(tree->tree);
    if (event_row != NULL) {
      *selected_events_ret = tree->events;
      return event_row;
    }
  }
  return NULL;
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
  QMapIterator<QString, EventTree*> i(G_event_tree_map);
  while (i.hasNext()) {
    // Get old tree info
    i.next();
    EventTree *tree = i.value();
    tree->openAllFolders();
  }
  updateHierarchyScrollbars();
  updateViews();
}

void MainWindow::on_closeFoldersButton_clicked() {
  QMapIterator<QString, EventTree*> i(G_event_tree_map);
  while (i.hasNext()) {
    // Get old tree info
    i.next();
    EventTree *tree = i.value();
    tree->closeAllFolders();
  }
  updateHierarchyScrollbars();
  updateViews();
}

void MainWindow::updateEventTreeSort() {
  SortType sort_type = ui->sortByIdButton->isChecked() ? SORT_BY_ID : ui->sortByNameButton->isChecked() ? SORT_BY_NAME : SORT_BY_TIME;
  QMapIterator<QString, EventTree*> i(G_event_tree_map);
  while (i.hasNext()) {
    i.next();
    EventTree *tree = i.value();
    tree->sortTree(sort_type);
  }
  updateHierarchyScrollbars();
  updateViews();
}

void MainWindow::on_sortByIdButton_clicked() {
  bool is_checked = ui->sortByIdButton->isChecked();
  if (!is_checked) {
    ui->sortByIdButton->setChecked(true);
  }
  ui->sortByNameButton->setChecked(false);
  ui->sortByTimeButton->setChecked(false);
  if (is_checked) {
    updateEventTreeSort();
  }
}

void MainWindow::on_sortByNameButton_clicked() {
  bool is_checked = ui->sortByNameButton->isChecked();
  if (!is_checked) {
    ui->sortByNameButton->setChecked(true);
  }
  ui->sortByIdButton->setChecked(false);
  ui->sortByTimeButton->setChecked(false);
  if (is_checked) {
    updateEventTreeSort();
  }
}

void MainWindow::on_sortByTimeButton_clicked() {
  bool is_checked = ui->sortByTimeButton->isChecked();
  if (!is_checked) {
    ui->sortByTimeButton->setChecked(true);
  }
  ui->sortByIdButton->setChecked(false);
  ui->sortByNameButton->setChecked(false);
  if (is_checked) {
    updateEventTreeSort();
  }
}

void MainWindow::on_mouseModeInfoButton_clicked() {
  bool is_checked = ui->mouseModeInfoButton->isChecked();
  if (!is_checked) {
    ui->mouseModeInfoButton->setChecked(true);
  }
  ui->mouseModeHistogramButton->setChecked(false);
  ui->mouseModeTimeShiftButton->setChecked(false);
  if (is_checked) {
    ui->eventsView->setMouseMode(EventsView::MOUSE_MODE_EVENT_INFO);
  }
}

void MainWindow::on_mouseModeHistogramButton_clicked() {
  bool is_checked = ui->mouseModeHistogramButton->isChecked();
  if (!is_checked) {
    ui->mouseModeHistogramButton->setChecked(true);
  }
  ui->mouseModeInfoButton->setChecked(false);
  ui->mouseModeTimeShiftButton->setChecked(false);
  if (is_checked) {
    ui->eventsView->setMouseMode(EventsView::MOUSE_MODE_EVENT_HISTOGRAM);
  }
}

void MainWindow::on_mouseModeTimeShiftButton_clicked() {
  bool is_checked = ui->mouseModeTimeShiftButton->isChecked();
  if (!is_checked) {
    ui->mouseModeTimeShiftButton->setChecked(true);
  }
  ui->mouseModeInfoButton->setChecked(false);
  ui->mouseModeHistogramButton->setChecked(false);
  if (is_checked) {
    ui->eventsView->setMouseMode(EventsView::MOUSE_MODE_TIME_SHIFT);
  }
}

void MainWindow::on_increaseFontSizeButton_clicked() {
  if (G_font_point_size < G_max_font_point_size) {
    G_font_point_size++;
    ui->hierarchyHeader->updateHeight();
    ui->eventsHeader->updateHeight();
    ui->profilingHeader->updateHeight();
    ui->hierarchyView->updateLineHeight();
    ui->eventsView->updateLineHeight();
    ui->profilingView->updateLineHeight();
    setWidgetUsability();
    updateHierarchyScrollbars();
    updateViews();
  }
}

void MainWindow::on_decreaseFontSizeButton_clicked() {
  if (G_font_point_size > G_min_font_point_size) {
    G_font_point_size--;
    ui->hierarchyHeader->updateHeight();
    ui->eventsHeader->updateHeight();
    ui->profilingHeader->updateHeight();
    ui->hierarchyView->updateLineHeight();
    ui->eventsView->updateLineHeight();
    ui->profilingView->updateLineHeight();
    setWidgetUsability();
    updateHierarchyScrollbars();
    updateViews();
  }
}

void MainWindow::updateHierarchyScrollbars() {
  int hierarchy_visible_w, hierarchy_actual_w, hierarchy_visible_h, hierarchy_actual_h;
  ui->hierarchyView->calculateGeometry(&hierarchy_visible_w, &hierarchy_actual_w, &hierarchy_visible_h, &hierarchy_actual_h);

  // Vertical scrollbar
  {
    int min = 0;
    int max = 0;
    int page_step = 1;
    int value = ui->hierarchyVScroll->value();
    if (hierarchy_visible_h < hierarchy_actual_h) {
      // Some stuff is hidden
      min = 0;
      max = hierarchy_actual_h - hierarchy_visible_h;
      page_step = hierarchy_visible_h;
    }
    if (value > max) value = max;
    ui->hierarchyVScroll->setRange(min, max);
    ui->hierarchyVScroll->setValue(value);
    ui->hierarchyVScroll->setPageStep(page_step);
    ui->hierarchyVScroll->setEnabled(max > 0);
  }

  // Horizontal scrollbar
  {
    int min = 0;
    int max = 0;
    int page_step = 1;
    int value = ui->hierarchyHScroll->value();
    if (hierarchy_visible_w < hierarchy_actual_w) {
      // Some stuff is hidden
      min = 0;
      max = hierarchy_actual_w - hierarchy_visible_w;
      page_step = hierarchy_visible_w;
    }
    if (value > max) value = max;
    ui->hierarchyHScroll->setRange(min, max);
    ui->hierarchyHScroll->setValue(value);
    ui->hierarchyHScroll->setPageStep(page_step);
    ui->hierarchyHScroll->setEnabled(max > 0);
  }
}

void MainWindow::updateEventsScrollRange() {
  double percent_visible, percent_offset;
  ui->eventsView->getTimeRange(&percent_visible, &percent_offset);

  {
    int min = 0;
    int max = 0;
    int page_step = 1;
    int single_step = 1;
    int actual_w = INT_MAX/2; // Can't use INT_MAX because the scroll handle is not drawn correctly
    int visible_w = (int)(percent_visible * actual_w);
    if (visible_w < actual_w) {
      // Some stuff is hidden
      min = 0;
      max = actual_w - visible_w;
      page_step = visible_w;
      single_step = page_step/9;
      if (single_step == 0) single_step = 1;
    }
    int value = (int)(percent_offset * actual_w);
    ui->eventsHScroll->setRange(min, max);
    ui->eventsHScroll->setValue(value);
    ui->eventsHScroll->setPageStep(page_step);
    ui->eventsHScroll->setSingleStep(single_step);
    ui->eventsHScroll->setEnabled(max > 0);
  }

  setWidgetUsability();
}

void MainWindow::updateEventsTimeOffset(int scroll_offset) {
  int max = ui->eventsHScroll->maximum();
  double percent_offset = 0.0;
  if (max > 0) {
    percent_offset = scroll_offset / (double)max;
  }
  ui->eventsView->updateTimeOffset(percent_offset);
  // NOTE: since the visible events may change, this may effect some of the tool buttons
  setWidgetUsability();
}

void MainWindow::on_prevEventButton_clicked() {
  Events *selected_events = NULL;
  EventTreeNode *events_row = eventRowSelected(&selected_events);
  ui->eventsView->centerPrevEvent(selected_events, events_row);
}

void MainWindow::on_nextEventButton_clicked() {
  Events *selected_events = NULL;
  EventTreeNode *events_row = eventRowSelected(&selected_events);
  ui->eventsView->centerNextEvent(selected_events, events_row);
}

void MainWindow::updateViews() {
  ui->hierarchyView->update();
  ui->eventsView->rebuildAndUpdate(); // This will send a signal to update the utilization column
}
