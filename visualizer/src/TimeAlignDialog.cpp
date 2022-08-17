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

#include <QMessageBox>
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
      UkEvents *events = event_tree->events;

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
          UkLoaderEventInfo *event_info = &events->event_info_list[i];
          common_event_names += QString(event_info->name);
        }
      } else {
        // Remove names that are not common
        QStringList new_common_event_names;
        for (uint16_t i=0; i<events->event_info_count; i++) {
          UkLoaderEventInfo *event_info = &events->event_info_list[i];
          QString name = event_info->name;
          if (common_event_names.contains(name)) {
            new_common_event_names += name;
          }
        }
        common_event_names = new_common_event_names;
      }
    }
    common_event_names.sort();
  }

  ui->startFromZeroRadio->setEnabled(one_or_more_files_have_non_zero_start_time);
  ui->alignByEventRadio->setEnabled(all_files_have_instances && common_event_names.count() > 0);
  if (all_files_have_instances && common_event_names.count() > 0) {
    ui->eventNameCombo->addItems(common_event_names);
  }
  bool prev_is_start = G_settings->value("alignment_event_is_start", false).toBool();
  ui->startEndCombo->setCurrentIndex(prev_is_start ? 0 : 1);
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

  setWidgetUsability();
}

TimeAlignDialog::~TimeAlignDialog() {
  // Nothing to do
}

void TimeAlignDialog::eventInfoChanged(int /*index*/) {
  setWidgetUsability();
}

static void getInstanceRange(UkEvents *events, uint16_t event_id, int *min_instance_ret, int *max_instance_ret) {
  uint32_t min_instance = 1, max_instance = 0;

  // Forward search for lowest instance
  for (uint32_t j=0; j<events->event_count; j++) {
    if (events->event_buffer[j].event_id == event_id) {
      min_instance = events->event_buffer[j].instance;
      break;
    }
  }

  // Reverse search for max instance
  for (uint32_t j=0; j<events->event_count; j++) {
    uint32_t j2 = events->event_count - 1 - j;
    if (events->event_buffer[j2].event_id == event_id) {
      max_instance = events->event_buffer[j2].instance;
      break;
    }
  }

  if (min_instance > INT_MAX || max_instance > INT_MAX) {
    // Out of range for a QSpinBox
    min_instance = 1;
    max_instance = 0;
  }

  *min_instance_ret = min_instance;
  *max_instance_ret = max_instance;
}

bool TimeAlignDialog::getCommonInstanceRange(QString event_name, bool is_start, int *min_instance_index_ret, int *max_instance_index_ret) {
  // NOTE: can only get here if all events file record instance and have the even name
  QMapIterator<QString, EventTree*> i(G_event_tree_map);
  bool got_initial_range = false;
  int min_instance = 1, max_instance = 0;
  while (i.hasNext()) {
    i.next();
    EventTree *event_tree = i.value();
    UkEvents *events = event_tree->events;

    // Get the event ID
    uint16_t event_id = 0;
    for (uint16_t j=0; j<events->event_info_count; j++) {
      UkLoaderEventInfo *event_info = &events->event_info_list[j];
      if (QString(event_info->name) == event_name) {
	event_id = is_start ? event_info->start_id : event_info->end_id;
	break;
      }
    }

    // Find the min and max instance
    if (!got_initial_range) {
      // Initial file
      getInstanceRange(events, event_id, &min_instance, &max_instance);
      if (max_instance < min_instance) return false;
      got_initial_range = true;
    } else {
      // Subsequent file
      int min_instance2 = 1, max_instance2 = 0;
      getInstanceRange(events, event_id, &min_instance2, &max_instance2);
      if (max_instance2 < min_instance2) return false;
      // Find intersection
      if (min_instance2 > min_instance) min_instance = min_instance2;
      if (max_instance2 < max_instance) max_instance = max_instance2;
      if (max_instance2 < min_instance2) return false;
    }
  }

  *min_instance_index_ret = min_instance;
  *max_instance_index_ret = max_instance;
  return true;
}

void TimeAlignDialog::setWidgetUsability() {
  ui->alignToCommonFrame->setEnabled(ui->alignByEventRadio->isChecked());
  if (ui->noAlignmentRadio->isChecked()) {
    G_settings->setValue("alignment_mode", "Native");  // One of "Native", "TimeZero", "EventId"
    emit timeAlignmentChanged();
  } else if (ui->startFromZeroRadio->isChecked()) {
    G_settings->setValue("alignment_mode", "TimeZero");  // One of "Native", "TimeZero", "EventId"
    emit timeAlignmentChanged();
  } else if (ui->alignByEventRadio->isChecked()) {
    bool is_start = (ui->startEndCombo->currentIndex() == 0);
    QString event_name = ui->eventNameCombo->currentText();
    int instance_index = ui->instanceSpin->value();
    int min_instance_index, max_instance_index;
    bool found_range = getCommonInstanceRange(event_name, is_start, &min_instance_index, &max_instance_index);
    if (!found_range) {
      QString message = "No common instances exist for the ";
      message += is_start ? "start" : "end";
      message += " event '" + event_name + "'. No alignments changed.";
      QMessageBox::warning(this, "Time Align", message);
      return;
    }
    ui->instanceSpin->setMinimum(min_instance_index);
    ui->instanceSpin->setMaximum(max_instance_index);
    if (instance_index < min_instance_index || instance_index > max_instance_index) {
      instance_index = min_instance_index;
    }
    ui->instanceSpin->setValue(instance_index);

    G_settings->setValue("alignment_mode", "EventId");  // One of "Native", "TimeZero", "EventId"
    G_settings->setValue("alignment_event_name", event_name);
    G_settings->setValue("alignment_event_is_start", is_start);
    G_settings->setValue("alignment_instance_index", instance_index);
    emit timeAlignmentChanged();
  }
}

void TimeAlignDialog::on_doneButton_clicked() {
  accept();
}
