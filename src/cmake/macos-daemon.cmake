# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

qt_add_executable(daemon MANUAL_FINALIZATION)

set_target_properties(daemon PROPERTIES
    OUTPUT_NAME "${BUILD_OSX_APP_IDENTIFIER}.daemon"
    XCODE_GENERATE_SCHEME TRUE
)

mz_target_handle_warnings(daemon)

find_library(FW_FOUNDATION Foundation)
find_library(FW_NETWORK Network)
find_library(FW_SYSTEMCONFIG SystemConfiguration)

target_link_libraries(daemon PRIVATE ${FW_FOUNDATION} ${FW_NETWORK} ${FW_SYSTEMCONFIG})
target_link_libraries(daemon PRIVATE Qt6::Core Qt6::Network)
target_link_libraries(daemon PRIVATE mzutils)

# VPN client include paths
target_include_directories(daemon PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(daemon PRIVATE "$<$<CONFIG:Debug>:MZ_DEBUG>")
target_compile_definitions(daemon PRIVATE "MZ_$<UPPER_CASE:${MZ_PLATFORM_NAME}>")

target_sources(daemon PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/daemon/daemon.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/daemon/daemon.h
    ${CMAKE_CURRENT_SOURCE_DIR}/daemon/daemonaccesscontrol.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/daemon/daemonaccesscontrol.h
    ${CMAKE_CURRENT_SOURCE_DIR}/daemon/daemonerrors.h
    ${CMAKE_CURRENT_SOURCE_DIR}/daemon/daemonlocalserverconnection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/daemon/daemonlocalserverconnection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/daemon/dnsutils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/daemon/iputils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/daemon/wireguardutils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/daemon/dnsutilsmacos.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/daemon/dnsutilsmacos.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/daemon/iputilsmacos.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/daemon/iputilsmacos.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/daemon/macosdaemon.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/daemon/macosdaemon.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/daemon/macosdaemonmain.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/daemon/macosdaemonserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/daemon/macosdaemonserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/daemon/macosdnsmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/daemon/macosdnsmanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/daemon/macosroutemonitor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/daemon/macosroutemonitor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/daemon/wireguardutilsmacos.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/daemon/wireguardutilsmacos.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/daemon/xpcdaemonserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/daemon/xpcdaemonserver.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/xpcdaemonprotocol.h
)

# Embed the daemon property list.
configure_file(${CMAKE_SOURCE_DIR}/macos/app/daemon.plist.in daemon.plist)
target_link_options(daemon PRIVATE
    LINKER:-sectcreate,__TEXT,__info_plist,${CMAKE_CURRENT_BINARY_DIR}/daemon.plist
)

qt_finalize_target(daemon)
