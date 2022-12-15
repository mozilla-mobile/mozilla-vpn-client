# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_definitions(-DWIN32_LEAN_AND_MEAN)

set_target_properties(mozillavpn PROPERTIES
    OUTPUT_NAME "Mozilla VPN"
    VERSION ${CMAKE_PROJECT_VERSION}
    WIN32_EXECUTABLE ON
)
# Todo: This will force the generation of a .pdb
# ignoring buildmode. we need to fix the relwithdebug target
# and then we can remove this :)
target_compile_options(mozillavpn
    PRIVATE
    $<$<CONFIG:Release>:/ZI>
)

# Generate the Windows version resource file.
configure_file(../windows/version.rc.in ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
target_sources(mozillavpn PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/version.rc)

# Windows platform source files
target_sources(mozillavpn PRIVATE
    commands/commandcrashreporter.cpp
    commands/commandcrashreporter.h
    eventlistener.cpp
    eventlistener.h
    modules/vpn/daemon/daemon.cpp
    modules/vpn/daemon/daemon.h
    modules/vpn/daemon/daemonlocalserver.cpp
    modules/vpn/daemon/daemonlocalserver.h
    modules/vpn/daemon/daemonlocalserverconnection.cpp
    modules/vpn/daemon/daemonlocalserverconnection.h
    modules/vpn/daemon/dnsutils.h
    modules/vpn/daemon/interfaceconfig.h
    modules/vpn/daemon/iputils.h
    modules/vpn/daemon/wireguardutils.h
    modules/vpn/localsocketcontroller.cpp
    modules/vpn/localsocketcontroller.h
    modules/vpn/platforms/windows/windowsapplistprovider.cpp
    modules/vpn/platforms/windows/windowsapplistprovider.h
    modules/vpn/platforms/windows/windowsappimageprovider.cpp
    modules/vpn/platforms/windows/windowsappimageprovider.h
    modules/vpn/platforms/windows/daemon/dnsutilswindows.cpp
    modules/vpn/platforms/windows/daemon/dnsutilswindows.h
    modules/vpn/platforms/windows/daemon/windowsdaemon.cpp
    modules/vpn/platforms/windows/daemon/windowsdaemon.h
    modules/vpn/platforms/windows/daemon/windowsdaemonserver.cpp
    modules/vpn/platforms/windows/daemon/windowsdaemonserver.h
    modules/vpn/platforms/windows/daemon/windowsdaemontunnel.cpp
    modules/vpn/platforms/windows/daemon/windowsdaemontunnel.h
    modules/vpn/platforms/windows/daemon/windowsroutemonitor.cpp
    modules/vpn/platforms/windows/daemon/windowsroutemonitor.h
    modules/vpn/platforms/windows/daemon/windowstunnellogger.cpp
    modules/vpn/platforms/windows/daemon/windowstunnellogger.h
    modules/vpn/platforms/windows/daemon/windowstunnelservice.cpp
    modules/vpn/platforms/windows/daemon/windowstunnelservice.h
    modules/vpn/platforms/windows/daemon/wireguardutilswindows.cpp
    modules/vpn/platforms/windows/daemon/wireguardutilswindows.h
    modules/vpn/platforms/windows/daemon/windowsfirewall.cpp
    modules/vpn/platforms/windows/daemon/windowsfirewall.h
    modules/vpn/platforms/windows/daemon/windowssplittunnel.cpp
    modules/vpn/platforms/windows/daemon/windowssplittunnel.h
    modules/vpn/platforms/windows/windowsnetworkwatcher.cpp
    modules/vpn/platforms/windows/windowsnetworkwatcher.h
    modules/vpn/platforms/windows/windowspingsender.cpp
    modules/vpn/platforms/windows/windowspingsender.h
    modules/vpn/platforms/windows/windowsservicemanager.cpp
    modules/vpn/platforms/windows/windowsservicemanager.h
    modules/vpn/wgquickprocess.cpp
    modules/vpn/wgquickprocess.h
    platforms/windows/windowscommons.cpp
    platforms/windows/windowscommons.h
    platforms/windows/windowscryptosettings.cpp
    platforms/windows/windowsstartatbootwatcher.cpp
    platforms/windows/windowsstartatbootwatcher.h
)

# Windows Qt6 UI workaround resources
if(Qt6_VERSION VERSION_GREATER_EQUAL 6.3.0)
    message(WARNING "Remove the Qt6 windows hack!")
else()
    target_sources(mozillavpn PRIVATE ui/qt6winhack.qrc)
endif()

include(cmake/golang.cmake)

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
            go build -buildmode c-shared -ldflags="-w -s" -trimpath -v -o "${CMAKE_CURRENT_BINARY_DIR}/balrog.dll"
)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_BINARY_DIR}/go-cache)
add_dependencies(mozillavpn balrogdll)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/balrog.dll DESTINATION .)

# Use Balrog for update support.
target_compile_definitions(mozillavpn PRIVATE MVPN_BALROG)
target_sources(mozillavpn PRIVATE
    update/balrog.cpp
    update/balrog.h
)

include(cmake/signature.cmake)

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

install(FILES ui/resources/logo.ico DESTINATION .)
