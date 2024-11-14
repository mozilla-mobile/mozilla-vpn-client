# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

option(BUILD_FLATPAK "Build for Flatpak distribution" OFF)

find_package(Qt6 REQUIRED COMPONENTS DBus)
target_link_libraries(mozillavpn PRIVATE Qt6::DBus)

# Linux platform source files
target_sources(mozillavpn PRIVATE
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/backendlogsobserver.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/backendlogsobserver.h
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxappimageprovider.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxappimageprovider.h
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxapplistprovider.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxapplistprovider.h
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxdependencies.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxdependencies.h
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxnetworkwatcher.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxnetworkwatcher.h
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxnetworkwatcherworker.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxnetworkwatcherworker.h
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxpingsender.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxpingsender.h
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxsystemtraynotificationhandler.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxsystemtraynotificationhandler.h
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/xdgcryptosettings.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/xdgcryptosettings.h
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/xdgportal.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/xdgportal.h
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/xdgstartatbootwatcher.h
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/xdgstartatbootwatcher.cpp
)

# Resolving the parent window handle for the XDG desktop portal on Wayland
# needs the Gui internal header files on Qt 6.5.0 and later. Otherwise it
# only works for X11.
if(Qt6_VERSION VERSION_GREATER_EQUAL 6.5.0)
    target_link_libraries(mozillavpn PRIVATE Qt6::GuiPrivate)
endif()

if(NOT BUILD_FLATPAK)
    # Link to libcap and libsecret
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBBPF REQUIRED IMPORTED_TARGET libbpf)
    pkg_check_modules(LIBCAP REQUIRED IMPORTED_TARGET libcap)
    pkg_check_modules(LIBSECRET REQUIRED IMPORTED_TARGET libsecret-1)
    if (QT_FEATURE_static)
        target_link_libraries(mozillavpn PRIVATE ${LIBBPF_STATIC_LIBRARIES} ${LIBCAP_STATIC_LIBRARIES} ${LIBSECRET_STATIC_LIBRARIES})
        target_include_directories(mozillavpn PRIVATE ${LIBBPF_STATIC_INCLUDE_DIRS} ${LIBCAP_STATIC_INCLUDE_DIRS} ${LIBSECRET_STATIC_INCLUDE_DIRS})
        target_compile_options(mozillavpn PRIVATE ${LIBBPF_STATIC_CFLAGS} ${LIBCAP_STATIC_CFLAGS} ${LIBSECRET_STATIC_CFLAGS})
    else()
        target_link_libraries(mozillavpn PRIVATE PkgConfig::LIBCAP PkgConfig::LIBSECRET PkgConfig::LIBBPF)
    endif()

    target_sources(mozillavpn PRIVATE
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxcontroller.cpp
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxcontroller.h
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxcryptosettings.cpp
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxcryptosettings.h
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/dbusclient.cpp
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/dbusclient.h
    )

    # Linux daemon source files
    target_sources(mozillavpn PRIVATE
        ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-tools/contrib/embeddable-wg-library/wireguard.c
        ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-tools/contrib/embeddable-wg-library/wireguard.h
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/daemon/apptracker.cpp
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/daemon/apptracker.h
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/daemon/bpfsplittunnel.cpp
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/daemon/bpfsplittunnel.h
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/daemon/dbusservice.cpp
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/daemon/dbusservice.h
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/daemon/dbustypeslinux.h
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/daemon/dnsutilslinux.cpp
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/daemon/dnsutilslinux.h
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/daemon/iputilslinux.cpp
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/daemon/iputilslinux.h
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/daemon/linuxdaemon.cpp
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/daemon/linuxfirewall.cpp
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/daemon/linuxfirewall.h
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/daemon/wireguardutilslinux.cpp
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/daemon/wireguardutilslinux.h
    )

    target_compile_options(mozillavpn PRIVATE -DPROTOCOL_VERSION=\"1\")

    set(DBUS_GENERATED_SOURCES)
    qt_add_dbus_interface(DBUS_GENERATED_SOURCES
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/daemon/org.mozilla.vpn.dbus.xml dbus_interface)
    qt_add_dbus_adaptor(DBUS_GENERATED_SOURCES
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/daemon/org.mozilla.vpn.dbus.xml
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/daemon/dbusservice.h
        ""
        dbus_adaptor)
    target_sources(mozillavpn PRIVATE ${DBUS_GENERATED_SOURCES})

    include(${CMAKE_SOURCE_DIR}/scripts/cmake/golang.cmake)
    add_go_library(netfilter ${CMAKE_SOURCE_DIR}/linux/netfilter/netfilter.go)
    target_link_libraries(mozillavpn PRIVATE netfilter)
else()
    # Linux source files for sandboxed builds
    target_compile_definitions(mozillavpn PRIVATE MZ_FLATPAK)

    # Network Manager controller - experimental
    pkg_check_modules(libnm REQUIRED IMPORTED_TARGET libnm)
    target_link_libraries(mozillavpn PRIVATE PkgConfig::libnm)
    target_sources(mozillavpn PRIVATE
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/networkmanagerconnection.h
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/networkmanagerconnection.cpp
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/networkmanagercontroller.h
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/networkmanagercontroller.cpp
    )
endif()
include(GNUInstallDirs)
install(TARGETS mozillavpn)

configure_file(${CMAKE_SOURCE_DIR}/linux/extra/org.mozilla.vpn.desktop.in
    ${CMAKE_CURRENT_BINARY_DIR}/org.mozilla.vpn.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.mozilla.vpn.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)

install(FILES ${CMAKE_SOURCE_DIR}/linux/extra/icons/16x16/org.mozilla.vpn.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/16x16/apps)

install(FILES ${CMAKE_SOURCE_DIR}/linux/extra/icons/32x32/org.mozilla.vpn.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/32x32/apps)

install(FILES ${CMAKE_SOURCE_DIR}/linux/extra/icons/48x48/org.mozilla.vpn.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/48x48/apps)

install(FILES ${CMAKE_SOURCE_DIR}/linux/extra/icons/64x64/org.mozilla.vpn.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/64x64/apps)

install(FILES ${CMAKE_SOURCE_DIR}/linux/extra/icons/128x128/org.mozilla.vpn.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/128x128/apps)

if(NOT BUILD_FLATPAK)
    install(FILES ${CMAKE_SOURCE_DIR}/src/platforms/linux/daemon/org.mozilla.vpn.conf
        DESTINATION /usr/share/dbus-1/system.d)

    configure_file(${CMAKE_SOURCE_DIR}/src/platforms/linux/daemon/org.mozilla.vpn.dbus.service.in
        ${CMAKE_CURRENT_BINARY_DIR}/org.mozilla.vpn.dbus.service)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.mozilla.vpn.dbus.service
        DESTINATION /usr/share/dbus-1/system-services)

    pkg_check_modules(SYSTEMD systemd)
    if("${SYSTEMD_FOUND}" EQUAL 1)
        pkg_get_variable(SYSTEMD_UNIT_DIR systemd systemdsystemunitdir)
    else()
        set(SYSTEMD_UNIT_DIR /lib/systemd/system)
    endif()
    configure_file(${CMAKE_SOURCE_DIR}/linux/mozillavpn.service.in
        ${CMAKE_CURRENT_BINARY_DIR}/mozillavpn.service)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/mozillavpn.service
        DESTINATION ${SYSTEMD_UNIT_DIR})

    install(SCRIPT ${CMAKE_SOURCE_DIR}/scripts/linux/postinst.cmake)
endif()
