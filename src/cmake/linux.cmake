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
    modules/vpn/platforms/linux/backendlogsobserver.cpp
    modules/vpn/platforms/linux/backendlogsobserver.h
    modules/vpn/platforms/linux/dbusclient.cpp
    modules/vpn/platforms/linux/dbusclient.h
    modules/vpn/platforms/linux/linuxappimageprovider.cpp
    modules/vpn/platforms/linux/linuxappimageprovider.h
    modules/vpn/platforms/linux/linuxapplistprovider.cpp
    modules/vpn/platforms/linux/linuxapplistprovider.h
    modules/vpn/platforms/linux/linuxcontroller.cpp
    modules/vpn/platforms/linux/linuxcontroller.h
    modules/vpn/platforms/linux/linuxnetworkwatcher.cpp
    modules/vpn/platforms/linux/linuxnetworkwatcher.h
    modules/vpn/platforms/linux/linuxnetworkwatcherworker.cpp
    modules/vpn/platforms/linux/linuxnetworkwatcherworker.h
    modules/vpn/platforms/linux/linuxpingsender.cpp
    modules/vpn/platforms/linux/linuxpingsender.h
    platforms/linux/linuxcryptosettings.cpp
    platforms/linux/linuxdependencies.cpp
    platforms/linux/linuxdependencies.h
    platforms/linux/linuxsystemtraynotificationhandler.cpp
    platforms/linux/linuxsystemtraynotificationhandler.h
)

# Linux daemon source files
target_sources(mozillavpn PRIVATE
    ../3rdparty/wireguard-tools/contrib/embeddable-wg-library/wireguard.c
    ../3rdparty/wireguard-tools/contrib/embeddable-wg-library/wireguard.h
    modules/vpn/daemon/daemon.cpp
    modules/vpn/daemon/daemon.h
    modules/vpn/daemon/dnsutils.h
    modules/vpn/daemon/interfaceconfig.h
    modules/vpn/daemon/iputils.h
    modules/vpn/daemon/wireguardutils.h
    modules/vpn/platforms/linux/daemon/apptracker.cpp
    modules/vpn/platforms/linux/daemon/apptracker.h
    modules/vpn/platforms/linux/daemon/dbusservice.cpp
    modules/vpn/platforms/linux/daemon/dbusservice.h
    modules/vpn/platforms/linux/daemon/dbustypeslinux.h
    modules/vpn/platforms/linux/daemon/dnsutilslinux.cpp
    modules/vpn/platforms/linux/daemon/dnsutilslinux.h
    modules/vpn/platforms/linux/daemon/iputilslinux.cpp
    modules/vpn/platforms/linux/daemon/iputilslinux.h
    modules/vpn/platforms/linux/daemon/linuxdaemon.cpp
    modules/vpn/platforms/linux/daemon/pidtracker.cpp
    modules/vpn/platforms/linux/daemon/pidtracker.h
    modules/vpn/platforms/linux/daemon/polkithelper.cpp
    modules/vpn/platforms/linux/daemon/polkithelper.h
    modules/vpn/platforms/linux/daemon/wireguardutilslinux.cpp
    modules/vpn/platforms/linux/daemon/wireguardutilslinux.h
)

add_definitions(-DPROTOCOL_VERSION=\"1\")

include(cmake/signature.cmake)

set(DBUS_GENERATED_SOURCES)
qt_add_dbus_interface(DBUS_GENERATED_SOURCES modules/vpn/platforms/linux/daemon/org.mozilla.vpn.dbus.xml dbus_interface)
qt_add_dbus_adaptor(DBUS_GENERATED_SOURCES
                    modules/vpn/platforms/linux/daemon/org.mozilla.vpn.dbus.xml
                    modules/vpn/platforms/linux/daemon/dbusservice.h
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
install(FILES modules/vpn/platforms/linux/daemon/org.mozilla.vpn.policy
    DESTINATION ${POLKIT_POLICY_DIR})

install(FILES modules/vpn/platforms/linux/daemon/org.mozilla.vpn.conf
    DESTINATION /usr/share/dbus-1/system.d)

install(FILES modules/vpn/platforms/linux/daemon/org.mozilla.vpn.dbus.service
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
