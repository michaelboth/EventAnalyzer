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

  /*+ remember what the time alignment mode was if user click on align button, otherwise from loaded files, always start with native time */
  bool all_files_have_instances = true;
  bool one_or_more_files_have_non_zero_start_time = false;
  {
    QMapIterator<QString, EventTree*> i(G_event_tree_map);
    bool got_initial_names = false;
    while (i.hasNext()) {
      i.next();
      EventTree *event_tree = i.value();
      Events *events = event_tree->events;

      if (!events->includes_instance) {
        all_files_have_instances = false;
        break;
      }

      uint64_t first_time = event_tree->native_start_time;
      if (first_time > 0) {
        one_or_more_files_have_non_zero_start_time = true;
      }

      if (!got_initial_names) {
        // Create initial name list
        got_initial_names = true;
        for (uint16_t i=0; i<events->event_info_count; i++) {
          EventInfo *event_info = &events->event_info_list[i];
          common_event_names += QString(event_info->name);
        }
      } else {
        // Remove names that are not common
        QStringList new_common_event_names;
        for (uint16_t i=0; i<events->event_info_count; i++) {
          EventInfo *event_info = &events->event_info_list[i];
          QString name = event_info->name;
          if (common_event_names.contains(name)) {
            new_common_event_names += name;
          }
        }
        common_event_names = new_common_event_names;
      }
    }
  }

  /*+ get instance ranges
    for (uint32_t j=0; j<events->event_count; j++) {
    events->event_buffer[j].time -= first_time;
    }
  */

  ui->startFromZeroRadio->setEnabled(one_or_more_files_have_non_zero_start_time);
  ui->alignByEventRadio->setEnabled(all_files_have_instances && common_event_names.count() > 0);
  if (all_files_have_instances && common_event_names.count() > 0) {
    ui->eventNameCombo->addItems(common_event_names);
  }

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
    /*+ if no common instances, then just use first instance found in each file */
    /*+
      eventNameCombo
      instanceSpin
      drawAlignmentLinesCheck
    */
    //*+*/emit alignToEventInstance(QString event_name, bool is_start, uint32_t instance);
  }
}

void TimeAlignDialog::on_doneButton_clicked() {
  accept();
}
