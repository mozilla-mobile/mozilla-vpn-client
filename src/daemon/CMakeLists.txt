# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(mozillavpn_daemon STATIC)

target_sources(mozillavpn_daemon INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/daemon.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/daemon.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dnsutils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/iputils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms
    ${CMAKE_CURRENT_SOURCE_DIR}/wireguardutils.h
)

if(${MZ_PLATFORM_NAME} STREQUAL "macos")
    target_sources(mozillavpn_daemon INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/daemonaccesscontrol.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/daemonaccesscontrol.h
        ${CMAKE_CURRENT_SOURCE_DIR}/daemonlocalserver.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/daemonlocalserver.h
        ${CMAKE_CURRENT_SOURCE_DIR}/daemonlocalserverconnection.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/daemonlocalserverconnection.h
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/dnsutilsmacos.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/dnsutilsmacos.h
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/iputilsmacos.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/iputilsmacos.h
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macosdaemon.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macosdaemon.h
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macosroutemonitor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macosroutemonitor.h
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/wireguardutilsmacos.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/wireguardutilsmacos.h
    )

    include(${CMAKE_SOURCE_DIR}/scripts/cmake/golang.cmake)
    # Build the Wireguard Go tunnel
    file(GLOB_RECURSE WIREGUARD_GO_DEPS ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-go/*.go)
    set(WIREGUARD_GO_ENV
        GOCACHE=${CMAKE_BINARY_DIR}/go-cache
        CC=${CMAKE_C_COMPILER}
        CXX=${CMAKE_CXX_COMPILER}
        GOROOT=${GOLANG_GOROOT}
        GOOS=darwin
        CGO_ENABLED=1
        GO111MODULE=on
        CGO_CFLAGS='-g -O3 -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} -isysroot ${OSX_SDK_PATH}'
        CGO_LDFLAGS='-g -O3 -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} -isysroot ${OSX_SDK_PATH}'
    )

    if(CMAKE_OSX_ARCHITECTURES)
        foreach(OSXARCH ${CMAKE_OSX_ARCHITECTURES})
            string(REPLACE "x86_64" "amd64" GOARCH ${OSXARCH})
            add_custom_command(
                OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/wireguard-go-${OSXARCH}
                COMMENT "Building wireguard-go for ${OSXARCH}"
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-go
                DEPENDS
                    ${WIREGUARD_GO_DEPS}
                    ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-go/go.mod
                    ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-go/go.sum
                COMMAND ${CMAKE_COMMAND} -E env ${WIREGUARD_GO_ENV} GOARCH=${GOARCH}
                        ${GOLANG_BUILD_TOOL} build -buildmode exe -buildvcs=false -trimpath -v
                            -o ${CMAKE_CURRENT_BINARY_DIR}/wireguard-go-${OSXARCH}
            )
            list(APPEND WG_GO_ARCH_BUILDS ${CMAKE_CURRENT_BINARY_DIR}/wireguard-go-${OSXARCH})
        endforeach()

        add_custom_target(build_wireguard_go
            COMMENT "Building wireguard-go"
            DEPENDS ${WG_GO_ARCH_BUILDS}
            COMMAND lipo -create -output ${CMAKE_CURRENT_BINARY_DIR}/wireguard-go ${WG_GO_ARCH_BUILDS}
        )
    else()
        # This only builds for the host architecture.
        add_custom_target(build_wireguard_go
            COMMENT "Building wireguard-go"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-go
            DEPENDS
                ${WIREGUARD_GO_DEPS}
                ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-go/go.mod
                ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-go/go.sum
            COMMAND ${CMAKE_COMMAND} -E env ${WIREGUARD_GO_ENV}
                    ${GOLANG_BUILD_TOOL} build -buildmode exe -buildvcs=false -trimpath -v
                        -o ${CMAKE_CURRENT_BINARY_DIR}/wireguard-go
        )
    endif()

    add_dependencies(mozillavpn_daemon build_wireguard_go)

elseif(${MZ_PLATFORM_NAME} STREQUAL "windows")
    target_sources(mozillavpn_daemon INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/daemonaccesscontrol.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/daemonaccesscontrol.h
        ${CMAKE_CURRENT_SOURCE_DIR}/daemonlocalserver.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/daemonlocalserver.h
        ${CMAKE_CURRENT_SOURCE_DIR}/daemonlocalserverconnection.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/daemonlocalserverconnection.h
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/dnsutilswindows.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/dnsutilswindows.h
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/windowsdaemon.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/windowsdaemon.h
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/windowsfirewall.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/windowsfirewall.h
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/windowsroutemonitor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/windowsroutemonitor.h
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/windowssplittunnel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/windowssplittunnel.h
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/windowstunnellogger.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/windowstunnellogger.h
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/windowstunnelservice.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/windowstunnelservice.h
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/wireguardutilswindows.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/wireguardutilswindows.h
    )

    include(${CMAKE_SOURCE_DIR}/scripts/cmake/golang.cmake)
elseif(${MZ_PLATFORM_NAME} STREQUAL "linux")
    target_sources(mozillavpn_daemon INTERFACE
        ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-tools/contrib/embeddable-wg-library/wireguard.c
        ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-tools/contrib/embeddable-wg-library/wireguard.h
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/linux/apptracker.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/linux/apptracker.h
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/linux/dbusservice.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/linux/dbusservice.h
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/linux/dbustypeslinux.h
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/linux/dnsutilslinux.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/linux/dnsutilslinux.h
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/linux/iputilslinux.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/linux/iputilslinux.h
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/linux/linuxdaemon.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/linux/pidtracker.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/linux/pidtracker.h
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/linux/wireguardutilslinux.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/linux/wireguardutilslinux.h
    )

    find_package(Qt6 REQUIRED COMPONENTS DBus)
    target_link_libraries(mozillavpn_daemon PRIVATE Qt6::DBus)

    add_definitions(-DPROTOCOL_VERSION=\"1\")
    set(DBUS_GENERATED_SOURCES)
    qt_add_dbus_interface(DBUS_GENERATED_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/linux/daemon/org.mozilla.vpn.dbus.xml dbus_interface)
    qt_add_dbus_adaptor(DBUS_GENERATED_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/linux/daemon/org.mozilla.vpn.dbus.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/linux/daemon/dbusservice.h
        ""
        dbus_adaptor)
    target_sources(mozillavpn_daemon PRIVATE ${DBUS_GENERATED_SOURCES})
endif()

target_include_directories(mozillavpn_daemon PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/..
)

# By making this public, any target which links to this target
# will add this directory to their include directories list.
target_include_directories(mozillavpn_daemon PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(mozillavpn_daemon PUBLIC Qt6::Core)

target_link_libraries(mozillavpn_daemon PUBLIC
    mozillavpn_utilities
    mozillavpn_logging
)
