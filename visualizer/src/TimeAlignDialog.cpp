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

  ui->startFromZeroRadio->setEnabled(one_or_more_files_have_non_zero_start_time);
  ui->alignByEventRadio->setEnabled(all_files_have_instances && common_event_names.count() > 0);
  /*+*/ui->alignByEventRadio->setEnabled(false);
  if (all_files_have_instances && common_event_names.count() > 0) {
    ui->eventNameCombo->addItems(common_event_names);
  }
  bool prev_is_start = G_settings->value("alignment_event_is_start", false).toBool();
  ui->startEndCombo->setCurrentIndex(prev_is_start ? 0 : 1);
  bool draw_alignment_lines = G_settings->value("draw_alignment_lines", true).toBool();
  ui->drawAlignmentLinesCheck->setChecked(draw_alignment_lines);
  QString prev_event_name = G_settings->value("alignment_event_name", "unset").toString();
  ui->eventNameCombo->setCurrentText(prev_event_name);
  int prev_instance_index = G_settings->value("alignment_instance_index", 0).toInt();
  ui->instanceSpin->setMinimum(0);
  ui->instanceSpin->setMaximum(INT_MAX);
  ui->instanceSpin->setValue(prev_instance_index);

  // Set the initial alignment mode
  QString prev_alignment_mode = G_settings->value("alignment_mode", "Native").toString();  // One of "Native", "TimeZero", "EventId"
  if (prev_alignment_mode == "Native") {
    ui->noAlignmentRadio->setChecked(true);
  } else if (prev_alignment_mode == "TimeZero") {
    ui->startFromZeroRadio->setChecked(true);
  } else {
    ui->alignByEventRadio->setChecked(true);
  }

  this->connect(ui->noAlignmentRadio, SIGNAL(clicked()), this, SLOT(setWidgetUsability()));
  this->connect(ui->startFromZeroRadio, SIGNAL(clicked()), this, SLOT(setWidgetUsability()));
  this->connect(ui->alignByEventRadio, SIGNAL(clicked()), this, SLOT(setWidgetUsability()));
  this->connect(ui->startEndCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(eventInfoChanged(int)));
  this->connect(ui->eventNameCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(eventInfoChanged(int)));
  this->connect(ui->instanceSpin, SIGNAL(valueChanged(int)), this, SLOT(eventInfoChanged(int)));
  this->connect(ui->drawAlignmentLinesCheck, SIGNAL(toggled(bool)), this, SLOT(drawAlignmentLines(bool)));

  setWidgetUsability();
}

TimeAlignDialog::~TimeAlignDialog() {
  // Nothing to do
}

void TimeAlignDialog::drawAlignmentLines(bool draw_lines) {
  G_settings->setValue("draw_alignment_lines", draw_lines);
  /*+ emit rebuild events */
}

void TimeAlignDialog::eventInfoChanged(int /*index*/) {
  setWidgetUsability();
}

void TimeAlignDialog::setWidgetUsability() {
  ui->alignToCommonFrame->setEnabled(ui->alignByEventRadio->isChecked());
  if (ui->noAlignmentRadio->isChecked()) {
    G_settings->setValue("alignment_mode", "Native");  // One of "Native", "TimeZero", "EventId"
    /*+ really only need one signal since G_settings has all the info */
    emit alignToNativeTime();
  } else if (ui->startFromZeroRadio->isChecked()) {
    G_settings->setValue("alignment_mode", "TimeZero");  // One of "Native", "TimeZero", "EventId"
    emit alignToTimeZero();
  } else if (ui->alignByEventRadio->isChecked()) {
    bool is_start = (ui->startEndCombo->currentIndex() == 0);
    QString event_name = ui->eventNameCombo->currentText();

    /*+ get instance range
      for (uint32_t j=0; j<events->event_count; j++) {
      events->event_buffer[j].time -= first_time;
      }
    */

    int instance_index = ui->instanceSpin->value();
    //*+*/printf("name='%s', is_start=%s, instance_index=%d\n", event_name.toLatin1().data(), is_start ? "yes" : "no", instance_index);

    /*+ if no common instances, then just use first instance found in each file */
    /*+ save results to settings */
    G_settings->setValue("alignment_mode", "EventId");  // One of "Native", "TimeZero", "EventId"
    G_settings->setValue("alignment_event_name", event_name);
    G_settings->setValue("alignment_event_is_start", is_start);
    G_settings->setValue("alignment_instance_index", instance_index);
    emit alignToEventInstance(event_name, is_start, instance_index); /*+ also include time shift amount? */
  }
}

void TimeAlignDialog::on_doneButton_clicked() {
  accept();
}
