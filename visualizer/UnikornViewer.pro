# Copyright 2021 Michael Both
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

QT += widgets

RESOURCES += resources.qrc

DEPENDPATH  += src ../ref
INCLUDEPATH += src ../ref

win32:CONFIG += embed_manifest_exe
#win32:CONFIG += console
#+CONFIG += release
CONFIG += debug
CONFIG += warn_on

#+DEFINES += NAME_HERE

HEADERS += src/main.hpp
HEADERS += src/MainWindow.hpp
HEADERS += ../ref/events_loader.h

SOURCES += src/main.cpp
SOURCES += src/MainWindow.cpp
SOURCES += ../ref/events_loader.c

FORMS += dialogs/MainWindow.ui