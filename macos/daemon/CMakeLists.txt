# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

qt_add_executable(daemon MANUAL_FINALIZATION)

set_target_properties(daemon PROPERTIES
    OUTPUT_NAME "${BUILD_OSX_APP_IDENTIFIER}.daemon"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${BUILD_OSX_APP_IDENTIFIER}.daemon"
    XCODE_ATTRIBUTE_MARKETING_VERSION "${CMAKE_PROJECT_VERSION}"
    XCODE_GENERATE_SCHEME TRUE
)

mz_target_handle_warnings(daemon)

find_library(FW_FOUNDATION Foundation)
find_library(FW_NETWORK Network)
find_library(FW_SECURITY Security)
find_library(FW_SYSTEMCONFIG SystemConfiguration)

target_link_libraries(daemon PRIVATE ${FW_FOUNDATION} ${FW_NETWORK} ${FW_SECURITY} ${FW_SYSTEMCONFIG})
target_link_libraries(daemon PRIVATE Qt6::Core Qt6::Network)
target_link_libraries(daemon PRIVATE mzutils mzdaemon)

# VPN client include paths
target_include_directories(daemon PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(daemon PRIVATE "$<$<CONFIG:Debug>:MZ_DEBUG>")

target_sources(daemon PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/dnsutilsmacos.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dnsutilsmacos.h
    ${CMAKE_CURRENT_SOURCE_DIR}/iputilsmacos.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/iputilsmacos.h
    ${CMAKE_CURRENT_SOURCE_DIR}/macosdaemon.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/macosdaemon.h
    ${CMAKE_CURRENT_SOURCE_DIR}/macosdaemonserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/macosdaemonserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/macosdnsmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/macosdnsmanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/macosroutemonitor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/macosroutemonitor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wireguardutilsmacos.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wireguardutilsmacos.h
    ${CMAKE_CURRENT_SOURCE_DIR}/xpcdaemonprotocol.h
    ${CMAKE_CURRENT_SOURCE_DIR}/xpcdaemonserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/xpcdaemonserver.mm
)

# Embed the daemon property list.
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in daemon.plist)
if(XCODE)
    set_target_properties(daemon PROPERTIES
        XCODE_ATTRIBUTE_INFOPLIST_FILE ${CMAKE_CURRENT_BINARY_DIR}/daemon.plist
        XCODE_ATTRIBUTE_CREATE_INFOPLIST_SECTION_IN_BINARY YES
    )
else()
    target_link_options(daemon PRIVATE
        LINKER:-sectcreate,__TEXT,__info_plist,${CMAKE_CURRENT_BINARY_DIR}/daemon.plist
    )
endif()

include(${CMAKE_SOURCE_DIR}/scripts/cmake/osxtools.cmake)
osx_codesign_target(daemon)

qt_finalize_target(daemon)
