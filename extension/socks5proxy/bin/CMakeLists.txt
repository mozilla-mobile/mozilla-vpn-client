# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

qt_add_executable(socksproxy 
    main.cpp
    sockslogger.cpp
    sockslogger.h
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
        winfwpolicy.cpp
        winfwpolicy.h
        winsvcthread.cpp
        winsvcthread.h
        )
    target_link_libraries(socksproxy PRIVATE Iphlpapi.lib)

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/socksproxy.exe
        DESTINATION .)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    target_compile_definitions(socksproxy PRIVATE PROXY_OS_LINUX)
    target_sources(socksproxy PRIVATE
        linuxbypass.cpp
        linuxbypass.h)

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBCAP REQUIRED IMPORTED_TARGET libcap)
    target_link_libraries(socksproxy PRIVATE PkgConfig::LIBCAP)

    # Install a Systemd service to run the proxy, if supported.
    pkg_check_modules(SYSTEMD systemd)
    if("${SYSTEMD_FOUND}" EQUAL 1)
        pkg_get_variable(SYSTEMD_UNIT_DIR systemd systemdsystemunitdir)
    elseif(NOT DEFINED ENV{container})
        set(SYSTEMD_UNIT_DIR /lib/systemd/system)
    endif()
    if(SYSTEMD_UNIT_DIR)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/socksproxy.service.in
            ${CMAKE_CURRENT_BINARY_DIR}/socksproxy.service)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/socksproxy.service
            DESTINATION ${SYSTEMD_UNIT_DIR})
    endif()

    install(TARGETS socksproxy)
else()
    # TODO: This is currently pointless on macos, 
    # so no point in shipping it.
endif()


add_dependencies(mozillavpn socksproxy)
