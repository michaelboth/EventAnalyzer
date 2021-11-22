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

#ifndef _TimeAlignDialog_hpp_
#define _TimeAlignDialog_hpp_

#include <QDialog>
#include <QStringList>

namespace Ui {
  class TimeAlignDialog;
}

class TimeAlignDialog : public QDialog {
  Q_OBJECT

public:
  explicit TimeAlignDialog(QWidget *parent = 0);
  ~TimeAlignDialog();

signals:
  void alignToNativeTime();
  void alignToTimeZero();
  void alignToEventInstance(QString event_name, bool is_start, uint32_t instance);

private slots:
  void on_doneButton_clicked();
  void setWidgetUsability();
  void eventInfoChanged(int index);

private:
  Ui::TimeAlignDialog *ui;
  QStringList common_event_names;

  bool getCommonInstanceRange(QString event_name, bool is_start, int *min_instance_index_ret, int *max_instance_index_ret);
};

#endif
