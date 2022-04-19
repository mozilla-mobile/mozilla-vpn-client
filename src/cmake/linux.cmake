find_package(Qt6 REQUIRED COMPONENTS DBus)
target_link_libraries(mozillavpn PRIVATE Qt6::DBus)

find_package(PkgConfig REQUIRED)
pkg_check_modules(polkit REQUIRED IMPORTED_TARGET polkit-gobject-1)
target_link_libraries(mozillavpn PRIVATE PkgConfig::polkit)

# Linux platform source files
target_sources(mozillavpn PRIVATE
    eventlistener.cpp
    platforms/linux/backendlogsobserver.cpp
    platforms/linux/dbusclient.cpp
    platforms/linux/linuxappimageprovider.cpp
    platforms/linux/linuxapplistprovider.cpp
    platforms/linux/linuxcontroller.cpp
    platforms/linux/linuxcryptosettings.cpp
    platforms/linux/linuxdependencies.cpp
    platforms/linux/linuxnetworkwatcher.cpp
    platforms/linux/linuxnetworkwatcherworker.cpp
    platforms/linux/linuxpingsender.cpp
    platforms/linux/linuxsystemtraynotificationhandler.cpp
    systemtraynotificationhandler.cpp
    tasks/authenticate/desktopauthenticationlistener.cpp
)

# Linux platform header files
target_sources(mozillavpn PRIVATE
    eventlistener.h
    platforms/linux/backendlogsobserver.h
    platforms/linux/dbusclient.h
    platforms/linux/linuxappimageprovider.h
    platforms/linux/linuxapplistprovider.h
    platforms/linux/linuxcontroller.h
    platforms/linux/linuxdependencies.h
    platforms/linux/linuxnetworkwatcher.h
    platforms/linux/linuxnetworkwatcherworker.h
    platforms/linux/linuxpingsender.h
    platforms/linux/linuxsystemtraynotificationhandler.h
    systemtraynotificationhandler.h
    tasks/authenticate/desktopauthenticationlistener.h
)

# Linux daemon source files
target_sources(mozillavpn PRIVATE
    ../3rdparty/wireguard-tools/contrib/embeddable-wg-library/wireguard.c
    daemon/daemon.cpp
    platforms/linux/daemon/apptracker.cpp
    platforms/linux/daemon/dbusservice.cpp
    platforms/linux/daemon/dnsutilslinux.cpp
    platforms/linux/daemon/iputilslinux.cpp
    platforms/linux/daemon/linuxdaemon.cpp
    platforms/linux/daemon/pidtracker.cpp
    platforms/linux/daemon/polkithelper.cpp
    platforms/linux/daemon/wireguardutilslinux.cpp
)

# Linux daemon header files
target_sources(mozillavpn PRIVATE
    ../3rdparty/wireguard-tools/contrib/embeddable-wg-library/wireguard.h
    daemon/interfaceconfig.h
    daemon/daemon.h
    daemon/dnsutils.h
    daemon/iputils.h
    daemon/wireguardutils.h
    platforms/linux/daemon/apptracker.h
    platforms/linux/daemon/dbusservice.h
    platforms/linux/daemon/dbustypeslinux.h
    platforms/linux/daemon/dnsutilslinux.h
    platforms/linux/daemon/iputilslinux.h
    platforms/linux/daemon/pidtracker.h
    platforms/linux/daemon/polkithelper.h
    platforms/linux/daemon/wireguardutilslinux.h
)

add_definitions(-DPROTOCOL_VERSION=\"1\")

set(GENERATED_SOURCES)
qt6_add_dbus_interface(GENERATED_SOURCES platforms/linux/daemon/org.mozilla.vpn.dbus.xml dbus_interface)
qt6_add_dbus_adaptor(GENERATED_SOURCES
                     platforms/linux/daemon/org.mozilla.vpn.dbus.xml
                     platforms/linux/daemon/dbusservice.h
                     DBusService
                     dbus_adaptor)
target_sources(mozillavpn PRIVATE ${GENERATED_SOURCES})

include(cmake/golang.cmake)
add_go_library(mozillavpn ../linux/netfilter/netfilter.go)

include(GNUInstallDirs)
install(FILES ../linux/extra/MozillaVPN.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)

install(FILES ../linux/extra/MozillaVPN-startup.desktop
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR})

install(FILES ../linux/extra/icons/16x16/mozillavpn.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/16x16/apps)

install(FILES ../linux/extra/icons/32x32/mozillavpn.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/32x32/apps)

install(FILES ../linux/extra/icons/48x48/mozillavpn.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/48x48/apps)

add_definitions(-DMVPN_ICON_PATH=\"${CMAKE_INSTALL_DATADIR}/icons/hicolor/64x64/apps/mozillavpn.png\")
install(FILES ../linux/extra/icons/64x64/mozillavpn.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/64x64/apps)

install(FILES ../linux/extra/icons/128x128/mozillavpn.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/128x128/apps)

install(FILES platforms/linux/org.mozillavpn.policy
    DESTINATION ${CMAKE_INSTALL_DATADIR}/polkit-1/actions)

install(FILES platforms/linux/daemon/org.mozilla.vpn.conf
    DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/system.d)

install(FILES platforms/linux/daemon/org.mozilla.vpn.dbus.service
    DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/system-services)

install(FILES ../linux/mozillavpn.service
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/systemd/system)

#ORIG_MOZILLAVPN_JSON = $$PWD/../../../extension/manifests/linux/mozillavpn.json
#manifestFile.input = ORIG_MOZILLAVPN_JSON
#manifestFile.output = $${OBJECTS_DIR}/mozillavpn.json
#manifestFile.commands = @python3 -c \'with open(\"$$ORIG_MOZILLAVPN_JSON\") as fin, open(\"$$manifestFile.output\", \"w\") as fout: print(\"\".join(fin).replace(\"/usr/lib/\", \"$${LIBPATH}/\"), end=\"\", file=fout)\'
#manifestFile.CONFIG = target_predeps no_link
#QMAKE_EXTRA_COMPILERS += manifestFile

#manifestFirefox.files = $$manifestFile.output
#manifestFirefox.path = $${LIBPATH}/mozilla/native-messaging-hosts
#manifestFirefox.depends = $$manifestFile.output
#manifestFirefox.CONFIG = no_check_exist
#INSTALLS += manifestFirefox

#manifestChrome.files = $$manifestFile.output
#manifestChrome.path = $${ETCPATH}/opt/chrome/native-messaging-hosts
#manifestChrome.depends = $$manifestFile.output
#manifestChrome.CONFIG = no_check_exist
#INSTALLS += manifestChrome

#manifestChromium.files = $$manifestFile.output
#manifestChromium.path = $${ETCPATH}/chromium/native-messaging-hosts
#manifestChromium.depends = $$manifestFile.output
#manifestChromium.CONFIG = no_check_exist
#INSTALLS += manifestChromium

#browserBridge.files = $$PWD/../../../extension/bridge/target/release/mozillavpnnp
#browserBridge.path = $${LIBPATH}/mozillavpn
#browserBridge.CONFIG = no_check_exist executable
#INSTALLS += browserBridge
