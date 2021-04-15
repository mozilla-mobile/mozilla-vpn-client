# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

CONFIG += c++1z

TEMPLATE  = app
TARGET = mozillavpnnp

# No QT for this project
QT =

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc
UI_DIR = .ui

SOURCES += \
        handler.cpp \
        logger.cpp \
        main.cpp \
        vpnconnection.cpp

HEADERS += \
        handler.h \
        logger.h \
        vpnconnection.h

linux:!android {
    target.path = /usr/lib/mozillavpn
    INSTALLS += target

    manifest.path = /usr/lib/mozilla/native-messaging-hosts
    manifest.files = manifests/linux/mozillavpn.json
    INSTALLS += manifest

} else {
    error(Unsupported platform)
}
