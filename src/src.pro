# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

include($$PWD/../version.pri)
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
DEFINES += BUILD_ID=\\\"$$BUILD_ID\\\"

!isEmpty(MVPN_EXTRA_USERAGENT) {
    DEFINES += MVPN_EXTRA_USERAGENT=\\\"$$MVPN_EXTRA_USERAGENT\\\"
}

CCACHE_BIN = $$system(which ccache)
!isEmpty(CCACHE_BIN) {
    message(Using ccache)
    load(ccache)
    QMAKE_CXXFLAGS +=-g -fdebug-prefix-map=$(shell pwd)=.
}

QT += network
QT += quick
QT += widgets
QT += charts
QT += websockets
QT += sql

# For the inspector
QT+= testlib
QT.testlib.CONFIG -= console
CONFIG += no_testcase_installs

TEMPLATE  = app

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050F00

INCLUDEPATH += \
            hacl-star \
            hacl-star/kremlin \
            hacl-star/kremlin/minimal \
            ../translations/generated \
            ../glean \
            ../lottie/lib \
            ../nebula

include($$PWD/../glean/glean.pri)

include($$PWD/../nebula/nebula.pri)

!wasm{
    include($$PWD/crashreporter/crashreporter.pri)
}

message("Adding Lottie")
include($$PWD/../lottie/lottie.pri)
INCLUDEPATH += ../lottie/lib

DEPENDPATH  += $${INCLUDEPATH}

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc
UI_DIR = .ui

SOURCES += \
        apppermission.cpp \
        authenticationlistener.cpp \
        authenticationinapp/authenticationinapp.cpp \
        authenticationinapp/authenticationinapplistener.cpp \
        authenticationinapp/incrementaldecoder.cpp \
        captiveportal/captiveportal.cpp \
        captiveportal/captiveportaldetection.cpp \
        captiveportal/captiveportaldetectionimpl.cpp \
        captiveportal/captiveportalmonitor.cpp \
        captiveportal/captiveportalnotifier.cpp \
        captiveportal/captiveportalrequest.cpp \
        captiveportal/captiveportalrequesttask.cpp \
        closeeventhandler.cpp \
        collator.cpp \
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
        connectionbenchmark/connectionbenchmark.cpp \
        connectionbenchmark/connectionbenchmarkdownload.cpp \
        connectionbenchmark/connectionbenchmarkmodel.cpp \
        connectionbenchmark/filedownloader.cpp \
        connectioncheck.cpp \
        connectiondataholder.cpp \
        connectionhealth.cpp \
        constants.cpp \
        controller.cpp \
        cryptosettings.cpp \
        curve25519.cpp \
        dnshelper.cpp \
        errorhandler.cpp \
        featurelist.cpp \
        filterproxymodel.cpp \
        fontloader.cpp \
        hacl-star/Hacl_Chacha20.c \
        hacl-star/Hacl_Chacha20Poly1305_32.c \
        hacl-star/Hacl_Curve25519_51.c \
        hacl-star/Hacl_Poly1305_32.c \
        hawkauth.cpp \
        hkdf.cpp \
        iaphandler.cpp \
        imageproviderfactory.cpp \
        inspector/inspectorwebsocketconnection.cpp \
        inspector/inspectorwebsocketserver.cpp \
        ipaddress.cpp \
        l18nstringsimpl.cpp \
        leakdetector.cpp \
        localizer.cpp \
        logger.cpp \
        loghandler.cpp \
        logoutobserver.cpp \
        main.cpp \
        models/device.cpp \
        models/devicemodel.cpp \
        models/feature.cpp \
        models/feedbackcategorymodel.cpp \
        models/helpmodel.cpp \
        models/keys.cpp \
        models/licensemodel.cpp \
        models/server.cpp \
        models/servercity.cpp \
        models/servercountry.cpp \
        models/servercountrymodel.cpp \
        models/serverdata.cpp \
        models/supportcategorymodel.cpp \
        models/survey.cpp \
        models/surveymodel.cpp \
        models/user.cpp \
        models/whatsnewmodel.cpp \
        mozillavpn.cpp \
        networkmanager.cpp \
        networkrequest.cpp \
        networkwatcher.cpp \
        notificationhandler.cpp \
        pinghelper.cpp \
        pingsender.cpp \
        platforms/dummy/dummyapplistprovider.cpp \
        platforms/dummy/dummyiaphandler.cpp \
        platforms/dummy/dummynetworkwatcher.cpp \
        platforms/dummy/dummypingsender.cpp \
        qmlengineholder.cpp \
        releasemonitor.cpp \
        rfc/rfc1112.cpp \
        rfc/rfc1918.cpp \
        rfc/rfc4193.cpp \
        rfc/rfc4291.cpp \
        rfc/rfc5735.cpp \
        serveri18n.cpp \
        settingsholder.cpp \
        simplenetworkmanager.cpp \
        statusicon.cpp \
        tasks/account/taskaccount.cpp \
        tasks/adddevice/taskadddevice.cpp \
        tasks/authenticate/taskauthenticate.cpp \
        tasks/captiveportallookup/taskcaptiveportallookup.cpp \
        tasks/getfeaturelist/taskgetfeaturelist.cpp \
        tasks/controlleraction/taskcontrolleraction.cpp \
        tasks/createsupportticket/taskcreatesupportticket.cpp \
        tasks/function/taskfunction.cpp \
        tasks/group/taskgroup.cpp \
        tasks/heartbeat/taskheartbeat.cpp \
        tasks/ipfinder/taskipfinder.cpp \
        tasks/products/taskproducts.cpp \
        tasks/release/taskrelease.cpp \
        tasks/removedevice/taskremovedevice.cpp \
        tasks/sendfeedback/tasksendfeedback.cpp \
        tasks/servers/taskservers.cpp \
        tasks/surveydata/tasksurveydata.cpp \
        taskscheduler.cpp \
        theme.cpp \
        timersingleshot.cpp \
        update/updater.cpp \
        update/versionapi.cpp \
        urlopener.cpp

HEADERS += \
        appimageprovider.h \
        apppermission.h \
        applistprovider.h \
        authenticationlistener.h \
        authenticationinapp/authenticationinapp.h \
        authenticationinapp/authenticationinapplistener.h \
        authenticationinapp/incrementaldecoder.h \
        captiveportal/captiveportal.h \
        captiveportal/captiveportaldetection.h \
        captiveportal/captiveportaldetectionimpl.h \
        captiveportal/captiveportalmonitor.h \
        captiveportal/captiveportalnotifier.h \
        captiveportal/captiveportalrequest.h \
        captiveportal/captiveportalrequesttask.h \
        closeeventhandler.h \
        collator.h \
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
        connectionbenchmark/connectionbenchmark.h \
        connectionbenchmark/connectionbenchmarkdownload.h \
        connectionbenchmark/connectionbenchmarkmodel.h \
        connectionbenchmark/filedownloader.h \
        connectioncheck.h \
        connectiondataholder.h \
        connectionhealth.h \
        constants.h \
        controller.h \
        controllerimpl.h \
        cryptosettings.h \
        curve25519.h \
        dnshelper.h \
        errorhandler.h \
        featurelist.h \
        features/featureappreview.h \
        features/featurecaptiveportal.h \
        features/featureconnectioninfo.h \
        features/featurecustomdns.h \
        features/featureinappaccountcreate.h \
        features/featureinappauth.h \
        features/featureinapppurchase.h \
        features/featurelocalareaaccess.h \
        features/featuremultiaccountcontainers.h \
        features/featuremultihop.h \
        features/featurenotificationcontrol.h \
        features/featuresharelogs.h \
        features/featuresplittunnel.h \
        features/featurestartonboot.h \
        features/featureunsecurednetworknotification.h \
        features/featureserverunavailablenotification.h \
        filterproxymodel.h \
        fontloader.h \
        hawkauth.h \
        hkdf.h \
        iaphandler.h \
        imageproviderfactory.h \
        inspector/inspectorwebsocketconnection.h \
        inspector/inspectorwebsocketserver.h \
        ipaddress.h \
        leakdetector.h \
        localizer.h \
        logger.h \
        loghandler.h \
        logoutobserver.h \
        models/device.h \
        models/devicemodel.h \
        models/feature.h \
        models/feedbackcategorymodel.h \
        models/helpmodel.h \
        models/keys.h \
        models/licensemodel.h \
        models/server.h \
        models/servercity.h \
        models/servercountry.h \
        models/servercountrymodel.h \
        models/serverdata.h \
        models/supportcategorymodel.h \
        models/survey.h \
        models/surveymodel.h \
        models/user.h \
        models/whatsnewmodel.h \
        mozillavpn.h \
        networkmanager.h \
        networkrequest.h \
        networkwatcher.h \
        networkwatcherimpl.h \
        notificationhandler.h \
        pinghelper.h \
        pingsender.h \
        platforms/dummy/dummyapplistprovider.h \
        platforms/dummy/dummyiaphandler.h \
        platforms/dummy/dummynetworkwatcher.h \
        platforms/dummy/dummypingsender.h \
        qmlengineholder.h \
        releasemonitor.h \
        rfc/rfc1112.h \
        rfc/rfc1918.h \
        rfc/rfc4193.h \
        rfc/rfc4291.h \
        rfc/rfc5735.h \
        serveri18n.h \
        settingsholder.h \
        simplenetworkmanager.h \
        statusicon.h \
        task.h \
        tasks/account/taskaccount.h \
        tasks/adddevice/taskadddevice.h \
        tasks/authenticate/taskauthenticate.h \
        tasks/captiveportallookup/taskcaptiveportallookup.h \
        tasks/getfeaturelist/taskgetfeaturelist.h \
        tasks/controlleraction/taskcontrolleraction.h \
        tasks/createsupportticket/taskcreatesupportticket.h \
        tasks/function/taskfunction.h \
        tasks/group/taskgroup.h \
        tasks/heartbeat/taskheartbeat.h \
        tasks/ipfinder/taskipfinder.h \
        tasks/products/taskproducts.h \
        tasks/release/taskrelease.h \
        tasks/removedevice/taskremovedevice.h \
        tasks/sendfeedback/tasksendfeedback.h \
        tasks/servers/taskservers.h \
        tasks/surveydata/tasksurveydata.h \
        taskscheduler.h \
        theme.h \
        timersingleshot.h \
        update/updater.h \
        update/versionapi.h \
        urlopener.h

webextension {
    message(Enabling the webextension support)

    DEFINES += MVPN_WEBEXTENSION

    SOURCES += \
            server/serverconnection.cpp \
            server/serverhandler.cpp
    HEADERS += \
            server/serverconnection.h \
            server/serverhandler.h
}

# Signal handling for unix platforms
unix {
    SOURCES += signalhandler.cpp
    HEADERS += signalhandler.h
}

RESOURCES += ui/resources.qrc
RESOURCES += ui/license.qrc
RESOURCES += ui/ui.qrc
RESOURCES += resources/certs/certs.qrc

QML_IMPORT_PATH =
QML_DESIGNER_IMPORT_PATH =

balrog {
    message(Balrog enabled)
    DEFINES += MVPN_BALROG

    SOURCES += update/balrog.cpp
    HEADERS += update/balrog.h
}

DUMMY {
    message(Dummy build)

    CONFIG += c++1z

    win* {
      CONFIG += embed_manifest_exe
      QT += svg
    } else {
      versionAtLeast(QT_VERSION, 5.15.1) {
        QMAKE_CXXFLAGS *= -Werror
      }
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
            systemtraynotificationhandler.cpp \
            tasks/authenticate/desktopauthenticationlistener.cpp

    HEADERS += \
            platforms/dummy/dummycontroller.h \
            systemtraynotificationhandler.h \
            tasks/authenticate/desktopauthenticationlistener.h
}

# Platform-specific: Linux
else:linux:!android {
    message(Linux build)
    include($$PWD/golang.pri)

    TARGET = mozillavpn
    QT += networkauth
    QT += dbus

    system(c++ -lgo 2>&1 | grep "__go_init_main" > /dev/null) {
        LIBS += -lgo
    }

    CONFIG += c++14

    DEFINES += MVPN_LINUX
    DEFINES += PROTOCOL_VERSION=\\\"$$DBUS_PROTOCOL_VERSION\\\"

    SOURCES += \
            eventlistener.cpp \
            platforms/linux/backendlogsobserver.cpp \
            platforms/linux/dbusclient.cpp \
            platforms/linux/linuxappimageprovider.cpp \
            platforms/linux/linuxapplistprovider.cpp \
            platforms/linux/linuxcontroller.cpp \
            platforms/linux/linuxcryptosettings.cpp \
            platforms/linux/linuxdependencies.cpp \
            platforms/linux/linuxnetworkwatcher.cpp \
            platforms/linux/linuxnetworkwatcherworker.cpp \
            platforms/linux/linuxpingsender.cpp \
            platforms/linux/linuxsystemtraynotificationhandler.cpp \
            systemtraynotificationhandler.cpp \
            tasks/authenticate/desktopauthenticationlistener.cpp

    HEADERS += \
            eventlistener.h \
            platforms/linux/backendlogsobserver.h \
            platforms/linux/dbusclient.h \
            platforms/linux/linuxappimageprovider.h \
            platforms/linux/linuxapplistprovider.h \
            platforms/linux/linuxcontroller.h \
            platforms/linux/linuxdependencies.h \
            platforms/linux/linuxnetworkwatcher.h \
            platforms/linux/linuxnetworkwatcherworker.h \
            platforms/linux/linuxpingsender.h \
            platforms/linux/linuxsystemtraynotificationhandler.h \
            systemtraynotificationhandler.h \
            tasks/authenticate/desktopauthenticationlistener.h

    # The daemon source code:
    SOURCES += \
            ../3rdparty/wireguard-tools/contrib/embeddable-wg-library/wireguard.c \
            daemon/daemon.cpp \
            platforms/linux/daemon/apptracker.cpp \
            platforms/linux/daemon/dbusservice.cpp \
            platforms/linux/daemon/dnsutilslinux.cpp \
            platforms/linux/daemon/iputilslinux.cpp \
            platforms/linux/daemon/linuxdaemon.cpp \
            platforms/linux/daemon/pidtracker.cpp \
            platforms/linux/daemon/polkithelper.cpp \
            platforms/linux/daemon/wireguardutilslinux.cpp

    HEADERS += \
            ../3rdparty/wireguard-tools/contrib/embeddable-wg-library/wireguard.h \
            daemon/interfaceconfig.h \
            daemon/daemon.h \
            daemon/dnsutils.h \
            daemon/iputils.h \
            daemon/wireguardutils.h \
            platforms/linux/daemon/apptracker.h \
            platforms/linux/daemon/dbusservice.h \
            platforms/linux/daemon/dbustypeslinux.h \
            platforms/linux/daemon/dnsutilslinux.h \
            platforms/linux/daemon/iputilslinux.h \
            platforms/linux/daemon/pidtracker.h \
            platforms/linux/daemon/polkithelper.h \
            platforms/linux/daemon/wireguardutilslinux.h

    isEmpty(USRPATH) {
        USRPATH=/usr
    }
    isEmpty(ETCPATH) {
        ETCPATH=/etc
    }

    DBUS_ADAPTORS += platforms/linux/daemon/org.mozilla.vpn.dbus.xml
    DBUS_INTERFACES = platforms/linux/daemon/org.mozilla.vpn.dbus.xml

    GO_MODULES = ../linux/netfilter/netfilter.go

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

    DEFINES += MVPN_ICON_PATH=\\\"$${USRPATH}/share/icons/hicolor/64x64/apps/mozillavpn.png\\\"
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

    systemd_service.files = ../linux/mozillavpn.service
    systemd_service.path = $${USRPATH}/lib/systemd/system
    INSTALLS += systemd_service

    CONFIG += link_pkgconfig
    PKGCONFIG += polkit-gobject-1
}

# Platform-specific: android
else:android {
    message(Android build)

    adjust {
        message(Adjust SDK enabled)
        DEFINES += MVPN_ADJUST

        SOURCES += adjust/adjustfiltering.cpp \
                   adjust/adjusthandler.cpp \
                   adjust/adjustproxy.cpp \
                   adjust/adjustproxyconnection.cpp \
                   adjust/adjustproxypackagehandler.cpp \
                   adjust/adjusttasksubmission.cpp

        HEADERS += adjust/adjustfiltering.h \
                   adjust/adjusthandler.h \
                   adjust/adjustproxy.h \
                   adjust/adjustproxyconnection.h \
                   adjust/adjustproxypackagehandler.h \
                   adjust/adjusttasksubmission.h
    }

    versionAtLeast(QT_VERSION, 5.15.1) {
      QMAKE_CXXFLAGS *= -Werror
    }
    versionAtLeast(QT_VERSION, 6.0.0) {
        # We need to include qtprivate api's
        # As QAndroidBinder is not yet implemented with a public api
        QT+=core-private
    }


    # Android Deploy-to-Qt strips the info anyway
    # but we want to create an extra bundle with the info :)
    CONFIG += force_debug_info
    CONFIG += c++14

    TARGET = mozillavpn
    QT += networkauth
    QT += svg
    QT += qml
    QT += xml
    LIBS += \-ljnigraphics\

    !versionAtLeast(QT_VERSION, 6.0.0) {
        QT += androidextras
    }

    DEFINES += MVPN_ANDROID


    INCLUDEPATH += platforms/android

    SOURCES +=  platforms/android/androidauthenticationlistener.cpp \
                platforms/android/androidcontroller.cpp \
                platforms/android/androidiaphandler.cpp \
                platforms/android/androidnotificationhandler.cpp \
                platforms/android/androidutils.cpp \
                platforms/android/androidwebview.cpp \
                platforms/android/androidvpnactivity.cpp \
                platforms/android/androiddatamigration.cpp \
                platforms/android/androidappimageprovider.cpp \
                platforms/android/androidapplistprovider.cpp \
                platforms/android/androidsharedprefs.cpp \
                tasks/authenticate/desktopauthenticationlistener.cpp \
                tasks/purchase/taskpurchase.cpp

    HEADERS +=  platforms/android/androidauthenticationlistener.h \
                platforms/android/androidcontroller.h \
                platforms/android/androidiaphandler.h \
                platforms/android/androidnotificationhandler.h \
                platforms/android/androidutils.h \
                platforms/android/androidwebview.h \
                platforms/android/androidvpnactivity.h \
                platforms/android/androiddatamigration.h\
                platforms/android/androidappimageprovider.h \
                platforms/android/androidapplistprovider.h \
                platforms/android/androidsharedprefs.h \
                platforms/android/androidjnicompat.h \
                tasks/authenticate/desktopauthenticationlistener.h \
                tasks/purchase/taskpurchase.h


    # Usable Linux Imports
    SOURCES += platforms/linux/linuxpingsender.cpp \
               platforms/linux/linuxcryptosettings.cpp

    HEADERS += platforms/linux/linuxpingsender.h

    # We need to compile our own openssl :/
    exists(../3rdparty/openSSL/openssl.pri) {
       include(../3rdparty/openSSL/openssl.pri)
    } else{
       message(Have you imported the 3rd-party git submodules? Read the README.md)
       error(Did not found openSSL in 3rdparty/openSSL - Exiting Android Build)
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

    versionAtLeast(QT_VERSION, 5.15.1) {
      QMAKE_CXXFLAGS *= -Werror
    }

    TARGET = MozillaVPN
    QMAKE_TARGET_BUNDLE_PREFIX = org.mozilla.macos
    QT += networkauth
    QT += svg

    CONFIG += c++1z

    # For the loginitem
    LIBS += -framework ServiceManagement
    LIBS += -framework Security
    LIBS += -framework CoreWLAN

    DEFINES += MVPN_MACOS

    SOURCES += \
            platforms/macos/macosmenubar.cpp \
            platforms/macos/macospingsender.cpp \
            platforms/macos/macosstartatbootwatcher.cpp \
            systemtraynotificationhandler.cpp \
            tasks/authenticate/desktopauthenticationlistener.cpp

    OBJECTIVE_SOURCES += \
            platforms/macos/macoscryptosettings.mm \
            platforms/macos/macosnetworkwatcher.mm \
            platforms/macos/macosutils.mm

    HEADERS += \
            platforms/macos/macosmenubar.h \
            platforms/macos/macospingsender.h \
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
                   platforms/macos/daemon/dnsutilsmacos.cpp \
                   platforms/macos/daemon/iputilsmacos.cpp \
                   platforms/macos/daemon/macosdaemon.cpp \
                   platforms/macos/daemon/macosdaemonserver.cpp \
                   platforms/macos/daemon/macosroutemonitor.cpp \
                   platforms/macos/daemon/wireguardutilsmacos.cpp
        HEADERS += \
                   daemon/interfaceconfig.h \
                   daemon/daemon.h \
                   daemon/daemonlocalserver.h \
                   daemon/daemonlocalserverconnection.h \
                   daemon/dnsutils.h \
                   daemon/iputils.h \
                   daemon/wireguardutils.h \
                   localsocketcontroller.h \
                   wgquickprocess.h \
                   platforms/macos/daemon/dnsutilsmacos.h \
                   platforms/macos/daemon/iputilsmacos.h \
                   platforms/macos/daemon/macosdaemon.h \
                   platforms/macos/daemon/macosdaemonserver.h \
                   platforms/macos/daemon/macosroutemonitor.h \
                   platforms/macos/daemon/wireguardutilsmacos.h
    }

    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.14
    QMAKE_INFO_PLIST=../macos/app/Info.plist
    QMAKE_ASSET_CATALOGS_APP_ICON = "AppIcon"
    QMAKE_ASSET_CATALOGS = $$PWD/../macos/app/Images.xcassets
}

# Platform-specific: IOS
else:ios {
    message(IOS build)

    adjust {
        message(Adjust SDK enabled)
        DEFINES += MVPN_ADJUST

        SOURCES += adjust/adjustfiltering.cpp \
                   adjust/adjusthandler.cpp \
                   adjust/adjustproxy.cpp \
                   adjust/adjustproxyconnection.cpp \
                   adjust/adjustproxypackagehandler.cpp \
                   adjust/adjusttasksubmission.cpp

        OBJECTIVE_SOURCES += platforms/ios/iosadjusthelper.mm

        HEADERS += adjust/adjustfiltering.h \
                   adjust/adjusthandler.h \
                   adjust/adjustproxy.h \
                   adjust/adjustproxyconnection.h \
                   adjust/adjustproxypackagehandler.h \
                   adjust/adjusttasksubmission.h

        OBJECTIVE_HEADERS += platforms/ios/iosadjusthelper.h
    }

    TARGET = MozillaVPN
    QMAKE_TARGET_BUNDLE_PREFIX = org.mozilla.ios
    QT += svg
    QT += gui-private

    CONFIG += c++1z

    # For the authentication
    LIBS += -framework AuthenticationServices

    # For notifications
    LIBS += -framework UIKit
    LIBS += -framework Foundation
    LIBS += -framework StoreKit
    LIBS += -framework UserNotifications

    DEFINES += MVPN_IOS

    SOURCES += \
            platforms/macos/macospingsender.cpp \
            tasks/purchase/taskpurchase.cpp

    OBJECTIVE_SOURCES += \
            platforms/ios/iosiaphandler.mm \
            platforms/ios/iosauthenticationlistener.mm \
            platforms/ios/ioscontroller.mm \
            platforms/ios/iosdatamigration.mm \
            platforms/ios/iosglue.mm \
            platforms/ios/iosnotificationhandler.mm \
            platforms/ios/iosutils.mm \
            platforms/macos/macoscryptosettings.mm

    HEADERS += \
            platforms/macos/macospingsender.h \
            tasks/purchase/taskpurchase.h

    OBJECTIVE_HEADERS += \
            platforms/ios/iosiaphandler.h \
            platforms/ios/iosauthenticationlistener.h \
            platforms/ios/ioscontroller.h \
            platforms/ios/iosdatamigration.h \
            platforms/ios/iosnotificationhandler.h \
            platforms/ios/iosutils.h

    QMAKE_INFO_PLIST= $$PWD/../ios/app/Info.plist
    QMAKE_ASSET_CATALOGS_APP_ICON = "AppIcon"
    QMAKE_ASSET_CATALOGS = $$PWD/../ios/app/Images.xcassets

    app_launch_screen.files = $$files($$PWD/../ios/app/MozillaVPNLaunchScreen.storyboard)
    QMAKE_BUNDLE_DATA += app_launch_screen

    ios_launch_screen_images.files = $$files($$PWD/../ios/app/launch.png)
    QMAKE_BUNDLE_DATA += ios_launch_screen_images
}

else:win* {
    message(Windows build)

    TARGET = MozillaVPN

    CONFIG += c++1z
    QMAKE_CXXFLAGS += -MP -Zc:preprocessor

    CONFIG(debug, debug|release) {
        QMAKE_CXXFLAGS += /Z7 /ZI /FdMozillaVPN.PDB /DEBUG
        QMAKE_LFLAGS_WINDOWS += /DEBUG
    }

    QT += networkauth
    QT += svg
    QT += winextras

    CONFIG += embed_manifest_exe
    DEFINES += MVPN_WINDOWS
    DEFINES += WIN32_LEAN_AND_MEAN #Solves Redifinition Errors Of Winsock

    RC_ICONS = ui/resources/logo.ico

    SOURCES += \
        daemon/daemon.cpp \
        daemon/daemonlocalserver.cpp \
        daemon/daemonlocalserverconnection.cpp \
        eventlistener.cpp \
        localsocketcontroller.cpp \
        platforms/windows/windowsapplistprovider.cpp  \
        platforms/windows/windowsappimageprovider.cpp \
        platforms/windows/daemon/dnsutilswindows.cpp \
        platforms/windows/daemon/windowsdaemon.cpp \
        platforms/windows/daemon/windowsdaemonserver.cpp \
        platforms/windows/daemon/windowsdaemontunnel.cpp \
        platforms/windows/daemon/windowsroutemonitor.cpp \
        platforms/windows/daemon/windowstunnellogger.cpp \
        platforms/windows/daemon/windowstunnelservice.cpp \
        platforms/windows/daemon/wireguardutilswindows.cpp \
        platforms/windows/daemon/windowsfirewall.cpp \
        platforms/windows/daemon/windowssplittunnel.cpp \
        platforms/windows/windowsservicemanager.cpp \
        platforms/windows/daemon/windowssplittunnel.cpp \
        platforms/windows/windowscommons.cpp \
        platforms/windows/windowscryptosettings.cpp \
        platforms/windows/windowsdatamigration.cpp \
        platforms/windows/windowsnetworkwatcher.cpp \
        platforms/windows/windowspingsender.cpp \
        platforms/windows/windowsstartatbootwatcher.cpp \
        tasks/authenticate/desktopauthenticationlistener.cpp \
        systemtraynotificationhandler.cpp \
        wgquickprocess.cpp

    HEADERS += \
        daemon/interfaceconfig.h \
        daemon/daemon.h \
        daemon/daemonlocalserver.h \
        daemon/daemonlocalserverconnection.h \
        daemon/dnsutils.h \
        daemon/iputils.h \
        daemon/wireguardutils.h \
        eventlistener.h \
        localsocketcontroller.h \
        platforms/windows/windowsapplistprovider.h \
        platforms/windows/windowsappimageprovider.h \
        platforms/windows/daemon/dnsutilswindows.h \
        platforms/windows/daemon/windowsdaemon.h \
        platforms/windows/daemon/windowsdaemonserver.h \
        platforms/windows/daemon/windowsdaemontunnel.h \
        platforms/windows/daemon/windowsroutemonitor.h \
        platforms/windows/daemon/windowstunnellogger.h \
        platforms/windows/daemon/windowstunnelservice.h \
        platforms/windows/daemon/wireguardutilswindows.h \
        platforms/windows/daemon/windowsfirewall.h \
        platforms/windows/daemon/windowssplittunnel.h \
        platforms/windows/windowsservicemanager.h \
        platforms/windows/windowscommons.h \
        platforms/windows/windowsdatamigration.h \
        platforms/windows/windowsnetworkwatcher.h \
        platforms/windows/windowspingsender.h \
        tasks/authenticate/desktopauthenticationlistener.h \
        platforms/windows/windowsstartatbootwatcher.h \
        systemtraynotificationhandler.h \
        wgquickprocess.h
}

else:wasm {
    message(WASM \\o/)
    DEFINES += MVPN_DUMMY
    DEFINES += MVPN_WASM

    versionAtLeast(QT_VERSION, 5.15.1) {
      QMAKE_CXXFLAGS *= -Werror
    }

    TARGET = mozillavpn
    QT += svg
    # sql not available for wasm.
    QT -= sql

    CONFIG += c++1z

    # 32Mb
    QMAKE_WASM_TOTAL_MEMORY=33554432
    QMAKE_LFLAGS+= "-s TOTAL_MEMORY=33554432"

    SOURCES += \
            platforms/dummy/dummycontroller.cpp \
            platforms/dummy/dummycryptosettings.cpp \
            platforms/macos/macosmenubar.cpp \
            platforms/wasm/wasmauthenticationlistener.cpp \
            platforms/wasm/wasmnetworkrequest.cpp \
            platforms/wasm/wasmnetworkwatcher.cpp \
            platforms/wasm/wasmwindowcontroller.cpp \
            systemtraynotificationhandler.cpp

    HEADERS += \
            platforms/dummy/dummycontroller.h \
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

RESOURCES += $$PWD/../translations/servers.qrc

exists($$PWD/../translations/generated/l18nstrings.h) {
    SOURCES += $$PWD/../translations/generated/l18nstrings_p.cpp
    HEADERS += $$PWD/../translations/generated/l18nstrings.h
} else {
    error("No l18nstrings.h. Have you generated the strings?")
}

exists($$PWD/../translations/translations.pri) {
    include($$PWD/../translations/translations.pri)
} else {
    message(Languages were not imported - using fallback English)
    TRANSLATIONS += \
        ../translations/en/mozillavpn_en.ts

    ts.commands += lupdate $$PWD -no-obsolete -ts $$PWD/../translations/en/mozillavpn_en.ts
    ts.CONFIG += no_check_exist
    ts.output = $$PWD/../translations/en/mozillavpn_en.ts
    ts.input = .
    QMAKE_EXTRA_TARGETS += ts
    PRE_TARGETDEPS += ts
}

QMAKE_LRELEASE_FLAGS += -idbased
CONFIG += lrelease
CONFIG += embed_translations

coverage {
    message(Coverage enabled)
    QMAKE_CXXFLAGS += -fprofile-instr-generate -fcoverage-mapping
    QMAKE_LFLAGS += -fprofile-instr-generate -fcoverage-mapping
}

debug {
    # If in debug mode, set mvpn_debug flag too.
    CONFIG += mvpn_debug
}

mvpn_debug {
    message(MVPN Debug enabled)
    DEFINES += MVPN_DEBUG

    # This Flag will enable a qmljsdebugger on 0.0.0.0:1234
    CONFIG+=qml_debug

}
