# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

message(Platform: linux)

TARGET = mozillavpn
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
isEmpty(LIBPATH) {
    LIBPATH=$${USRPATH}/lib
}
isEmpty(ETCPATH) {
    ETCPATH=/etc
}

DBUS_ADAPTORS += platforms/linux/daemon/org.mozilla.vpn.dbus.xml
DBUS_INTERFACES = platforms/linux/daemon/org.mozilla.vpn.dbus.xml

GO_MODULES = $$PWD/../../../linux/netfilter/netfilter.go

target.path = $${USRPATH}/bin
INSTALLS += target

desktopFile.files = $$PWD/../../../linux/extra/MozillaVPN.desktop
desktopFile.path = $${USRPATH}/share/applications
INSTALLS += desktopFile

autostartFile.files = $$PWD/../../../linux/extra/MozillaVPN-startup.desktop
autostartFile.path = $${ETCPATH}/xdg/autostart
INSTALLS += autostartFile

icon16x16.files = $$PWD/../../../linux/extra/icons/16x16/mozillavpn.png
icon16x16.path = $${USRPATH}/share/icons/hicolor/16x16/apps
INSTALLS += icon16x16

icon32x32.files = $$PWD/../../../linux/extra/icons/32x32/mozillavpn.png
icon32x32.path = $${USRPATH}/share/icons/hicolor/32x32/apps
INSTALLS += icon32x32

icon48x48.files = $$PWD/../../../linux/extra/icons/48x48/mozillavpn.png
icon48x48.path = $${USRPATH}/share/icons/hicolor/48x48/apps
INSTALLS += icon48x48

DEFINES += MVPN_ICON_PATH=\\\"$${USRPATH}/share/icons/hicolor/64x64/apps/mozillavpn.png\\\"
icon64x64.files = $$PWD/../../../linux/extra/icons/64x64/mozillavpn.png
icon64x64.path = $${USRPATH}/share/icons/hicolor/64x64/apps
INSTALLS += icon64x64

icon128x128.files = $$PWD/../../../linux/extra/icons/128x128/mozillavpn.png
icon128x128.path = $${USRPATH}/share/icons/hicolor/128x128/apps
INSTALLS += icon128x128

polkit_actions.path = $${USRPATH}/share/polkit-1/actions
polkit_actions.files = $$PWD/../../platforms/linux/daemon/org.mozilla.vpn.policy
INSTALLS += polkit_actions

dbus_conf.files = $$PWD/../../platforms/linux/daemon/org.mozilla.vpn.conf
dbus_conf.path = $${USRPATH}/share/dbus-1/system.d/
INSTALLS += dbus_conf

dbus_service.files = $$PWD/../../platforms/linux/daemon/org.mozilla.vpn.dbus.service
dbus_service.path = $${USRPATH}/share/dbus-1/system-services
INSTALLS += dbus_service

systemd_service.files = $$PWD/../../../linux/mozillavpn.service
systemd_service.path = $${USRPATH}/lib/systemd/system
INSTALLS += systemd_service

ORIG_MOZILLAVPN_JSON = $$PWD/../../../extension/manifests/linux/mozillavpn.json
manifestFile.input = ORIG_MOZILLAVPN_JSON
manifestFile.output = $$PWD/../../../linux/mozillavpn.json
manifestFile.commands = @python3 -c \'with open(\"$$ORIG_MOZILLAVPN_JSON\") as fin, open(\"$$manifestFile.output\", \"w\") as fout: [print(l.replace(\"/usr/lib/\", \"$${LIBPATH}/\"), end=\"\", file=fout) for l in fin]\'
manifestFile.CONFIG = target_predeps no_link
QMAKE_EXTRA_COMPILERS += manifestFile

manifestFirefox.files = $$manifestFile.output
manifestFirefox.path = $${LIBPATH}/mozilla/native-messaging-hosts
manifestFirefox.depends = $$manifestFile.output
manifestFirefox.CONFIG = no_check_exist
INSTALLS += manifestFirefox

manifestChrome.files = $$manifestFile.output
manifestChrome.path = $${ETCPATH}/opt/chrome/native-messaging-hosts
manifestChrome.depends = $$manifestFile.output
manifestChrome.CONFIG = no_check_exist
INSTALLS += manifestChrome

manifestChromium.files = $$manifestFile.output
manifestChromium.path = $${ETCPATH}/chromium/native-messaging-hosts
manifestChromium.depends = $$manifestFile.output
manifestChromium.CONFIG = no_check_exist
INSTALLS += manifestChromium

browserBridge.files = $$PWD/../../../extension/bridge/target/release/mozillavpnnp
browserBridge.path = $${LIBPATH}/mozillavpn
browserBridge.CONFIG = no_check_exist executable
INSTALLS += browserBridge

CONFIG += link_pkgconfig
PKGCONFIG += polkit-gobject-1
