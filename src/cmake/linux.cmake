# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

option(BUILD_FLATPAK "Build for Flatpak distribution" OFF)

find_package(Qt6 REQUIRED COMPONENTS DBus)
target_link_libraries(mozillavpn PRIVATE Qt6::DBus)

# Linux platform source files
target_sources(mozillavpn PRIVATE
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/dbustypes.h
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxappimageprovider.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxappimageprovider.h
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxapplistprovider.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxapplistprovider.h
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxnetworkwatcher.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxnetworkwatcher.h
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxnetworkwatcherworker.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxnetworkwatcherworker.h
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxpingsender.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxpingsender.h
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxsystemtraynotificationhandler.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxsystemtraynotificationhandler.h
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxutils.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxutils.h
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/xdgappearance.cpp
    ${CMAKE_SOURCE_DIR}/src/platforms/linux/xdgappearance.h
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
    # Link to libsecret
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBSECRET REQUIRED IMPORTED_TARGET libsecret-1)
    if (QT_FEATURE_static)
        target_link_libraries(mozillavpn PRIVATE ${LIBSECRET_STATIC_LIBRARIES})
        target_include_directories(mozillavpn PRIVATE ${LIBSECRET_STATIC_INCLUDE_DIRS})
        target_compile_options(mozillavpn PRIVATE ${LIBSECRET_STATIC_CFLAGS})
    else()
        target_link_libraries(mozillavpn PRIVATE PkgConfig::LIBSECRET)
    endif()

    qt_add_dbus_interface(DBUS_GENERATED_SOURCES
        ${CMAKE_SOURCE_DIR}/linux/daemon/org.mozilla.vpn.dbus.xml dbus_interface)

    target_sources(mozillavpn PRIVATE
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxcontroller.cpp
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxcontroller.h
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxcryptosettings.cpp
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/linuxcryptosettings.h
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/dbusclient.cpp
        ${CMAKE_SOURCE_DIR}/src/platforms/linux/dbusclient.h
        ${DBUS_GENERATED_SOURCES}
    )
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

add_custom_command(
    DEPENDS ${CMAKE_SOURCE_DIR}/linux/extra/org.mozilla.vpn.desktop.sh
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/org.mozilla.vpn.desktop
    COMMAND ${CMAKE_SOURCE_DIR}/linux/extra/org.mozilla.vpn.desktop.sh -b ${CMAKE_INSTALL_FULL_BINDIR} -o ${CMAKE_CURRENT_BINARY_DIR}/org.mozilla.vpn.desktop
)

add_custom_command(
    DEPENDS ${CMAKE_SOURCE_DIR}/linux/extra/org.mozilla.vpn.metainfo.sh
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/org.mozilla.vpn.metainfo.xml
    COMMAND ${CMAKE_SOURCE_DIR}/linux/extra/org.mozilla.vpn.metainfo.sh -d ${CMAKE_INSTALL_FULL_DATADIR} -o ${CMAKE_CURRENT_BINARY_DIR}/org.mozilla.vpn.metainfo.xml
)
target_sources(mozillavpn PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/org.mozilla.vpn.desktop
    ${CMAKE_CURRENT_BINARY_DIR}/org.mozilla.vpn.metainfo.xml
)
set_source_files_properties(
    ${CMAKE_CURRENT_BINARY_DIR}/org.mozilla.vpn.desktop
    ${CMAKE_CURRENT_BINARY_DIR}/org.mozilla.vpn.metainfo.xml
    PROPERTIES
        GENERATED TRUE
        HEADER_FILE_ONLY TRUE
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.mozilla.vpn.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.mozilla.vpn.metainfo.xml
    DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)

configure_file(${CMAKE_SOURCE_DIR}/linux/extra/org.mozilla.vpn.releases.xml.in
    ${CMAKE_CURRENT_BINARY_DIR}/org.mozilla.vpn.releases.xml)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.mozilla.vpn.releases.xml
    DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ui/resources/logo-generic.svg
    RENAME org.mozilla.vpn.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)

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
