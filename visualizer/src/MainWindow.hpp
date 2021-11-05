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

#ifndef _MainWindow_hpp_
#define _MainWindow_hpp_

#include <QMainWindow>

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

protected:
  void closeEvent(QCloseEvent *event);

private slots:
  // Hierarchy tool bar
  void on_loadButton_clicked();
  void on_closeAllButton_clicked();
  void on_closeSelectedButton_clicked();
  void on_setFilterButton_clicked();
  void on_clearFilterButton_clicked();
  void on_showFoldersButton_clicked();
  void on_showThreadsButton_clicked();
  void on_openFoldersButton_clicked();
  void on_closeFoldersButton_clicked();
  void on_sortByIdButton_clicked();
  void on_sortByNameButton_clicked();
  void on_sortByTimeButton_clicked();
  void on_increaseFontSizeButton_clicked();
  void on_decreaseFontSizeButton_clicked();

private:
  Ui::MainWindow *ui;

  void setWidgetUsability();
  QPixmap recolorImage(QImage &image, QColor color);
  QIcon buildIcon(QString filename, QColor normal_color, QColor disabled_color, QColor active_color, QColor selected_color);
};

#endif