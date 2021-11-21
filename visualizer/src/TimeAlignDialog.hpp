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
  void alignToEventInstance(uint16_t event_id, uint32_t instance);

private slots:
  void on_doneButton_clicked();
  /*+
  void on_noAlignmentRadio_clicked();
  void on_startFromZeroRadio_clicked();
  void on_alignByEventRadio_clicked();
  */
  void setWidgetUsability();

private:
  Ui::TimeAlignDialog *ui;
};

#endif
