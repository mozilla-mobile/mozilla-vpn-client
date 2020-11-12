# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

include($$PWD/../version.pri)
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
        captiveportal/captiveportal.cpp \
        captiveportal/captiveportalactivator.cpp \
        captiveportal/captiveportaldetection.cpp \
        captiveportal/captiveportalrequest.cpp \
        command.cpp \
        commandlineparser.cpp \
        commands/commandactivate.cpp \
        commands/commanddeactivate.cpp \
        commands/commandlogin.cpp \
        commands/commandlogout.cpp \
        commands/commandselect.cpp \
        commands/commandservers.cpp \
        commands/commandstatus.cpp \
        commands/commandui.cpp \
        connectiondataholder.cpp \
        connectionhealth.cpp \
        controller.cpp \
        cryptosettings.cpp \
        curve25519.cpp \
        errorhandler.cpp \
        fontloader.cpp \
        hacl-star/Hacl_Chacha20.c \
        hacl-star/Hacl_Chacha20Poly1305_32.c \
        hacl-star/Hacl_Curve25519_51.c \
        hacl-star/Hacl_Poly1305_32.c \
        localizer.cpp \
        logger.cpp \
        loghandler.cpp \
        main.cpp \
        models/helpmodel.cpp \
        models/user.cpp \
        models/device.cpp \
        models/devicemodel.cpp \
        models/keys.cpp \
        models/server.cpp \
        models/servercity.cpp \
        models/servercountry.cpp \
        models/servercountrymodel.cpp \
        models/serverdata.cpp \
        mozillavpn.cpp \
        networkmanager.cpp \
        networkrequest.cpp \
        pingsender.cpp \
        platforms/dummy/dummypingsendworker.cpp \
        qmlengineholder.cpp \
        releasemonitor.cpp \
        settingsholder.cpp \
        signalhandler.cpp \
        simplenetworkmanager.cpp \
        statusicon.cpp \
        systemtrayhandler.cpp \
        tasks/accountandservers/taskaccountandservers.cpp \
        tasks/adddevice/taskadddevice.cpp \
        tasks/authenticate/taskauthenticate.cpp \
        tasks/captiveportallookup/taskcaptiveportallookup.cpp \
        tasks/removedevice/taskremovedevice.cpp \
        timercontroller.cpp \
        timersingleshot.cpp

HEADERS += \
        captiveportal/captiveportal.h \
        captiveportal/captiveportalactivator.h \
        captiveportal/captiveportaldetection.h \
        captiveportal/captiveportalrequest.h \
        command.h \
        commandlineparser.h \
        commands/commandactivate.h \
        commands/commanddeactivate.h \
        commands/commandlogin.h \
        commands/commandlogout.h \
        commands/commandselect.h \
        commands/commandservers.h \
        commands/commandstatus.h \
        commands/commandui.h \
        connectiondataholder.h \
        connectionhealth.h \
        constants.h \
        controller.h \
        controllerimpl.h \
        cryptosettings.h \
        curve25519.h \
        errorhandler.h \
        fontloader.h \
        ipaddressrange.h \
        localizer.h \
        logger.h \
        loghandler.h \
        models/device.h \
        models/devicemodel.h \
        models/helpmodel.h \
        models/keys.h \
        models/server.h \
        models/servercity.h \
        models/servercountry.h \
        models/servercountrymodel.h \
        models/serverdata.h \
        models/user.h \
        mozillavpn.h \
        networkmanager.h \
        networkrequest.h \
        pingsender.h \
        pingsendworker.h \
        platforms/dummy/dummypingsendworker.h \
        qmlengineholder.h \
        releasemonitor.h \
        settingsholder.h \
        signalhandler.h \
        simplenetworkmanager.h \
        statusicon.h \
        systemtrayhandler.h \
        task.h \
        tasks/accountandservers/taskaccountandservers.h \
        tasks/adddevice/taskadddevice.h \
        tasks/authenticate/taskauthenticate.h \
        tasks/captiveportallookup/taskcaptiveportallookup.h \
        tasks/function/taskfunction.h \
        tasks/removedevice/taskremovedevice.h \
        timercontroller.h \
        timersingleshot.h

debug {
    QT+= testlib
    CONFIG += no_testcase_installs

    SOURCES += \
            inspector/inspectorconnection.cpp \
            inspector/inspectorserver.cpp

    HEADERS += \
            inspector/inspectorconnection.h \
            inspector/inspectorserver.h
}

# Platform-specific: Linux
linux:!android {
    message(Linux build)

    QMAKE_CXXFLAGS *= -Werror

    TARGET = mozillavpn
    QT += networkauth
    QT += svg

    SOURCES += \
            platforms/linux/backendlogsobserver.cpp \
            platforms/linux/dbus.cpp \
            platforms/linux/linuxcontroller.cpp \
            platforms/linux/linuxcryptosettings.cpp \
            platforms/linux/linuxdependencies.cpp \
            platforms/linux/linuxpingsendworker.cpp \
            tasks/authenticate/authenticationlistener.cpp

    HEADERS += \
            platforms/linux/backendlogsobserver.h \
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
    QT += svg
    QT += androidextras

    SOURCES += platforms/android/androidcontroller.cpp
    HEADERS += platforms/android/androidcontroller.h


    # Usable Linux Imports
    SOURCES += platforms/linux/linuxpingsendworker.cpp \
            tasks/authenticate/authenticationlistener.cpp\
            platforms/linux/linuxcryptosettings.cpp

    HEADERS +=platforms/linux/linuxpingsendworker.h \
            platforms/linux/linuxcryptosettings.h \
            tasks/authenticate/authenticationlistener.h

    # We need to compile our own openssl :/
    exists(../3rdparty/openSSL/openssl.pri) {
       include(../3rdparty/openSSL/openssl.pri)
    } else{
       message(Have you imported the 3rd-party git submodules? Read the README.md)
       error(Did not found openSSL in 3rdparty/openSSL - Exiting Android Build )
    }

    # For the android build we need to unset those
    # Otherwise the packaging will fail 🙅
    OBJECTS_DIR =
    MOC_DIR =
    RCC_DIR =
    UI_DIR =
    ANDROID_ABIS = x86 armeabi-v7a arm64-v8a

    DISTFILES += \
        ../android/AndroidManifest.xml \
        ../android/build.gradle \
        ../android/gradle/wrapper/gradle-wrapper.jar \
        ../android/gradle/wrapper/gradle-wrapper.properties \
        ../android/gradlew \
        ../android/gradlew.bat \
        ../android/res/values/libs.xml

    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/../android
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
    LIBS += -framework Security

    SOURCES += \
            platforms/macos/macosmenubar.cpp \
            platforms/macos/macospingsendworker.cpp \
            platforms/macos/macosstartatbootwatcher.cpp \
            tasks/authenticate/authenticationlistener.cpp

    OBJECTIVE_SOURCES += \
            platforms/macos/macoscryptosettings.mm \
            platforms/macos/macosglue.mm \
            platforms/macos/macosutils.mm

    HEADERS += \
            platforms/macos/macosmenubar.h \
            platforms/macos/macospingsendworker.h \
            platforms/macos/macosstartatbootwatcher.h \
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
                ../3rdparty/Wireguard-apple/WireGuard/WireGuard/Crypto \
                ../3rdparty/wireguard-apple/WireGuard/Shared/Model \

    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.14
    QMAKE_INFO_PLIST=../macos/app/Info.plist
    QMAKE_ASSET_CATALOGS = $$PWD/../macos/app/Images.xcassets
    QMAKE_ASSET_CATALOGS_APP_ICON = "AppIcon"
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

    SOURCES += \
            platforms/dummy/dummycontroller.cpp \
            platforms/ios/iaphandler.cpp \
            platforms/ios/taskiosproducts.cpp \
            platforms/macos/macospingsendworker.cpp

    OBJECTIVE_SOURCES += \
            platforms/ios/iosdatamigration.mm \
            platforms/ios/iosutils.mm \
            platforms/ios/authenticationlistener.mm \
            platforms/macos/macoscryptosettings.mm \
            platforms/macos/macosglue.mm \
            platforms/macos/macoscontroller.mm

    HEADERS += \
            platforms/dummy/dummycontroller.h \
            platforms/ios/iaphandler.h \
            platforms/ios/taskiosproducts.h \
            platforms/macos/macospingsendworker.h

    OBJECTIVE_HEADERS += \
            platforms/ios/iosdatamigration.h \
            platforms/ios/iosutils.h \
            platforms/ios/authenticationlistener.h \
            platforms/macos/macoscontroller.h

    QMAKE_INFO_PLIST= $$PWD/../ios/app/Info.plist
    QMAKE_ASSET_CATALOGS = $$PWD/../ios/app/Images.xcassets
    QMAKE_ASSET_CATALOGS_APP_ICON = "AppIcon"
}

# Anything else
else {
    error(Unsupported platform)
}

RESOURCES += qml.qrc

CONFIG += qmltypes
QML_IMPORT_NAME = Mozilla.VPN
QML_IMPORT_MAJOR_VERSION = 1

QML_IMPORT_PATH =
QML_DESIGNER_IMPORT_PATH =


exists($$PWD/../translations/translations.pri) {
    include($$PWD/../translations/translations.pri)
}
else{
    message(Languages were not imported - using fallback english)
    TRANSLATIONS += \
        ../translations/mozillavpn_en.ts

    ts.commands += lupdate $$PWD -no-obsolete -ts $$PWD/../translations/mozillavpn_en.ts
    ts.CONFIG += no_check_exist
    ts.output = $$PWD/../translations/mozillavpn_en.ts
    ts.input = .
    QMAKE_EXTRA_TARGETS += ts
    PRE_TARGETDEPS += ts
}


QMAKE_LRELEASE_FLAGS += -idbased
CONFIG += lrelease
CONFIG += embed_translations
