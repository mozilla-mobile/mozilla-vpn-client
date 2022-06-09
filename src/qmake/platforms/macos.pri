# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

message(Platform: macos)

QMAKE_CXXFLAGS *= -Werror

TARGET = "Mozilla VPN"

isEmpty(MVPN_DEVELOPMENT_TEAM) {
    MVPN_DEVELOPMENT_TEAM = "43AQ936H96"
}

isEmpty(MVPN_GROUP_ID_MACOS) {
    MVPN_GROUP_ID_MACOS = "group.org.mozilla.macos.Guardian"
}

isEmpty(MVPN_APP_ID_PREFIX) {
    QMAKE_TARGET_BUNDLE_PREFIX = org.mozilla.macos
} else {
    QMAKE_TARGET_BUNDLE_PREFIX = $$MVPN_APP_ID_PREFIX
}

QMAKE_BUNDLE = "FirefoxVPN"

CONFIG += c++1z

# For the loginitem
LIBS += -framework SystemConfiguration
LIBS += -framework ServiceManagement
LIBS += -framework Security
LIBS += -framework CoreWLAN

DEFINES += MVPN_MACOS

SOURCES += \
    daemon/daemon.cpp \
    daemon/daemonlocalserver.cpp \
    daemon/daemonlocalserverconnection.cpp \
    localsocketcontroller.cpp \
    platforms/macos/daemon/dnsutilsmacos.cpp \
    platforms/macos/daemon/iputilsmacos.cpp \
    platforms/macos/daemon/macosdaemon.cpp \
    platforms/macos/daemon/macosdaemonserver.cpp \
    platforms/macos/daemon/macosroutemonitor.cpp \
    platforms/macos/daemon/wireguardutilsmacos.cpp \
    platforms/macos/macosauthenticationlistener.cpp \
    platforms/macos/macosmenubar.cpp \
    platforms/macos/macospingsender.cpp \
    platforms/macos/macosstartatbootwatcher.cpp \
    systemtraynotificationhandler.cpp \
    tasks/authenticate/desktopauthenticationlistener.cpp \
    wgquickprocess.cpp

OBJECTIVE_SOURCES += \
    platforms/macos/macoscryptosettings.mm \
    platforms/macos/macosnetworkwatcher.mm \
    platforms/macos/macosutils.mm

HEADERS += \
    daemon/interfaceconfig.h \
    daemon/daemon.h \
    daemon/daemonlocalserver.h \
    daemon/daemonlocalserverconnection.h \
    daemon/dnsutils.h \
    daemon/iputils.h \
    daemon/wireguardutils.h \
    localsocketcontroller.h \
    platforms/macos/daemon/dnsutilsmacos.h \
    platforms/macos/daemon/iputilsmacos.h \
    platforms/macos/daemon/macosdaemon.h \
    platforms/macos/daemon/macosdaemonserver.h \
    platforms/macos/daemon/macosroutemonitor.h \
    platforms/macos/daemon/wireguardutilsmacos.h \
    platforms/macos/macosauthenticationlistener.h \
    platforms/macos/macosmenubar.h \
    platforms/macos/macospingsender.h \
    platforms/macos/macosstartatbootwatcher.h \
    systemtraynotificationhandler.h \
    tasks/authenticate/desktopauthenticationlistener.h \
    wgquickprocess.h

OBJECTIVE_HEADERS += \
    platforms/macos/macosnetworkwatcher.h \
    platforms/macos/macosutils.h

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.14
QMAKE_INFO_PLIST= $$PWD/../../../macos/app/Info.plist
QMAKE_ASSET_CATALOGS_APP_ICON = "AppIcon"
QMAKE_ASSET_CATALOGS = $$PWD/../../../macos/app/Images.xcassets

extension_bridge.files = $$PWD/../../../extension/bridge/target/release/mozillavpnnp
extension_bridge.path = 'Contents/Resources/utils'
QMAKE_BUNDLE_DATA += extension_bridge

extension_manifest.files = $$PWD/../../../extension/manifests/macos/mozillavpn.json
extension_manifest.path = 'Contents/Resources/utils'
QMAKE_BUNDLE_DATA += extension_manifest

wireguardGo.input = WIREGUARDGO
wireguardGo.output = ${QMAKE_FILE_IN}/wireguard-go
wireguardGo.commands = @echo Compiling Wireguard GO ${QMAKE_FILE_IN} && \
   cd "${QMAKE_FILE_IN}" && \
   make
wireguardGo.clean_commands = cd "${QMAKE_FILE_IN}" && make clean generate-version-and-build
wireguardGo.CONFIG = target_predeps no_link

QMAKE_EXTRA_COMPILERS += wireguardGo
WIREGUARDGO = $$PWD/../../../3rdparty/wireguard-go

wireguardGo_binary.files = $$PWD/../../../3rdparty/wireguard-go/wireguard-go
wireguardGo_binary.path = 'Contents/Resources/utils'
QMAKE_BUNDLE_DATA += wireguardGo_binary

LD_RUNPATH_SEARCH_PATHS.name = "LD_RUNPATH_SEARCH_PATHS"
LD_RUNPATH_SEARCH_PATHS.value = '"$(inherited) @executable_path/../Frameworks"'
QMAKE_MAC_XCODE_SETTINGS += LD_RUNPATH_SEARCH_PATHS

SWIFT_VERSION.name = "SWIFT_VERSION"
SWIFT_VERSION.value = "5.0"
QMAKE_MAC_XCODE_SETTINGS += SWIFT_VERSION

CLANG_ENABLE_MODULES.name = "CLANG_ENABLE_MODULES"
CLANG_ENABLE_MODULES.value = 'YES'
QMAKE_MAC_XCODE_SETTINGS += CLANG_ENABLE_MODULES

MARKETING_VERSION.name = "MARKETING_VERSION"
MARKETING_VERSION.value = $$VERSION
QMAKE_MAC_XCODE_SETTINGS += MARKETING_VERSION

CURRENT_PROJECT_VERSION.name = "CURRENT_PROJECT_VERSION"
CURRENT_PROJECT_VERSION.value = "$$system(echo "$$VERSION" | cut -d. -f1).$$system(date +"%Y%m%d%H%M")"
QMAKE_MAC_XCODE_SETTINGS += CURRENT_PROJECT_VERSION

PRODUCT_NAME.name = "PRODUCT_NAME"
PRODUCT_NAME.value = "Mozilla VPN"
QMAKE_MAC_XCODE_SETTINGS += PRODUCT_NAME

INFOPLIST_FILE.name = "INFOPLIST_FILE"
INFOPLIST_FILE.value = $$PWD/../../../macos/app/Info.plist
QMAKE_MAC_XCODE_SETTINGS += INFOPLIST_FILE

CODE_SIGN_ENTITLEMENTS.name = "CODE_SIGN_ENTITLEMENTS"
CODE_SIGN_ENTITLEMENTS.value = $$PWD/../../../macos/app/app.entitlements
QMAKE_MAC_XCODE_SETTINGS += CODE_SIGN_ENTITLEMENTS

CODE_SIGN_IDENTITY.name = "CODE_SIGN_IDENTITY"
CODE_SIGN_IDENTITY.value = 'Apple Development'
QMAKE_MAC_XCODE_SETTINGS += CODE_SIGN_IDENTITY

DEVELOPMENT_TEAM.name = "DEVELOPMENT_TEAM"
DEVELOPMENT_TEAM.value = "$$MVPN_DEVELOPMENT_TEAM"
QMAKE_MAC_XCODE_SETTINGS += DEVELOPMENT_TEAM

GROUP_ID_MACOS.name = "GROUP_ID_MACOS"
GROUP_ID_MACOS.value = "$$MVPN_GROUP_ID_MACOS"
QMAKE_MAC_XCODE_SETTINGS += GROUP_ID_MACOS

MVPN_APP_ID_MACOS = "$${QMAKE_TARGET_BUNDLE_PREFIX}.$${QMAKE_BUNDLE}"

APP_ID_MACOS.name = "APP_ID_MACOS"
APP_ID_MACOS.value = "$$MVPN_APP_ID_MACOS"
QMAKE_MAC_XCODE_SETTINGS += APP_ID_MACOS

SWIFT_OPTIMIZATION_LEVEL.name = "SWIFT_OPTIMIZATION_LEVEL"
SWIFT_OPTIMIZATION_LEVEL.value = "-Onone"
QMAKE_MAC_XCODE_SETTINGS += SWIFT_OPTIMIZATION_LEVEL

GCC_PREPROCESSOR_DEFINITIONS.name = "GCC_PREPROCESSOR_DEFINITIONS"
GCC_PREPROCESSOR_DEFINITIONS.value = 'GROUP_ID=\"$${MVPN_DEVELOPMENT_TEAM}.$${MVPN_APP_ID_MACOS}\"'
QMAKE_MAC_XCODE_SETTINGS += GCC_PREPROCESSOR_DEFINITIONS
