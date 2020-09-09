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

INCLUDEPATH += \
            hacl-star \
            hacl-star/kremlin \
            hacl-star/kremlin/minimal

DEPENDPATH  += $${INCLUDEPATH}

SOURCES += \
        connectionhealth.cpp \
        controller.cpp \
        curve25519.cpp \
        device.cpp \
        devicemodel.cpp \
        hacl-star/Hacl_Curve25519_51.c \
        keys.cpp \
        logger.cpp \
        main.cpp \
        mozillavpn.cpp \
        networkrequest.cpp \
        pingsender.cpp \
        platforms/dummy/dummypingsendworker.cpp \
        platforms/linux/wgquickdependencies.cpp \
        releasemonitor.cpp \
        server.cpp \
        servercity.cpp \
        servercountry.cpp \
        servercountrymodel.cpp \
        serverdata.cpp \
        serversfetcher.cpp \
        signalhandler.cpp \
        tasks/account/taskaccount.cpp \
        tasks/adddevice/taskadddevice.cpp \
        tasks/authenticate/taskauthenticate.cpp \
        tasks/authenticate/taskauthenticationverifier.cpp \
        tasks/fetchservers/taskfetchservers.cpp \
        tasks/removedevice/taskremovedevice.cpp \
        timercontroller.cpp \
        user.cpp

HEADERS += \
        connectionhealth.h \
        controller.h \
        controllerimpl.h \
        curve25519.h \
        device.h \
        devicemodel.h \
        keys.h \
        logger.h \
        mozillavpn.h \
        networkrequest.h \
        pingsender.h \
        pingsendworker.h \
        platforms/dummy/dummypingsendworker.h \
        platforms/linux/wgquickdependencies.h \
        releasemonitor.h \
        server.h \
        servercity.h \
        servercountry.h \
        servercountrymodel.h \
        serverdata.h \
        serversfetcher.h \
        signalhandler.h \
        task.h \
        tasks/account/taskaccount.h \
        tasks/adddevice/taskadddevice.h \
        tasks/authenticate/taskauthenticate.h \
        tasks/authenticate/taskauthenticationverifier.h \
        tasks/fetchservers/taskfetchservers.h \
        tasks/function/taskfunction.h \
        tasks/removedevice/taskremovedevice.h \
        timercontroller.h \
        user.h

# Platform-specific controller
linux {
     message(Linux build - controller)
     SOURCES += \
             platforms/linux/linuxcontroller.cpp \
             platforms/linux/wgquickprocess.cpp

     HEADERS += \
             platforms/linux/linuxcontroller.h \
             platforms/linux/wgquickprocess.h
} else {
     message(Unknown build - Using the dummy controller)
     SOURCES += \
             platforms/dummy/dummycontroller.cpp

     HEADERS += \
             platforms/dummy/dummycontroller.h
}

# Platform-specific ping sender
linux {
     message(Linux build - ping)
     SOURCES += \
             platforms/linux/linuxpingsendworker.cpp

     HEADERS += \
             platforms/linux/linuxpingsendworker.h
} else:macos {
     message(MacOSX build - ping)
     SOURCES += \
             platforms/macos/macospingsendworker.cpp

     HEADERS += \
             platforms/macos/macospingsendworker.h
} else {
     message(Unknown build - dummy ping)
}

# Other platform-specific utils
macos {
    SOURCES += \
            platforms/macos/macosutils.mm

    HEADERS += \
            platforms/macos/macosutils.h
}

linux {
    isEmpty(PREFIX) {
        PREFIX=/opt/$${TARGET}
    }

    isEmpty(NO_POLKIT) {
        message(Use polkit)
        DEFINES += USE_POLKIT

        QT += dbus
        DBUS_INTERFACES = ../linux/org.mozilla.vpn.dbus.xml

        SOURCES += \
        platforms/linux/dbus.cpp

        HEADERS += \
        platforms/linux/dbus.h
    }

    target.path = $${PREFIX}/bin
    INSTALLS += target
}

RESOURCES += qml.qrc

CONFIG += qmltypes
QML_IMPORT_NAME = Mozilla.VPN
QML_IMPORT_MAJOR_VERSION = 1

ICON = resources/icon.icns

QML_IMPORT_PATH =
QML_DESIGNER_IMPORT_PATH =

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc
UI_DIR = .ui
