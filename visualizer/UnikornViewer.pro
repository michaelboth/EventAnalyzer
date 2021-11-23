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

DEPENDPATH  += src ../ref ../inc
INCLUDEPATH += src ../ref ../inc

win32:CONFIG += embed_manifest_exe
#win32:CONFIG += console
CONFIG += release
#CONFIG += debug
CONFIG += warn_on
#CONFIG += c99
CONFIG += c++17

HEADERS += src/main.hpp
HEADERS += src/MainWindow.hpp
HEADERS += src/EventTree.hpp
HEADERS += src/EventsHeader.hpp
HEADERS += src/EventsView.hpp
HEADERS += src/UtilizationView.hpp
HEADERS += src/HelpfulFunctions.hpp
HEADERS += src/GenericHeader.hpp
HEADERS += src/HierarchyView.hpp
HEADERS += src/TimeAlignDialog.hpp
HEADERS += src/EventFilterDialog.hpp
HEADERS += ../ref/events_loader.h
HEADERS += ../inc/unikorn.h

SOURCES += src/main.cpp
SOURCES += src/MainWindow.cpp
SOURCES += src/EventTree.cpp
SOURCES += src/EventsHeader.cpp
SOURCES += src/EventsView.cpp
SOURCES += src/UtilizationView.cpp
SOURCES += src/HelpfulFunctions.cpp
SOURCES += src/GenericHeader.cpp
SOURCES += src/HierarchyView.cpp
SOURCES += src/TimeAlignDialog.cpp
SOURCES += src/EventFilterDialog.cpp
SOURCES += ../ref/events_loader.c

FORMS += dialogs/MainWindow.ui
FORMS += dialogs/TimeAlignDialog.ui
FORMS += dialogs/EventFilterDialog.ui
