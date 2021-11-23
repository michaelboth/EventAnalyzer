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

#ifndef _EventFilterDialog_hpp_
#define _EventFilterDialog_hpp_

#include <QDialog>
#include <QStringList>
#include <QListWidgetItem>

namespace Ui {
  class EventFilterDialog;
}

class EventFilterDialog : public QDialog {
  Q_OBJECT

public:
  explicit EventFilterDialog(QWidget *parent = 0);
  ~EventFilterDialog();

signals:
  void eventFilterChanged();

private slots:
  void on_checkAllButton_clicked();
  void on_uncheckAllButton_clicked();
  void on_doneButton_clicked();
  void eventFilterChanged(QListWidgetItem *item);

private:
  Ui::EventFilterDialog *ui;
  QStringList event_names;
  bool emit_fine_grain_signals;
};

#endif
