# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

message(Platform: windows)

TARGET = MozillaVPN

versionAtLeast(QT_VERSION, 6.0.0) {
    versionAtLeast(QT_VERSION, 6.3.0) {
        # See https://mozilla-hub.atlassian.net/browse/VPN-1894
        error(Remove the qt6 windows hack!)
    }
    RESOURCES += $$PWD/../../ui/qt6winhack.qrc
}

CONFIG += c++1z
QMAKE_CXXFLAGS += -MP -Zc:preprocessor

CONFIG(debug, debug|release) {
    QMAKE_CXXFLAGS += /Z7 /ZI /FdMozillaVPN.PDB /DEBUG
    QMAKE_LFLAGS_WINDOWS += /DEBUG
}

CONFIG += embed_manifest_exe
DEFINES += MVPN_WINDOWS
DEFINES += WIN32_LEAN_AND_MEAN #Solves Redifinition Errors Of Winsock

RC_ICONS = $$PWD/../../ui/resources/logo.ico

SOURCES += \
    commands/commandcrashreporter.cpp \
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
    platforms/windows/windowsnetworkwatcher.cpp \
    platforms/windows/windowspingsender.cpp \
    platforms/windows/windowsstartatbootwatcher.cpp \
    tasks/authenticate/desktopauthenticationlistener.cpp \
    systemtraynotificationhandler.cpp \
    wgquickprocess.cpp

HEADERS += \
    commands/commandcrashreporter.h \
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
    platforms/windows/windowsnetworkwatcher.h \
    platforms/windows/windowspingsender.h \
    tasks/authenticate/desktopauthenticationlistener.h \
    platforms/windows/windowsstartatbootwatcher.h \
    systemtraynotificationhandler.h \
    wgquickprocess.h


# Installs for the /unsigned folder

mvpnExe.files = $$PWD/../../../mozillavpn.exe
mvpnExe.path = $$PWD/../../../unsigned/
mvpnExe.CONFIG = no_check_exist executable
INSTALLS += mvpnExe

MozillaVPNpdb.files = $$PWD/../../../MozillaVPN.pdb
MozillaVPNpdb.path = $$PWD/../../../unsigned/
MozillaVPNpdb.CONFIG = no_check_exist executable
INSTALLS += MozillaVPNpdb

libssl.files = $$PWD/../../../libssl-1_1-x64.dll
libssl.path = $$PWD/../../../unsigned/
INSTALLS += libssl

libcrypto.files = $$PWD/../../../libcrypto-1_1-x64.dll
libcrypto.path = $$PWD/../../../unsigned/
INSTALLS += libcrypto

VC142.files = $$PWD/../../../Microsoft_VC142_CRT_x64.msm
VC142.path = $$PWD/../../../unsigned/
INSTALLS += VC142

tunnel.files = $$PWD/../../../windows/tunnel/x64/tunnel.dll
tunnel.path = $$PWD/../../../unsigned/
tunnel.CONFIG = no_check_exist executable
INSTALLS += tunnel

wintun.files = $$PWD/../../../windows/tunnel/.deps/wintun/bin/amd64/wintun.dll
wintun.path = $$PWD/../../../unsigned/
wintun.CONFIG = no_check_exist executable
INSTALLS += wintun

balrog.files = $$PWD/../../../balrog/x64/balrog.dll
balrog.path = $$PWD/../../../unsigned/
balrog.CONFIG = no_check_exist executable
INSTALLS += balrog

splittunnelcat.files = $$PWD/../../../windows/split-tunnel/mullvad-split-tunnel.cat
splittunnelcat.path = $$PWD/../../../unsigned/
INSTALLS += splittunnelcat

splittunnelinf.files = $$PWD/../../../windows/split-tunnel/mullvad-split-tunnel.inf
splittunnelinf.path = $$PWD/../../../unsigned/
INSTALLS += splittunnelinf

splittunnelsys.files = $$PWD/../../../windows/split-tunnel/mullvad-split-tunnel.sys
splittunnelsys.path = $$PWD/../../../unsigned/
INSTALLS += splittunnelsys

WdfCoinstaller01011.files = $$PWD/../../../windows/split-tunnel/WdfCoinstaller01011.dll
WdfCoinstaller01011.path = $$PWD/../../../unsigned/
INSTALLS += WdfCoinstaller01011

mozillavpnjson.files = $$PWD/../../../extension/manifests/windows/mozillavpn.json
mozillavpnjson.path = $$PWD/../../../unsigned/
mozillavpnnp.CONFIG = no_check_exist executable
INSTALLS += mozillavpnjson

mozillavpnnp.files = $$PWD/../../../mozillavpnnp.exe
mozillavpnnp.path = $$PWD/../../../unsigned/
mozillavpnnp.CONFIG = no_check_exist executable
INSTALLS += mozillavpnnp
