QT += network
QT += quick
QT += widgets

CONFIG += c++11

TEMPLATE  = app
TARGET    = mozillavpn

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        mozillavpn.cpp \
        task.cpp \
        taskauthenticate.cpp

HEADERS += \
        mozillavpn.h \
        task.h \
        taskauthenticate.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += qml.qrc

CONFIG += qmltypes
QML_IMPORT_NAME = Mozilla.VPN
QML_IMPORT_MAJOR_VERSION = 1

ICON = resources/icon.icns

QML_IMPORT_PATH =
QML_DESIGNER_IMPORT_PATH =

OBJECTS_DIR = .obj
MOC_DIR = .moc
