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

#ifndef _GenericHeader_h_
#define _GenericHeader_h_
 
#include <QWidget>
#include <QPixmap>

class GenericHeader : public QWidget
{
  Q_OBJECT

public:
  GenericHeader(QWidget *parent=0);
  ~GenericHeader();
  void updateHeight();
  void setTitle(QString title);

protected:
  void paintEvent(QPaintEvent *event);

signals:

public slots:

private:
  QString title;
};

#endif
