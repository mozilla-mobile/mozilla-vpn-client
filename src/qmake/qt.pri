# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

QT += network
QT += networkauth
QT += quick
QT += widgets
QT += websockets
QT += sql

!linux {
    QT += svg
}

versionAtLeast(QT_VERSION, 6.0.0) {
    message("Enable QT5 Compat")
    QT += core5compat
}

# For the inspector
QT+= testlib
QT.testlib.CONFIG -= console
CONFIG += no_testcase_installs

QML_IMPORT_PATH =
QML_DESIGNER_IMPORT_PATH =

OBJECTS_DIR = $$PWD/../.obj
MOC_DIR = $$PWD/../.moc
RCC_DIR = $$PWD/../.rcc
UI_DIR = $$PWD/../.ui
