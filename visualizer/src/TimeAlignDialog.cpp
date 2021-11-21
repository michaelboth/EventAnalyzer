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

#include "ui_TimeAlignDialog.h"
#include "TimeAlignDialog.hpp"
#include "main.hpp"

TimeAlignDialog::TimeAlignDialog(QWidget *parent) : QDialog(parent), ui(new Ui::TimeAlignDialog) {
  ui->setupUi(this);

  /*+ get events that can be aligned */
  /*+ see if event files recorded instances */
  /*+ see if any files have a non zero start time */

  this->connect(ui->noAlignmentRadio, SIGNAL(clicked()), this, SLOT(setWidgetUsability()));
  this->connect(ui->startFromZeroRadio, SIGNAL(clicked()), this, SLOT(setWidgetUsability()));
  this->connect(ui->alignByEventRadio, SIGNAL(clicked()), this, SLOT(setWidgetUsability()));

  setWidgetUsability();
}

TimeAlignDialog::~TimeAlignDialog() {
  // Nothing to do
}

void TimeAlignDialog::setWidgetUsability() {
  ui->alignToCommonFrame->setEnabled(ui->alignByEventRadio->isChecked());
  if (ui->noAlignmentRadio->isChecked()) {
    emit alignToNativeTime();
  } else if (ui->startFromZeroRadio->isChecked()) {
    emit alignToTimeZero();
  } else if (ui->alignByEventRadio->isChecked()) {
    /*+
      eventNameCombo
      instanceSpin
      drawAlignmentLinesCheck
    */
    //*+*/emit alignToEventInstance(uint16_t event_id, uint32_t instance);
  }
}

void TimeAlignDialog::on_doneButton_clicked() {
  accept();
}
