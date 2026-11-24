# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(mzdaemon STATIC
    daemon.cpp
    daemon.h
    daemonerrors.h
    daemonlocalserverconnection.cpp
    daemonlocalserverconnection.h
    dnsutils.h
    iputils.h
    wireguardutils.h
    mock/dnsutilsmock.cpp
    mock/dnsutilsmock.h
    mock/mockdaemon.cpp
    mock/mockdaemon.h
    mock/wireguardutilsmock.cpp
    mock/wireguardutilsmock.h
    obfuscator/obfuscator.h
)

# QProcess is not supported on these platforms.
if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten" AND NOT IOS)
    target_sources(mzdaemon PRIVATE
        obfuscator/qprocessobfuscator.cpp
        obfuscator/qprocessobfuscator.h
    )
endif()

target_include_directories(mzdaemon PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mzdaemon PUBLIC Qt6::Core Qt6::Network)
target_link_libraries(mzdaemon PUBLIC mzutils)
target_compile_definitions(mzdaemon PRIVATE "MZ_$<UPPER_CASE:${MZ_PLATFORM_NAME}>")

if(NOT CMAKE_CROSSCOMPILING)
    add_subdirectory(tests)
endif()
