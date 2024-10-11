# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

qt_add_executable(socksproxy 
    main.cpp
    verboselogger.cpp
    verboselogger.h
)

target_link_libraries(socksproxy PUBLIC 
    Qt6::Core 
    Qt6::Network
    libSocks5proxy
)

if(WIN32)
    target_compile_definitions(socksproxy PRIVATE PROXY_OS_WIN)
    target_sources(socksproxy PRIVATE
        windowsbypass.cpp
        windowsbypass.h
        winsvcthread.cpp
        winsvcthread.h)
    target_link_libraries(socksproxy PRIVATE Iphlpapi.lib)

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/socksproxy.exe
        DESTINATION .)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    target_sources(socksproxy PRIVATE
        linuxbypass.cpp
        linuxbypass.h)

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBCAP REQUIRED IMPORTED_TARGET libcap)
    target_link_libraries(socksproxy PRIVATE PkgConfig::LIBCAP)

    # TODO: not install that yet.
    #install(FILES ${CMAKE_CURRENT_BINARY_DIR}/socksproxy
    #    DESTINATION ${CMAKE_INSTALL_DATADIR}/socksproxy)
else()
    # TODO: This is currently pointless on macos, 
    # so no point in shipping it.
endif()


add_dependencies(mozillavpn socksproxy)
