# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_definitions(-DWIN32_LEAN_AND_MEAN)

set_target_properties(mozillavpn PROPERTIES OUTPUT_NAME "Mozilla VPN")

#CONFIG(debug, debug|release) {
#    QMAKE_CXXFLAGS += /Z7 /ZI /FdMozillaVPN.PDB /DEBUG
#    QMAKE_LFLAGS_WINDOWS += /DEBUG
#}

#CONFIG += embed_manifest_exe

#RC_ICONS = $$PWD/../../ui/resources/logo.ico

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
    tasks/authenticate/desktopauthenticationlistener.cpp
    systemtraynotificationhandler.cpp
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
    tasks/authenticate/desktopauthenticationlistener.h
    platforms/windows/windowsstartatbootwatcher.h
    systemtraynotificationhandler.h
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

install(TARGETS mozillavpn DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES $<TARGET_PDB_FILE:mozillavpn> DESTINATION  ${CMAKE_INSTALL_PREFIX} OPTIONAL)

cmake_path(CONVERT "$ENV{VCToolsRedistDir}" TO_CMAKE_PATH_LIST VC_TOOLS_REDIST_PATH)
install(FILES ${VC_TOOLS_REDIST_PATH}/MergeModules/Microsoft_VC142_CRT_x64.msm DESTINATION ${CMAKE_INSTALL_PREFIX})

install(FILES
    ../windows/split-tunnel/mullvad-split-tunnel.cat
    ../windows/split-tunnel/mullvad-split-tunnel.inf
    ../windows/split-tunnel/mullvad-split-tunnel.sys
    ../windows/split-tunnel/WdfCoinstaller01011.dll
    ../extension/manifests/windows/mozillavpn.json
    DESTINATION ${CMAKE_INSTALL_PREFIX}
)

## TODO: Are these still needed? It's not clear.
#libssl.files = $$PWD/../../../libssl-1_1-x64.dll
#libssl.path = $$PWD/../../../unsigned/
#INSTALLS += libssl
#
#libcrypto.files = $$PWD/../../../libcrypto-1_1-x64.dll
#libcrypto.path = $$PWD/../../../unsigned/
#INSTALLS += libcrypto
#
