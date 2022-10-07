# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

find_package(Qt6 REQUIRED COMPONENTS DBus)
target_link_libraries(mozillavpn PRIVATE Qt6::DBus)

find_package(PkgConfig REQUIRED)
pkg_check_modules(polkit REQUIRED IMPORTED_TARGET polkit-gobject-1)
target_link_libraries(mozillavpn PRIVATE PkgConfig::polkit)

# Linux platform source files
target_sources(mozillavpn PRIVATE
    eventlistener.cpp
    eventlistener.h
    platforms/linux/backendlogsobserver.cpp
    platforms/linux/backendlogsobserver.h
    platforms/linux/dbusclient.cpp
    platforms/linux/dbusclient.h
    platforms/linux/linuxappimageprovider.cpp
    platforms/linux/linuxappimageprovider.h
    platforms/linux/linuxapplistprovider.cpp
    platforms/linux/linuxapplistprovider.h
    platforms/linux/linuxcontroller.cpp
    platforms/linux/linuxcontroller.h
    platforms/linux/linuxcryptosettings.cpp
    platforms/linux/linuxdependencies.cpp
    platforms/linux/linuxdependencies.h
    platforms/linux/linuxnetworkwatcher.cpp
    platforms/linux/linuxnetworkwatcher.h
    platforms/linux/linuxnetworkwatcherworker.cpp
    platforms/linux/linuxnetworkwatcherworker.h
    platforms/linux/linuxpingsender.cpp
    platforms/linux/linuxpingsender.h
    platforms/linux/linuxsystemtraynotificationhandler.cpp
    platforms/linux/linuxsystemtraynotificationhandler.h
)

# Linux daemon source files
target_sources(mozillavpn PRIVATE
    ../3rdparty/wireguard-tools/contrib/embeddable-wg-library/wireguard.c
    ../3rdparty/wireguard-tools/contrib/embeddable-wg-library/wireguard.h
    daemon/daemon.cpp
    daemon/daemon.h
    daemon/dnsutils.h
    daemon/interfaceconfig.h
    daemon/iputils.h
    daemon/wireguardutils.h
    platforms/linux/daemon/apptracker.cpp
    platforms/linux/daemon/apptracker.h
    platforms/linux/daemon/dbusservice.cpp
    platforms/linux/daemon/dbusservice.h
    platforms/linux/daemon/dbustypeslinux.h
    platforms/linux/daemon/dnsutilslinux.cpp
    platforms/linux/daemon/dnsutilslinux.h
    platforms/linux/daemon/iputilslinux.cpp
    platforms/linux/daemon/iputilslinux.h
    platforms/linux/daemon/linuxdaemon.cpp
    platforms/linux/daemon/pidtracker.cpp
    platforms/linux/daemon/pidtracker.h
    platforms/linux/daemon/polkithelper.cpp
    platforms/linux/daemon/polkithelper.h
    platforms/linux/daemon/wireguardutilslinux.cpp
    platforms/linux/daemon/wireguardutilslinux.h
)

add_definitions(-DPROTOCOL_VERSION=\"1\")

include(cmake/signature.cmake)
include(cmake/vpnglean.cmake)

set(DBUS_GENERATED_SOURCES)
qt_add_dbus_interface(DBUS_GENERATED_SOURCES platforms/linux/daemon/org.mozilla.vpn.dbus.xml dbus_interface)
qt_add_dbus_adaptor(DBUS_GENERATED_SOURCES
                    platforms/linux/daemon/org.mozilla.vpn.dbus.xml
                    platforms/linux/daemon/dbusservice.h
                    ""
                    dbus_adaptor)
target_sources(mozillavpn PRIVATE ${DBUS_GENERATED_SOURCES})

include(cmake/golang.cmake)
add_go_library(netfilter ../linux/netfilter/netfilter.go)
target_link_libraries(mozillavpn PRIVATE netfilter)

include(GNUInstallDirs)
install(TARGETS mozillavpn)

configure_file(../linux/extra/mozillavpn.desktop.in
    ${CMAKE_CURRENT_BINARY_DIR}/mozillavpn.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/mozillavpn.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)

configure_file(../linux/extra/mozillavpn-startup.desktop.in
    ${CMAKE_CURRENT_BINARY_DIR}/mozillavpn-startup.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/mozillavpn-startup.desktop
    DESTINATION /etc/xdg/autostart)

install(FILES ../linux/extra/icons/16x16/mozillavpn.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/16x16/apps)

install(FILES ../linux/extra/icons/32x32/mozillavpn.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/32x32/apps)

install(FILES ../linux/extra/icons/48x48/mozillavpn.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/48x48/apps)

add_definitions(-DMVPN_ICON_PATH=\"${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/icons/hicolor/64x64/apps/mozillavpn.png\")
install(FILES ../linux/extra/icons/64x64/mozillavpn.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/64x64/apps)

install(FILES ../linux/extra/icons/128x128/mozillavpn.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/128x128/apps)

pkg_get_variable(POLKIT_POLICY_DIR polkit-gobject-1 policydir)
install(FILES platforms/linux/daemon/org.mozilla.vpn.policy
    DESTINATION ${POLKIT_POLICY_DIR})

install(FILES platforms/linux/daemon/org.mozilla.vpn.conf
    DESTINATION /usr/share/dbus-1/system.d)

install(FILES platforms/linux/daemon/org.mozilla.vpn.dbus.service
    DESTINATION /usr/share/dbus-1/system-services)

pkg_check_modules(SYSTEMD systemd)
if("${SYSTEMD_FOUND}" EQUAL 1)
    pkg_get_variable(SYSTEMD_UNIT_DIR systemd systemdsystemunitdir)
else()
    set(SYSTEMD_UNIT_DIR /lib/systemd/system)
endif()
configure_file(../linux/mozillavpn.service.in
    ${CMAKE_CURRENT_BINARY_DIR}/mozillavpn.service)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/mozillavpn.service
    DESTINATION ${SYSTEMD_UNIT_DIR})
