# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

find_package(Qt6 REQUIRED COMPONENTS DBus)
target_link_libraries(mozillavpn PRIVATE Qt6::DBus)

find_package(PkgConfig REQUIRED)
pkg_check_modules(libsecret REQUIRED IMPORTED_TARGET libsecret-1)
target_link_libraries(mozillavpn PRIVATE PkgConfig::libsecret)

# Linux platform source files
target_sources(mozillavpn PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/backendlogsobserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/backendlogsobserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/dbusclient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/dbusclient.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/linuxappimageprovider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/linuxappimageprovider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/linuxapplistprovider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/linuxapplistprovider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/linuxcontroller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/linuxcontroller.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/linuxdependencies.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/linuxdependencies.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/linuxnetworkwatcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/linuxnetworkwatcher.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/linuxnetworkwatcherworker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/linuxnetworkwatcherworker.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/linuxpingsender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/linuxpingsender.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/linuxsystemtraynotificationhandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/linuxsystemtraynotificationhandler.h
)

# Linux daemon source files
target_sources(mozillavpn PRIVATE
   ../3rdparty/wireguard-tools/contrib/embeddable-wg-library/wireguard.c
   ../3rdparty/wireguard-tools/contrib/embeddable-wg-library/wireguard.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/daemon/daemon.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/daemon/daemon.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/daemon/dnsutils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/daemon/iputils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/daemon/wireguardutils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/daemon/apptracker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/daemon/apptracker.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/daemon/dbusservice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/daemon/dbusservice.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/daemon/dbustypeslinux.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/daemon/dnsutilslinux.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/daemon/dnsutilslinux.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/daemon/iputilslinux.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/daemon/iputilslinux.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/daemon/linuxdaemon.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/daemon/pidtracker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/daemon/pidtracker.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/daemon/wireguardutilslinux.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/daemon/wireguardutilslinux.h
)

add_definitions(-DPROTOCOL_VERSION=\"1\")

set(DBUS_GENERATED_SOURCES)
qt_add_dbus_interface(DBUS_GENERATED_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/daemon/org.mozilla.vpn.dbus.xml dbus_interface)
qt_add_dbus_adaptor(DBUS_GENERATED_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/daemon/org.mozilla.vpn.dbus.xml
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/linux/daemon/dbusservice.h
    ""
    dbus_adaptor)
target_sources(mozillavpn PRIVATE ${DBUS_GENERATED_SOURCES})

include(${CMAKE_SOURCE_DIR}/scripts/cmake/golang.cmake)
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

install(FILES apps/vpn/platforms/linux/daemon/org.mozilla.vpn.conf
    DESTINATION /usr/share/dbus-1/system.d)

configure_file(apps/vpn/platforms/linux/daemon/org.mozilla.vpn.dbus.service.in
    ${CMAKE_CURRENT_BINARY_DIR}/org.mozilla.vpn.dbus.service)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.mozilla.vpn.dbus.service
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
