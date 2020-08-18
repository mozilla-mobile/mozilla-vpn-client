VERSION = 0.1
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

QT += network
QT += quick
QT += widgets

CONFIG += c++1z

unix {
    QMAKE_CXXFLAGS *= -Werror
}

TEMPLATE  = app
TARGET    = mozillavpn

DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += . \
               tasks/authenticate \
               tasks/adddevice \
               tasks/fetchservers \
               tasks/removedevice \
               tasks/function

DEPENDPATH  += $${INCLUDEPATH}

SOURCES += \
        devicedata.cpp \
        main.cpp \
        mozillavpn.cpp \
        networkrequest.cpp \
        servercountrymodel.cpp \
        serversfetcher.cpp \
        tasks/adddevice/taskadddevice.cpp \
        tasks/authenticate/taskauthenticate.cpp \
        tasks/authenticate/taskauthenticationverifier.cpp \
        tasks/fetchservers/taskfetchservers.cpp \
        tasks/removedevice/taskremovedevice.cpp \
        userdata.cpp \
        wireguardkeys.cpp

HEADERS += \
        devicedata.h \
        mozillavpn.h \
        networkrequest.h \
        servercountry.h \
        servercountrymodel.h \
        serversfetcher.h \
        task.h \
        tasks/adddevice/taskadddevice.h \
        tasks/authenticate/taskauthenticate.h \
        tasks/authenticate/taskauthenticationverifier.h \
        tasks/fetchservers/taskfetchservers.h \
        tasks/function/taskfunction.h \
        tasks/removedevice/taskremovedevice.h \
        userdata.h \
        wireguardkeys.h

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

# Let's store the mozillavpn_wg into the Contents/Resources
mozillavpn_wg.files = $$OUT_PWD/../vendors/wireguard-tools/src/mozillavpn_wg
mozillavpn_wg.path = Contents/Resources
QMAKE_BUNDLE_DATA += mozillavpn_wg

OBJECTS_DIR = .obj
MOC_DIR = .moc
