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

DEPENDPATH  += $${INCLUDEPATH}

SOURCES += \
        controller.cpp \
        curve25519/curve25519.cpp \
        curve25519/curve25519_wg.c \
        device.cpp \
        devicemodel.cpp \
        keys.cpp \
        main.cpp \
        mozillavpn.cpp \
        networkrequest.cpp \
        platforms/linux/wgquickprocess.cpp \
        server.cpp \
        servercity.cpp \
        servercountry.cpp \
        servercountrymodel.cpp \
        serverdata.cpp \
        serversfetcher.cpp \
        tasks/account/taskaccount.cpp \
        tasks/adddevice/taskadddevice.cpp \
        tasks/authenticate/taskauthenticate.cpp \
        tasks/authenticate/taskauthenticationverifier.cpp \
        tasks/fetchservers/taskfetchservers.cpp \
        tasks/removedevice/taskremovedevice.cpp \
        user.cpp

HEADERS += \
        controller.h \
        controllerimpl.h \
        curve25519/curve25519.h \
        curve25519/curve25519_wg.h \
        device.h \
        devicemodel.h \
        keys.h \
        mozillavpn.h \
        networkrequest.h \
        platforms/linux/wgquickprocess.h \
        server.h \
        servercity.h \
        servercountry.h \
        servercountrymodel.h \
        serverdata.h \
        serversfetcher.h \
        task.h \
        tasks/account/taskaccount.h \
        tasks/adddevice/taskadddevice.h \
        tasks/authenticate/taskauthenticate.h \
        tasks/authenticate/taskauthenticationverifier.h \
        tasks/fetchservers/taskfetchservers.h \
        tasks/function/taskfunction.h \
        tasks/removedevice/taskremovedevice.h \
        user.h

linux-g++ {
     message(Linux build)
     SOURCES += \
             platforms/linux/linuxcontroller.cpp

     HEADERS += \
             platforms/linux/linuxcontroller.h
} else {
     message(Unknown build - Using the dummy controller)
     SOURCES += \
             platforms/dummy/dummycontroller.cpp

     HEADERS += \
             platforms/dummy/dummycontroller.h
}

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
