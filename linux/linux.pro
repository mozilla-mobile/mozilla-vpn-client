TEMPLATE  = app
TARGET    = mozillavpn-daemon

QT += dbus
DBUS_ADAPTORS += org.mozilla.vpn.dbus.xml

CONFIG += c++1z

QMAKE_CXXFLAGS *= -Werror

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES  += \
            dbus.cpp \
            main.cpp \
            polkithelper.cpp \
            ../src/platforms/linux/wgquickprocess.cpp

HEADERS += \
        dbus.h \
        polkithelper.h \
        ../src/platforms/linux/wgquickprocess.h

isEmpty(PREFIX) {
    PREFIX=/opt/$${TARGET}
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
