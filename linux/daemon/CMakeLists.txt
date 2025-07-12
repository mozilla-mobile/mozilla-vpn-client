# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

qt_add_executable(daemon MANUAL_FINALIZATION)

set_target_properties(daemon PROPERTIES
    OUTPUT_NAME "mozillavpn-daemon"
)
target_include_directories(daemon PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(daemon PRIVATE Qt6::Core Qt6::Network)
target_link_libraries(daemon PRIVATE mzutils mzdaemon)

find_package(Qt6 REQUIRED COMPONENTS DBus)
target_link_libraries(daemon PRIVATE Qt6::DBus)

# Link to libcap
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCAP REQUIRED IMPORTED_TARGET libcap)
if (QT_FEATURE_static)
    target_link_libraries(daemon PRIVATE ${LIBCAP_STATIC_LIBRARIES})
    target_include_directories(daemon PRIVATE ${LIBCAP_STATIC_INCLUDE_DIRS})
    target_compile_options(daemon PRIVATE ${LIBCAP_STATIC_CFLAGS})
else()
    target_link_libraries(daemon PRIVATE PkgConfig::LIBCAP)
endif()

target_sources(daemon PRIVATE
    ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-tools/contrib/embeddable-wg-library/wireguard.c
    ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-tools/contrib/embeddable-wg-library/wireguard.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apptracker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apptracker.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dbusservice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dbusservice.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dnsmetatypes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dnsutilslinux.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dnsutilslinux.h
    ${CMAKE_CURRENT_SOURCE_DIR}/iputilslinux.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/iputilslinux.h
    ${CMAKE_CURRENT_SOURCE_DIR}/linuxfirewall.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/linuxfirewall.h
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wireguardutilslinux.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wireguardutilslinux.h
)

set(DBUS_GENERATED_SOURCES)
qt_add_dbus_adaptor(DBUS_GENERATED_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/org.mozilla.vpn.dbus.xml
    ${CMAKE_CURRENT_SOURCE_DIR}/dbusservice.h
    ""
    dbus_adaptor)
target_sources(daemon PRIVATE ${DBUS_GENERATED_SOURCES})

target_compile_options(daemon PRIVATE -DPROTOCOL_VERSION=\"1\")

include(${CMAKE_SOURCE_DIR}/scripts/cmake/golang.cmake)
add_go_library(netfilter ${CMAKE_SOURCE_DIR}/linux/netfilter/netfilter.go)
target_link_libraries(daemon PRIVATE netfilter)

include(GNUInstallDirs)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/org.mozilla.vpn.conf
    DESTINATION /usr/share/dbus-1/system.d)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/org.mozilla.vpn.dbus.service.in
    ${CMAKE_CURRENT_BINARY_DIR}/org.mozilla.vpn.dbus.service)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.mozilla.vpn.dbus.service
    DESTINATION /usr/share/dbus-1/system-services)

pkg_check_modules(SYSTEMD systemd)
if("${SYSTEMD_FOUND}" EQUAL 1)
    pkg_get_variable(SYSTEMD_UNIT_DIR systemd systemdsystemunitdir)
else()
    set(SYSTEMD_UNIT_DIR /lib/systemd/system)
endif()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mozillavpn.service.in
    ${CMAKE_CURRENT_BINARY_DIR}/mozillavpn.service)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/mozillavpn.service
    DESTINATION ${SYSTEMD_UNIT_DIR})

install(SCRIPT ${CMAKE_SOURCE_DIR}/scripts/linux/postinst.cmake)

qt_finalize_target(daemon)
