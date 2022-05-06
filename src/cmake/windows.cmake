# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_definitions(-DWIN32_LEAN_AND_MEAN)

set_target_properties(mozillavpn PROPERTIES
    OUTPUT_NAME "Mozilla VPN"
    VERSION ${CMAKE_PROJECT_VERSION}
    WIN32_EXECUTABLE ON
)

# Generate the Windows version resource file.
configure_file(../windows/version.rc.in ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
target_sources(mozillavpn PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/version.rc)

# Windows platform source files
target_sources(mozillavpn PRIVATE
    commands/commandcrashreporter.cpp
    daemon/daemon.cpp
    daemon/daemonlocalserver.cpp
    daemon/daemonlocalserverconnection.cpp
    eventlistener.cpp
    localsocketcontroller.cpp
    platforms/windows/windowsapplistprovider.cpp 
    platforms/windows/windowsappimageprovider.cpp
    platforms/windows/daemon/dnsutilswindows.cpp
    platforms/windows/daemon/windowsdaemon.cpp
    platforms/windows/daemon/windowsdaemonserver.cpp
    platforms/windows/daemon/windowsdaemontunnel.cpp
    platforms/windows/daemon/windowsroutemonitor.cpp
    platforms/windows/daemon/windowstunnellogger.cpp
    platforms/windows/daemon/windowstunnelservice.cpp
    platforms/windows/daemon/wireguardutilswindows.cpp
    platforms/windows/daemon/windowsfirewall.cpp
    platforms/windows/daemon/windowssplittunnel.cpp
    platforms/windows/windowsservicemanager.cpp
    platforms/windows/daemon/windowssplittunnel.cpp
    platforms/windows/windowscommons.cpp
    platforms/windows/windowscryptosettings.cpp
    platforms/windows/windowsnetworkwatcher.cpp
    platforms/windows/windowspingsender.cpp
    platforms/windows/windowsstartatbootwatcher.cpp
    wgquickprocess.cpp
)

# Windows platform header files
target_sources(mozillavpn PRIVATE
    commands/commandcrashreporter.h
    daemon/interfaceconfig.h
    daemon/daemon.h
    daemon/daemonlocalserver.h
    daemon/daemonlocalserverconnection.h
    daemon/dnsutils.h
    daemon/iputils.h
    daemon/wireguardutils.h
    eventlistener.h
    localsocketcontroller.h
    platforms/windows/windowsapplistprovider.h
    platforms/windows/windowsappimageprovider.h
    platforms/windows/daemon/dnsutilswindows.h
    platforms/windows/daemon/windowsdaemon.h
    platforms/windows/daemon/windowsdaemonserver.h
    platforms/windows/daemon/windowsdaemontunnel.h
    platforms/windows/daemon/windowsroutemonitor.h
    platforms/windows/daemon/windowstunnellogger.h
    platforms/windows/daemon/windowstunnelservice.h
    platforms/windows/daemon/wireguardutilswindows.h
    platforms/windows/daemon/windowsfirewall.h
    platforms/windows/daemon/windowssplittunnel.h
    platforms/windows/windowsservicemanager.h
    platforms/windows/windowscommons.h
    platforms/windows/windowsnetworkwatcher.h
    platforms/windows/windowspingsender.h
    platforms/windows/windowsstartatbootwatcher.h
    wgquickprocess.h
)

# Windows Qt6 UI workaround resources
if(${Qt6_VERSION} VERSION_GREATER_EQUAL 6.3.0)
    message(WARNING "Remove the Qt6 windows hack!")
elif(${Qt6_VERSION} VERSION_GREATER_EQUAL 6.0.0)
    target_sources(mozillavpn PRIVATE ui/qt6winhack.qrc)
endif()

include(cmake/golang.cmake)

# Enable Balrog for update support.
add_definitions(-DMVPN_BALROG)
add_go_library(mozillavpn ../balrog/balrog-api.go)
target_sources(mozillavpn PRIVATE
    update/balrog.cpp
    update/balrog.h
)

install(TARGETS mozillavpn DESTINATION .)
install(FILES $<TARGET_PDB_FILE:mozillavpn> DESTINATION . OPTIONAL)

## Deploy Qt runtime dependencies during installation.
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

install(FILES
    ../windows/split-tunnel/mullvad-split-tunnel.cat
    ../windows/split-tunnel/mullvad-split-tunnel.inf
    ../windows/split-tunnel/mullvad-split-tunnel.sys
    ../windows/split-tunnel/WdfCoinstaller01011.dll
    ../extension/manifests/windows/mozillavpn.json
    DESTINATION .
)

install(FILES ui/resources/logo.ico DESTINATION .)

## TODO: Are these still needed? It's not clear.
#libssl.files = $$PWD/../../../libssl-1_1-x64.dll
#libssl.path = $$PWD/../../../unsigned/
#INSTALLS += libssl
#
#libcrypto.files = $$PWD/../../../libcrypto-1_1-x64.dll
#libcrypto.path = $$PWD/../../../unsigned/
#INSTALLS += libcrypto
#
