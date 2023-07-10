# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_definitions(-DWIN32_LEAN_AND_MEAN)

set_target_properties(mozillavpn PROPERTIES
    OUTPUT_NAME "Mozilla VPN"
    VERSION ${CMAKE_PROJECT_VERSION}
    WIN32_EXECUTABLE ON
)

# When used with MSVC, we find that RelWithDebInfo produces rather suboptimal
# compiler flags. If we want to generate debugging symbols, then we should
# add them to the Release configuration instead.
#
# See: https://gitlab.kitware.com/cmake/cmake/-/issues/20812
#
if(MSVC)
    target_compile_options(mozillavpn PRIVATE $<$<CONFIG:Release>:/Zi>)
    target_link_options(mozillavpn PRIVATE $<$<CONFIG:Release>:/debug>)
endif()

# Generate the Windows version resource file.
configure_file(../windows/version.rc.in ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
target_sources(mozillavpn PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/version.rc)

# Windows platform source files
target_sources(mozillavpn PRIVATE
     ${CMAKE_CURRENT_SOURCE_DIR}/daemon/daemon.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/daemon/daemon.h
     ${CMAKE_CURRENT_SOURCE_DIR}/daemon/daemonlocalserver.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/daemon/daemonlocalserver.h
     ${CMAKE_CURRENT_SOURCE_DIR}/daemon/daemonlocalserverconnection.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/daemon/daemonlocalserverconnection.h
     ${CMAKE_CURRENT_SOURCE_DIR}/daemon/dnsutils.h
     ${CMAKE_CURRENT_SOURCE_DIR}/daemon/interfaceconfig.h
     ${CMAKE_CURRENT_SOURCE_DIR}/daemon/iputils.h
     ${CMAKE_CURRENT_SOURCE_DIR}/daemon/wireguardutils.h
     ${CMAKE_CURRENT_SOURCE_DIR}/localsocketcontroller.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/localsocketcontroller.h
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/windowsapplistprovider.cpp 
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/windowsapplistprovider.h
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/windowsappimageprovider.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/windowsappimageprovider.h
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/daemon/dnsutilswindows.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/daemon/dnsutilswindows.h
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/daemon/windowsdaemon.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/daemon/windowsdaemon.h
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/daemon/windowsdaemonserver.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/daemon/windowsdaemonserver.h
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/daemon/windowsdaemontunnel.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/daemon/windowsdaemontunnel.h
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/daemon/windowsroutemonitor.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/daemon/windowsroutemonitor.h
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/daemon/windowstunnellogger.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/daemon/windowstunnellogger.h
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/daemon/windowstunnelservice.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/daemon/windowstunnelservice.h
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/daemon/wireguardutilswindows.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/daemon/wireguardutilswindows.h
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/daemon/windowsfirewall.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/daemon/windowsfirewall.h
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/daemon/windowssplittunnel.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/daemon/windowssplittunnel.h
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/windowsservicemanager.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/windowsservicemanager.h
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/windowscommons.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/windowscommons.h
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/windowsnetworkwatcher.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/windowsnetworkwatcher.h
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/windowspingsender.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/windowspingsender.h
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/windowsstartatbootwatcher.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/windowsstartatbootwatcher.h
)

# Windows Qt6 UI workaround resources
if(Qt6_VERSION VERSION_GREATER_EQUAL 6.3.0)
    message(WARNING "Remove the Qt6 windows hack!")
else()
    target_sources(mozillavpn PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ui/qt6winhack.qrc)
endif()

include(${CMAKE_SOURCE_DIR}/scripts/cmake/golang.cmake)

# Build the Balrog library as a DLL
add_custom_target(balrogdll ALL
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/balrog.dll ${CMAKE_CURRENT_BINARY_DIR}/balrog.h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/balrog
    COMMAND ${CMAKE_COMMAND} -E env 
                GOCACHE=${CMAKE_BINARY_DIR}/go-cache
                GOOS=windows CGO_ENABLED=1
                CC=gcc
                CGO_CFLAGS="-O3 -Wall -Wno-unused-function -Wno-switch -std=gnu11 -DWINVER=0x0601"
                CGO_LDFLAGS="-Wl,--dynamicbase -Wl,--nxcompat -Wl,--export-all-symbols -Wl,--high-entropy-va"
            go build -buildmode c-shared -buildvcs=false -ldflags="-w -s" -trimpath -v -o "${CMAKE_CURRENT_BINARY_DIR}/balrog.dll"
)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_BINARY_DIR}/go-cache)
add_dependencies(mozillavpn balrogdll)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/balrog.dll DESTINATION .)

# Use Balrog for update support.
target_compile_definitions(mozillavpn PRIVATE MVPN_BALROG)
target_sources(mozillavpn PRIVATE
     ${CMAKE_CURRENT_SOURCE_DIR}/update/balrog.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/update/balrog.h
)

install(TARGETS mozillavpn DESTINATION .)
install(FILES $<TARGET_PDB_FILE:mozillavpn> DESTINATION . OPTIONAL)

# Deploy Qt runtime dependencies during installation.
get_target_property(QT_QMLLINT_EXECUTABLE Qt6::qmllint LOCATION)
get_filename_component(QT_TOOL_PATH ${QT_QMLLINT_EXECUTABLE} PATH)
find_program(QT_WINDEPLOY_EXECUTABLE
    NAMES windeployqt
    PATHS ${QT_TOOL_PATH}
    NO_DEFAULT_PATH)
set(WINDEPLOYQT_FLAGS "--verbose 1 --no-translations --compiler-runtime --dir . --plugindir plugins")
install(CODE "execute_process(COMMAND \"${QT_WINDEPLOY_EXECUTABLE}\" \"$<TARGET_FILE:mozillavpn>\" ${WINDEPLOYQT_FLAGS} WORKING_DIRECTORY \${CMAKE_INSTALL_PREFIX})")

# Use the merge module that comes with our version of Visual Studio
cmake_path(CONVERT "$ENV{VCToolsRedistDir}" TO_CMAKE_PATH_LIST VC_TOOLS_REDIST_PATH)
install(FILES ${VC_TOOLS_REDIST_PATH}/MergeModules/Microsoft_VC${MSVC_TOOLSET_VERSION}_CRT_x64.msm
    DESTINATION . RENAME Microsoft_CRT_x64.msm)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ui/resources/logo.ico DESTINATION .)
