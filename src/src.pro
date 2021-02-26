# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

include($$PWD/../version.pri)
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
DEFINES += BUILD_ID=\\\"$$BUILD_ID\\\"

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
        apppermission.cpp \
        authenticationlistener.cpp \
        captiveportal/captiveportal.cpp \
        captiveportal/captiveportaldetection.cpp \
        captiveportal/captiveportaldetectionimpl.cpp \
        captiveportal/captiveportalmonitor.cpp \
        captiveportal/captiveportalnotifier.cpp \
        captiveportal/captiveportalrequest.cpp \
        closeeventhandler.cpp \
        command.cpp \
        commandlineparser.cpp \
        commands/commandactivate.cpp \
        commands/commanddeactivate.cpp \
        commands/commanddevice.cpp \
        commands/commandlogin.cpp \
        commands/commandlogout.cpp \
        commands/commandselect.cpp \
        commands/commandservers.cpp \
        commands/commandstatus.cpp \
        commands/commandui.cpp \
        connectioncheck.cpp \
        connectiondataholder.cpp \
        connectionhealth.cpp \
        controller.cpp \
        cryptosettings.cpp \
        curve25519.cpp \
        errorhandler.cpp \
        featurelist.cpp \
        fontloader.cpp \
        hacl-star/Hacl_Chacha20.c \
        hacl-star/Hacl_Chacha20Poly1305_32.c \
        hacl-star/Hacl_Curve25519_51.c \
        hacl-star/Hacl_Poly1305_32.c \
        ipaddress.cpp \
        ipaddressrange.cpp \
        leakdetector.cpp \
        localizer.cpp \
        logger.cpp \
        loghandler.cpp \
        logoutobserver.cpp \
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
        networkwatcher.cpp \
        notificationhandler.cpp \
        pinghelper.cpp \
        pingsender.cpp \
        platforms/dummy/dummyapplistprovider.cpp \
        qmlengineholder.cpp \
        releasemonitor.cpp \
        rfc1918.cpp \
        rfc4193.cpp \
        settingsholder.cpp \
        simplenetworkmanager.cpp \
        statusicon.cpp \
        systemtrayhandler.cpp \
        tasks/accountandservers/taskaccountandservers.cpp \
        tasks/adddevice/taskadddevice.cpp \
        tasks/authenticate/taskauthenticate.cpp \
        tasks/captiveportallookup/taskcaptiveportallookup.cpp \
        tasks/controlleraction/taskcontrolleraction.cpp \
        tasks/function/taskfunction.cpp \
        tasks/removedevice/taskremovedevice.cpp \
        timercontroller.cpp \
        timersingleshot.cpp \
        update/updater.cpp \
        update/versionapi.cpp \
        urlopener.cpp

HEADERS += \
        apppermission.h \
        applistprovider.h \
        authenticationlistener.h \
        captiveportal/captiveportal.h \
        captiveportal/captiveportaldetection.h \
        captiveportal/captiveportaldetectionimpl.h \
        captiveportal/captiveportalmonitor.h \
        captiveportal/captiveportalnotifier.h \
        captiveportal/captiveportalrequest.h \
        closeeventhandler.h \
        command.h \
        commandlineparser.h \
        commands/commandactivate.h \
        commands/commanddeactivate.h \
        commands/commanddevice.h \
        commands/commandlogin.h \
        commands/commandlogout.h \
        commands/commandselect.h \
        commands/commandservers.h \
        commands/commandstatus.h \
        commands/commandui.h \
        connectioncheck.h \
        connectiondataholder.h \
        connectionhealth.h \
        constants.h \
        controller.h \
        controllerimpl.h \
        cryptosettings.h \
        curve25519.h \
        errorhandler.h \
        featurelist.h \
        fontloader.h \
        ipaddress.h \
        ipaddressrange.h \
        leakdetector.h \
        localizer.h \
        logger.h \
        loghandler.h \
        logoutobserver.h \
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
        networkwatcher.h \
        networkwatcherimpl.h \
        notificationhandler.h \
        pinghelper.h \
        pingsender.h \
        pingsendworker.h \
        platforms/dummy/dummyapplistprovider.h \
        qmlengineholder.h \
        releasemonitor.h \
        rfc1918.h \
        rfc4193.h \
        settingsholder.h \
        simplenetworkmanager.h \
        statusicon.h \
        systemtrayhandler.h \
        task.h \
        tasks/accountandservers/taskaccountandservers.h \
        tasks/adddevice/taskadddevice.h \
        tasks/authenticate/taskauthenticate.h \
        tasks/captiveportallookup/taskcaptiveportallookup.h \
        tasks/controlleraction/taskcontrolleraction.h \
        tasks/function/taskfunction.h \
        tasks/removedevice/taskremovedevice.h \
        timercontroller.h \
        timersingleshot.h \
        update/updater.h \
        update/versionapi.h \
        urlopener.h

inspector {
    message(Enabling the inspector)

    QT+= websockets
    QT+= testlib
    QT.testlib.CONFIG -= console
    CONFIG += no_testcase_installs

    RESOURCES += inspector/inspector.qrc

    DEFINES += MVPN_INSPECTOR

    SOURCES += \
            inspector/inspectorhttpconnection.cpp \
            inspector/inspectorhttpserver.cpp \
            inspector/inspectorwebsocketconnection.cpp \
            inspector/inspectorwebsocketserver.cpp

    HEADERS += \
            inspector/inspectorhttpconnection.h \
            inspector/inspectorhttpserver.h \
            inspector/inspectorwebsocketconnection.h \
            inspector/inspectorwebsocketserver.h
}

# Signal handling for unix platforms
unix {
    SOURCES += signalhandler.cpp
    HEADERS += signalhandler.h
}

RESOURCES += qml.qrc

QML_IMPORT_PATH =
QML_DESIGNER_IMPORT_PATH =

production {
    message(Production build)
    DEFINES += MVPN_PRODUCTION_MODE
    RESOURCES += logo_prod.qrc
} else {
    message(Staging build)
    RESOURCES += logo_beta.qrc
}

balrog {
    message(Balrog enabled)
    DEFINES += MVPN_BALROG

    SOURCES += update/balrog.cpp
    HEADERS += update/balrog.h
}

DUMMY {
    message(Dummy build)

    win* {
      CONFIG += embed_manifest_exe
      QT += svg
    } else {
      QMAKE_CXXFLAGS *= -Werror
    }

    macos {
      TARGET = MozillaVPN
    } else {
      TARGET = mozillavpn
    }

    QT += networkauth

    DEFINES += MVPN_DUMMY

    SOURCES += \
            platforms/dummy/dummycontroller.cpp \
            platforms/dummy/dummycryptosettings.cpp \
            platforms/dummy/dummypingsendworker.cpp \
            systemtraynotificationhandler.cpp \
            tasks/authenticate/desktopauthenticationlistener.cpp

    HEADERS += \
            platforms/dummy/dummycontroller.h \
            platforms/dummy/dummypingsendworker.h \
            systemtraynotificationhandler.h \
            tasks/authenticate/desktopauthenticationlistener.h
}

# Platform-specific: Linux
else:linux:!android {
    message(Linux build)

    QMAKE_CXXFLAGS *= -Werror

    TARGET = mozillavpn
    QT += networkauth
    QT += dbus

    DEFINES += MVPN_LINUX
    DEFINES += PROTOCOL_VERSION=\\\"$$DBUS_PROTOCOL_VERSION\\\"

    SOURCES += \
            eventlistener.cpp \
            platforms/linux/backendlogsobserver.cpp \
            platforms/linux/dbusclient.cpp \
            platforms/linux/linuxcontroller.cpp \
            platforms/linux/linuxcryptosettings.cpp \
            platforms/linux/linuxdependencies.cpp \
            platforms/linux/linuxnetworkwatcher.cpp \
            platforms/linux/linuxpingsendworker.cpp \
            systemtraynotificationhandler.cpp \
            tasks/authenticate/desktopauthenticationlistener.cpp

    HEADERS += \
            eventlistener.h \
            platforms/linux/backendlogsobserver.h \
            platforms/linux/dbusclient.h \
            platforms/linux/linuxcontroller.h \
            platforms/linux/linuxdependencies.h \
            platforms/linux/linuxnetworkwatcher.h \
            platforms/linux/linuxpingsendworker.h \
            systemtraynotificationhandler.h \
            tasks/authenticate/desktopauthenticationlistener.h

    # The daemon source code:
    SOURCES += \
            ../3rdparty/wireguard-tools/contrib/embeddable-wg-library/wireguard.c \
            daemon/daemon.cpp \
            platforms/linux/daemon/dbusservice.cpp \
            platforms/linux/daemon/linuxdaemon.cpp \
            platforms/linux/daemon/polkithelper.cpp \
            platforms/linux/daemon/wgutilslinux.cpp \
            wgquickprocess.cpp

    HEADERS += \
            ../3rdparty/wireguard-tools/contrib/embeddable-wg-library/wireguard.h \
            daemon/daemon.h \
            daemon/wgutils.h \
            platforms/linux/daemon/dbusservice.h \
            platforms/linux/daemon/polkithelper.h \
            platforms/linux/daemon/wgutilslinux.h \
            wgquickprocess.h

    isEmpty(USRPATH) {
        USRPATH=/usr
    }
    isEmpty(ETCPATH) {
        ETCPATH=/etc
    }

    DBUS_ADAPTORS += platforms/linux/daemon/org.mozilla.vpn.dbus.xml
    DBUS_INTERFACES = platforms/linux/daemon/org.mozilla.vpn.dbus.xml

    target.path = $${USRPATH}/bin
    INSTALLS += target

    desktopFile.path = $${USRPATH}/share/applications
    desktopFile.files = ../linux/extra/MozillaVPN.desktop
    INSTALLS += desktopFile

    autostartFile.path = $${ETCPATH}/xdg/autostart
    autostartFile.files = ../linux/extra/MozillaVPN-startup.desktop
    INSTALLS += autostartFile

    icon16x16.path = $${USRPATH}/share/icons/hicolor/16x16/apps
    icon16x16.files = ../linux/extra/icons/16x16/mozillavpn.png
    INSTALLS += icon16x16

    icon32x32.path = $${USRPATH}/share/icons/hicolor/32x32/apps
    icon32x32.files = ../linux/extra/icons/32x32/mozillavpn.png
    INSTALLS += icon32x32

    icon48x48.path = $${USRPATH}/share/icons/hicolor/48x48/apps
    icon48x48.files = ../linux/extra/icons/48x48/mozillavpn.png
    INSTALLS += icon48x48

    icon64x64.path = $${USRPATH}/share/icons/hicolor/64x64/apps
    icon64x64.files = ../linux/extra/icons/64x64/mozillavpn.png
    INSTALLS += icon64x64

    icon128x128.path = $${USRPATH}/share/icons/hicolor/128x128/apps
    icon128x128.files = ../linux/extra/icons/128x128/mozillavpn.png
    INSTALLS += icon128x128

    polkit_actions.files = platforms/linux/daemon/org.mozilla.vpn.policy
    polkit_actions.path = $${USRPATH}/share/polkit-1/actions
    INSTALLS += polkit_actions

    dbus_conf.files = platforms/linux/daemon/org.mozilla.vpn.conf
    dbus_conf.path = $${USRPATH}/share/dbus-1/system.d/
    INSTALLS += dbus_conf

    dbus_service.files = platforms/linux/daemon/org.mozilla.vpn.dbus.service
    dbus_service.path = $${USRPATH}/share/dbus-1/system-services
    INSTALLS += dbus_service

    DEFINES += MVPN_DATA_PATH=\\\"$${USRPATH}/share/mozillavpn\\\"
    helper.path = $${USRPATH}/share/mozillavpn
    helper.files = ../linux/daemon/helper.sh
    INSTALLS += helper

    CONFIG += link_pkgconfig
    PKGCONFIG += polkit-gobject-1
}

# Platform-specific: android
else:android {
    message(Android build)

    QMAKE_CXXFLAGS *= -Werror
    # Android Deploy-to-Qt strips the info anyway
    # but we want to create an extra bundle with the info :)
    CONFIG += force_debug_info

    TARGET = mozillavpn
    QT += networkauth
    QT += svg
    QT += androidextras
    QT += qml
    QT += xml
    LIBS += \-ljnigraphics\

    DEFINES += MVPN_ANDROID

    ANDROID_ABIS = x86 x86_64 armeabi-v7a arm64-v8a

    INCLUDEPATH += platforms/android

    SOURCES +=  platforms/android/androidauthenticationlistener.cpp \
                platforms/android/androidcontroller.cpp \
                platforms/android/androidnotificationhandler.cpp \
                platforms/android/androidutils.cpp \
                platforms/android/androidwebview.cpp \
                platforms/android/androidstartatbootwatcher.cpp \
                platforms/android/androiddatamigration.cpp \
                platforms/android/androidappimageprovider.cpp \
                platforms/android/androidapplistprovider.cpp \
                platforms/android/androidsharedprefs.cpp \
                tasks/authenticate/desktopauthenticationlistener.cpp

    HEADERS +=  platforms/android/androidauthenticationlistener.h \
                platforms/android/androidcontroller.h \
                platforms/android/androidnotificationhandler.h \
                platforms/android/androidutils.h \
                platforms/android/androidwebview.h \
                platforms/android/androidstartatbootwatcher.h\
                platforms/android/androiddatamigration.h\
                platforms/android/androidappimageprovider.h \
                platforms/android/androidapplistprovider.h \
                platforms/android/androidsharedprefs.h \
                tasks/authenticate/desktopauthenticationlistener.h

    # Usable Linux Imports
    SOURCES += platforms/linux/linuxpingsendworker.cpp \
               platforms/linux/linuxcryptosettings.cpp

    HEADERS += platforms/linux/linuxpingsendworker.h

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
    LIBS += -framework CoreWLAN

    DEFINES += MVPN_MACOS

    SOURCES += \
            platforms/macos/macosmenubar.cpp \
            platforms/macos/macospingsendworker.cpp \
            platforms/macos/macosstartatbootwatcher.cpp \
            systemtraynotificationhandler.cpp \
            tasks/authenticate/desktopauthenticationlistener.cpp

    OBJECTIVE_SOURCES += \
            platforms/macos/macoscryptosettings.mm \
            platforms/macos/macosnetworkwatcher.mm \
            platforms/macos/macosutils.mm

    HEADERS += \
            platforms/macos/macosmenubar.h \
            platforms/macos/macospingsendworker.h \
            platforms/macos/macosstartatbootwatcher.h \
            systemtraynotificationhandler.h \
            tasks/authenticate/desktopauthenticationlistener.h

    OBJECTIVE_HEADERS += \
            platforms/macos/macosnetworkwatcher.h \
            platforms/macos/macosutils.h

    isEmpty(MVPN_MACOS) {
        message(No integration required for this build - let\'s use the dummy controller)

        SOURCES += platforms/dummy/dummycontroller.cpp
        HEADERS += platforms/dummy/dummycontroller.h
    } else:networkextension {
        message(Network extension mode)

        DEFINES += MVPN_MACOS_NETWORKEXTENSION

        INCLUDEPATH += \
                    ../3rdparty/Wireguard-apple/WireGuard/WireGuard/Crypto \
                    ../3rdparty/wireguard-apple/WireGuard/Shared/Model \

        OBJECTIVE_SOURCES += \
                platforms/ios/ioscontroller.mm \
                platforms/ios/iosglue.mm

        OBJECTIVE_HEADERS += \
                platforms/ios/iosscontroller.h
    } else {
        message(Daemon mode)

        DEFINES += MVPN_MACOS_DAEMON

        SOURCES += \
                   daemon/daemon.cpp \
                   daemon/daemonlocalserver.cpp \
                   daemon/daemonlocalserverconnection.cpp \
                   localsocketcontroller.cpp \
                   wgquickprocess.cpp \
                   platforms/macos/daemon/macosdaemon.cpp \
                   platforms/macos/daemon/macosdaemonserver.cpp
        HEADERS += \
                   daemon/daemon.h \
                   daemon/daemonlocalserver.h \
                   daemon/daemonlocalserverconnection.h \
                   daemon/wgutils.h \
                   localsocketcontroller.h \
                   wgquickprocess.h \
                   platforms/macos/daemon/macosdaemon.h \
                   platforms/macos/daemon/macosdaemonserver.h
    }

    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.14
    QMAKE_INFO_PLIST=../macos/app/Info.plist
    QMAKE_ASSET_CATALOGS_APP_ICON = "AppIcon"

    production {
        QMAKE_ASSET_CATALOGS = $$PWD/../macos/app/Images.xcassets
    } else {
        QMAKE_ASSET_CATALOGS = $$PWD/../macos/app/Images-beta.xcassets
    }
}

# Platform-specific: IOS
else:ios {
    message(IOS build)

    TARGET = MozillaVPN
    QMAKE_TARGET_BUNDLE_PREFIX = org.mozilla.ios
    QT += svg
    QT += gui-private

    # For the authentication
    LIBS += -framework AuthenticationServices

    # For notifications
    LIBS += -framework UIKit
    LIBS += -framework Foundation
    LIBS += -framework StoreKit
    LIBS += -framework UserNotifications

    DEFINES += MVPN_IOS

    SOURCES += \
            platforms/ios/taskiosproducts.cpp \
            platforms/macos/macospingsendworker.cpp

    OBJECTIVE_SOURCES += \
            platforms/ios/iaphandler.mm \
            platforms/ios/iosauthenticationlistener.mm \
            platforms/ios/ioscontroller.mm \
            platforms/ios/iosdatamigration.mm \
            platforms/ios/iosglue.mm \
            platforms/ios/iosnotificationhandler.mm \
            platforms/ios/iosutils.mm \
            platforms/macos/macoscryptosettings.mm

    HEADERS += \
            platforms/ios/taskiosproducts.h \
            platforms/macos/macospingsendworker.h

    OBJECTIVE_HEADERS += \
            platforms/ios/iaphandler.h \
            platforms/ios/iosauthenticationlistener.h \
            platforms/ios/ioscontroller.h \
            platforms/ios/iosdatamigration.h \
            platforms/ios/iosnotificationhandler.h \
            platforms/ios/iosutils.h

    QMAKE_INFO_PLIST= $$PWD/../ios/app/Info.plist
    QMAKE_ASSET_CATALOGS_APP_ICON = "AppIcon"

    production {
        QMAKE_ASSET_CATALOGS = $$PWD/../ios/app/Images.xcassets
    } else {
        QMAKE_ASSET_CATALOGS = $$PWD/../ios/app/Images-beta.xcassets
    }

    app_launch_screen.files = $$files($$PWD/../ios/app/MozillaVPNLaunchScreen.storyboard)
    QMAKE_BUNDLE_DATA += app_launch_screen

    ios_launch_screen_images.files = $$files($$PWD/../ios/app/launch.png)
    QMAKE_BUNDLE_DATA += ios_launch_screen_images
}

else:win* {
    message(Windows build)

    TARGET = MozillaVPN

    QT += networkauth
    QT += svg

    CONFIG += embed_manifest_exe
    DEFINES += MVPN_WINDOWS

    production {
        RC_ICONS = ui/resources/logo.ico
    } else {
        RC_ICONS = ui/resources/logo-beta.ico
    }

    SOURCES += \
        daemon/daemon.cpp \
        daemon/daemonlocalserver.cpp \
        daemon/daemonlocalserverconnection.cpp \
        eventlistener.cpp \
        localsocketcontroller.cpp \
        platforms/windows/daemon/windowsdaemon.cpp \
        platforms/windows/daemon/windowsdaemonserver.cpp \
        platforms/windows/daemon/windowsdaemontunnel.cpp \
        platforms/windows/daemon/windowstunnelmonitor.cpp \
        platforms/windows/windowscaptiveportaldetection.cpp \
        platforms/windows/windowscaptiveportaldetectionthread.cpp \
        platforms/windows/windowscommons.cpp \
        platforms/windows/windowscryptosettings.cpp \
        platforms/windows/windowsdatamigration.cpp \
        platforms/windows/windowsnetworkwatcher.cpp \
        platforms/windows/windowspingsendworker.cpp \
        platforms/windows/windowsstartatbootwatcher.cpp \
        tasks/authenticate/desktopauthenticationlistener.cpp \
        systemtraynotificationhandler.cpp \
        wgquickprocess.cpp

    HEADERS += \
        daemon/daemon.h \
        daemon/daemonlocalserver.h \
        daemon/daemonlocalserverconnection.h \
        daemon/wgutils.h \
        eventlistener.h \
        localsocketcontroller.h \
        platforms/windows/daemon/windowsdaemon.h \
        platforms/windows/daemon/windowsdaemonserver.h \
        platforms/windows/daemon/windowsdaemontunnel.h \
        platforms/windows/daemon/windowstunnelmonitor.h \
        platforms/windows/windowscaptiveportaldetection.h \
        platforms/windows/windowscaptiveportaldetectionthread.h \
        platforms/windows/windowscommons.h \
        platforms/windows/windowsdatamigration.h \
        platforms/windows/windowsnetworkwatcher.h \
        platforms/windows/windowspingsendworker.h \
        tasks/authenticate/desktopauthenticationlistener.h \
        platforms/windows/windowsstartatbootwatcher.h \
        systemtraynotificationhandler.h \
        wgquickprocess.h
}

else:wasm {
    message(WASM \\o/)
    DEFINES += MVPN_DUMMY
    DEFINES += MVPN_WASM

    QMAKE_CXXFLAGS *= -Werror

    TARGET = mozillavpn
    QT += svg

    SOURCES += \
            platforms/dummy/dummycontroller.cpp \
            platforms/dummy/dummycryptosettings.cpp \
            platforms/dummy/dummypingsendworker.cpp \
            platforms/macos/macosmenubar.cpp \
            platforms/wasm/wasmauthenticationlistener.cpp \
            platforms/wasm/wasmnetworkrequest.cpp \
            platforms/wasm/wasmnetworkwatcher.cpp \
            platforms/wasm/wasmwindowcontroller.cpp \
            systemtraynotificationhandler.cpp

    HEADERS += \
            platforms/dummy/dummycontroller.h \
            platforms/dummy/dummypingsendworker.h \
            platforms/macos/macosmenubar.h \
            platforms/wasm/wasmauthenticationlistener.h \
            platforms/wasm/wasmnetworkwatcher.h \
            platforms/wasm/wasmwindowcontroller.h \
            systemtraynotificationhandler.h

    SOURCES -= networkrequest.cpp
    RESOURCES += platforms/wasm/networkrequests.qrc
}

# Anything else
else {
    error(Unsupported platform)
}

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
