!defined(VERSION, var):VERSION = 1
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

QT += network
QT += quick
QT += widgets
QT += charts

CONFIG += c++1z

TEMPLATE  = app

DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += \
            hacl-star \
            hacl-star/kremlin \
            hacl-star/kremlin/minimal

DEPENDPATH  += $${INCLUDEPATH}

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc
UI_DIR = .ui

SOURCES += \
        connectiondataholder.cpp \
        connectionhealth.cpp \
        controller.cpp \
        curve25519.cpp \
        device.cpp \
        devicemodel.cpp \
        errorhandler.cpp \
        hacl-star/Hacl_Curve25519_51.c \
        keys.cpp \
        localizer.cpp \
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
        settingsholder.cpp \
        signalhandler.cpp \
        systemtrayhandler.cpp \
        tasks/accountandservers/taskaccountandservers.cpp \
        tasks/adddevice/taskadddevice.cpp \
        tasks/authenticate/taskauthenticate.cpp \
        tasks/removedevice/taskremovedevice.cpp \
        timercontroller.cpp \
        user.cpp

HEADERS += \
        connectiondataholder.h \
        connectionhealth.h \
        controller.h \
        controllerimpl.h \
        curve25519.h \
        device.h \
        devicemodel.h \
        errorhandler.h \
        keys.h \
        localizer.h \
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
        settingsholder.h \
        signalhandler.h \
        systemtrayhandler.h \
        task.h \
        tasks/accountandservers/taskaccountandservers.h \
        tasks/adddevice/taskadddevice.h \
        tasks/authenticate/taskauthenticate.h \
        tasks/function/taskfunction.h \
        tasks/removedevice/taskremovedevice.h \
        timercontroller.h \
        user.h

# Platform-specific: Linux
linux:!android {
    message(Linux build)

    QMAKE_CXXFLAGS *= -Werror

    TARGET = mozillavpn
    QT += networkauth

    SOURCES += \
            platforms/linux/dbus.cpp \
            platforms/linux/linuxcontroller.cpp \
            platforms/linux/linuxdependencies.cpp \
            platforms/linux/linuxpingsendworker.cpp \
            tasks/authenticate/authenticationlistener.cpp

    HEADERS += \
            platforms/linux/dbus.h \
            platforms/linux/linuxcontroller.h \
            platforms/linux/linuxdependencies.h \
            platforms/linux/linuxpingsendworker.h \
            tasks/authenticate/authenticationlistener.h

    isEmpty(PREFIX) {
        PREFIX=/usr
    }

    QT += dbus
    DBUS_INTERFACES = ../linux/daemon/org.mozilla.vpn.dbus.xml

    target.path = $${PREFIX}/bin
    INSTALLS += target
}

else:android{
message(Android build)

QMAKE_CXXFLAGS *= -Werror

TARGET = mozillavpn
QT += networkauth

SOURCES += platforms/dummy/dummycontroller.cpp
HEADERS += platforms/dummy/dummycontroller.h

SOURCES += platforms/linux/linuxpingsendworker.cpp \
        tasks/authenticate/authenticationlistener.cpp

HEADERS +=platforms/linux/linuxpingsendworker.h \
        tasks/authenticate/authenticationlistener.h

include(platforms/android/openSSL/openssl.pri)


# For the android build we need to unset those
# Otherwise the packaging will fail 🙅‍
OBJECTS_DIR =
MOC_DIR =
RCC_DIR =
UI_DIR =
ANDROID_ABIS = x86
}

# Platform-specific: MacOS
else:macos {
    message(MacOSX build)

    QMAKE_CXXFLAGS *= -Werror

    TARGET = MozillaVPN
    QMAKE_TARGET_BUNDLE_PREFIX = org.mozilla.macos
    QT += networkauth

    # For the loginitem
    LIBS += -framework ServiceManagement

    SOURCES += \
            platforms/macos/macosglue.cpp \
            platforms/macos/macospingsendworker.cpp \
            tasks/authenticate/authenticationlistener.cpp

    OBJECTIVE_SOURCES += \
            platforms/macos/macosutils.mm

    HEADERS += \
            platforms/macos/macospingsendworker.h \
            tasks/authenticate/authenticationlistener.h

    OBJECTIVE_HEADERS += \
            platforms/macos/macosutils.h

    isEmpty(MACOS_INTEGRATION) {
        message(No integration required for this build - let\'s use the dummy controller)

        SOURCES += platforms/dummy/dummycontroller.cpp
        HEADERS += platforms/dummy/dummycontroller.h
    } else {
        message(Wireguard integration)

        DEFINES += MACOS_INTEGRATION

        OBJECTIVE_SOURCES += \
                platforms/macos/macoscontroller.mm

        OBJECTIVE_HEADERS += \
                platforms/macos/macoscontroller.h
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

    TARGET = MozillaVPN
    QMAKE_TARGET_BUNDLE_PREFIX = org.mozilla.ios
    QT += svg
    QT += gui-private
    QT += purchasing

    # For the authentication
    LIBS += -framework AuthenticationServices

    DEFINES += IOS_INTEGRATION

    message(No integration required for this build - let\'s use the dummy controller)

    SOURCES += \
            platforms/dummy/dummycontroller.cpp \
            platforms/ios/iaphandler.cpp \
            platforms/macos/macosglue.cpp \
            platforms/macos/macospingsendworker.cpp

    OBJECTIVE_SOURCES += \
            platforms/ios/iosutils.mm \
            platforms/ios/authenticationlistener.mm \
            platforms/macos/macoscontroller.mm

    HEADERS += \
            platforms/dummy/dummycontroller.h \
            platforms/ios/iaphandler.h \
            platforms/macos/macospingsendworker.h

    OBJECTIVE_HEADERS += \
            platforms/ios/iosutils.h \
            platforms/ios/authenticationlistener.h \
            platforms/macos/macoscontroller.h

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


TRANSLATIONS += \
    ../translations/mozillavpn_en.ts \
    ../translations/mozillavpn_it.ts \
    ../translations/mozillavpn_zh.ts

CONFIG += lrelease
CONFIG += embed_translations
