VERSION = 1
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

QT += network
QT += networkauth
QT += quick
QT += widgets

CONFIG += c++1z

TEMPLATE  = app

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
        releasemonitor.cpp \
        server.cpp \
        servercity.cpp \
        servercountry.cpp \
        servercountrymodel.cpp \
        serverdata.cpp \
        serversfetcher.cpp \
        signalhandler.cpp \
        systemtrayhandler.cpp \
        tasks/account/taskaccount.cpp \
        tasks/adddevice/taskadddevice.cpp \
        tasks/authenticate/authenticationlistener.cpp \
        tasks/authenticate/taskauthenticate.cpp \
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
        releasemonitor.h \
        server.h \
        servercity.h \
        servercountry.h \
        servercountrymodel.h \
        serverdata.h \
        serversfetcher.h \
        signalhandler.h \
        systemtrayhandler.h \
        task.h \
        tasks/account/taskaccount.h \
        tasks/adddevice/taskadddevice.h \
        tasks/authenticate/authenticationlistener.h \
        tasks/authenticate/taskauthenticate.h \
        tasks/fetchservers/taskfetchservers.h \
        tasks/function/taskfunction.h \
        tasks/removedevice/taskremovedevice.h \
        timercontroller.h \
        user.h

# Platform-specific: Linux
linux {
    message(Linux build)

    QMAKE_CXXFLAGS *= -Werror

    TARGET = mozillavpn

    SOURCES += \
            platforms/linux/linuxcontroller.cpp \
            platforms/linux/linuxpingsendworker.cpp \
            platforms/linux/wgquickdependencies.cpp \
            platforms/linux/wgquickprocess.cpp

    HEADERS += \
            platforms/linux/linuxcontroller.h \
            platforms/linux/linuxpingsendworker.h \
            platforms/linux/wgquickdependencies.h \
            platforms/linux/wgquickprocess.h

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

# Platform-specific: MacOS
else:macos {
    message(MacOSX build)

    QMAKE_CXXFLAGS *= -Werror

    TARGET = MozillaVPN
    QMAKE_TARGET_BUNDLE_PREFIX = org.mozilla.macos

    SOURCES += \
            platforms/macos/macosglue.cpp \
            platforms/macos/macospingsendworker.cpp \
            platforms/macos/macosutils.mm

    HEADERS += \
            platforms/macos/macospingsendworker.h \
            platforms/macos/macosutils.h

    isEmpty(MACOS_INTEGRATION) {
        message(No integration required for this build - let\'s use the dummy controller)

        SOURCES += platforms/dummy/dummycontroller.cpp
        HEADERS += platforms/dummy/dummycontroller.h
    } else {
        message(Wireguard integration)

        DEFINES += MACOS_INTEGRATION

        SOURCES += \
                platforms/macos/macoscontroller.cpp \
                platforms/macos/macosswiftcontroller.mm

        HEADERS += \
                platforms/macos/macoscontroller.h \
                platforms/macos/macosswiftcontroller.h
    }

    INCLUDEPATH += \
                ../wireguard-apple/WireGuard/WireGuard/Crypto \
                ../wireguard-apple/WireGuard/Shared/Model \

    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.14
    QMAKE_INFO_PLIST=../macos/Info.plist
}

# Platform-specific: IOS
else:ios {
    message(IOS build)

    TARGET = MozillaVPN_ios
    QMAKE_TARGET_BUNDLE_PREFIX = org.mozilla.ios
    QT += svg

    message(No integration required for this build - let\'s use the dummy controller)

    SOURCES += \
            platforms/ios/iosutils.mm \
            platforms/dummy/dummycontroller.cpp \
            platforms/macos/macospingsendworker.cpp

    HEADERS += \
            platforms/ios/iosutils.mm \
            platforms/dummy/dummycontroller.h \
            platforms/macos/macospingsendworker.h

    QMAKE_INFO_PLIST=../ios/Info.plist
}

# Anything else
else {
    error(Unsupported platform)
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
