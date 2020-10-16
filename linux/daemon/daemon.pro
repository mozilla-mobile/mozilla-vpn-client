# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

include($$PWD/../../version.pri)

TEMPLATE  = app
TARGET    = mozillavpn-daemon

QT += dbus
DBUS_ADAPTORS += org.mozilla.vpn.dbus.xml

CONFIG += c++1z

QMAKE_CXXFLAGS *= -Werror

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

SOURCES += \
        ../../3rdparty/wireguard-tools/contrib/embeddable-wg-library/wireguard.c \
        ../../src/logger.cpp \
        ../../src/loghandler.cpp \
        ../../src/signalhandler.cpp \
        dbus.cpp \
        main.cpp \
        polkithelper.cpp \
        wgquickprocess.cpp

HEADERS += \
        ../../3rdparty/wireguard-tools/contrib/embeddable-wg-library/wireguard.h \
        ../../src/logger.h \
        ../../src/loghandler.h \
        ../../src/signalhandler.h \
        dbus.h \
        polkithelper.h \
        wgquickprocess.h

isEmpty(PREFIX) {
    PREFIX=/usr
}

target.path = $${PREFIX}/bin
INSTALLS += target

polkit_actions.files = org.mozilla.vpn.policy
polkit_actions.path = $${PREFIX}/share/polkit-1/actions
INSTALLS += polkit_actions

dbus_conf.files = org.mozilla.vpn.conf
dbus_conf.path = $${PREFIX}/share/dbus-1/system.d/
INSTALLS += dbus_conf

dbus_service.files = org.mozilla.vpn.dbus.service
dbus_service.path = $${PREFIX}/share/dbus-1/system-services
INSTALLS += dbus_service

CONFIG += link_pkgconfig
PKGCONFIG += polkit-gobject-1

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc
UI_DIR = .ui
