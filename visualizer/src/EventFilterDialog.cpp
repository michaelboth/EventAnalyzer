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
#include "ui_EventFilterDialog.h"
#include "EventFilterDialog.hpp"
#include "HelpfulFunctions.hpp"
#include "main.hpp"

EventFilterDialog::EventFilterDialog(QWidget *parent) : QDialog(parent), ui(new Ui::EventFilterDialog) {
  ui->setupUi(this);

  emit_fine_grain_signals = true;

  // Get unique event names
  QMapIterator<QString, EventTree*> i(G_event_tree_map);
  while (i.hasNext()) {
    i.next();
    EventTree *event_tree = i.value();
    Events *events = event_tree->events;
    for (uint16_t i=0; i<events->event_info_count; i++) {
      EventInfo *event_info = &events->event_info_list[i];
      QString name = event_info->name;
      if (!event_names.contains(name)) {
	event_names += name;
      }
    }
  }

  // Sort the names
  event_names.sort();

  // Fill in the event list
  for (QString name: event_names) {
    QListWidgetItem *item = new QListWidgetItem(name, ui->eventList);
    item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    bool is_filtered = G_event_filters.contains(name);
    item->setCheckState(is_filtered ? Qt::Unchecked : Qt::Checked);
  }

  this->connect(ui->eventList, SIGNAL(itemChanged(QListWidgetItem *)), this, SLOT(eventFilterChanged(QListWidgetItem *)));
}

EventFilterDialog::~EventFilterDialog() {
  // Nothing to do
}

void EventFilterDialog::eventFilterChanged(QListWidgetItem *item) {
  QString name = item->text();
  if (item->checkState() == Qt::Checked) {
    if (G_event_filters.contains(name)) {
      G_event_filters.removeAll(name);
    }
  } else {
    if (!G_event_filters.contains(name)) {
      G_event_filters += name;
    }
  }
  if (emit_fine_grain_signals) {
    emit eventFilterChanged();
  }
}

void EventFilterDialog::on_checkAllButton_clicked() {
  emit_fine_grain_signals = false;
  for (int i=0; i<ui->eventList->count(); i++) {
    QListWidgetItem *item = ui->eventList->item(i);
    item->setCheckState(Qt::Checked);
  }
  emit_fine_grain_signals = true;
  emit eventFilterChanged();
}

void EventFilterDialog::on_uncheckAllButton_clicked() {
  emit_fine_grain_signals = false;
  for (int i=0; i<ui->eventList->count(); i++) {
    QListWidgetItem *item = ui->eventList->item(i);
    item->setCheckState(Qt::Unchecked);
  }
  emit_fine_grain_signals = true;
  emit eventFilterChanged();
}

void EventFilterDialog::on_doneButton_clicked() {
  accept();
}
