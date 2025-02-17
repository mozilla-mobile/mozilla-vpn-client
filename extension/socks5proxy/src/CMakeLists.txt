# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

qt_add_library(libSocks5proxy STATIC)
set_target_properties(libSocks5proxy PROPERTIES FOLDER "Libs")
target_link_libraries(libSocks5proxy PUBLIC 
    Qt6::Core 
    Qt6::Network)

add_dependencies(libSocks5proxy c-ares)

target_compile_definitions(libSocks5proxy PRIVATE CARES_STATICLIB)
target_include_directories(libSocks5proxy PUBLIC ${CMAKE_BINARY_DIR}/external/include)
target_link_directories( libSocks5proxy PUBLIC ${CMAKE_BINARY_DIR}/external/lib)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
   target_compile_definitions(libSocks5proxy PUBLIC QT_NO_DEBUG_OUTPUT)
endif()


target_sources(libSocks5proxy PRIVATE
    socks5.h
    socks5.cpp
    socks5connection.cpp
    socks5connection.h
    dnsresolver.h
    dnsresolver.cpp
)

if(WIN32)
    target_sources(libSocks5proxy PRIVATE 
        socks5local_windows.cpp
        dnsserverlookup_windows.cpp
        winutils.cpp
        winutils.h
    )
    target_link_libraries(libSocks5proxy PUBLIC cares.lib)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    target_sources(libSocks5proxy PRIVATE socks5local_linux.cpp dnsserverlookup_default.cpp)
else()
    target_sources(libSocks5proxy PRIVATE socks5local_default.cpp dnsserverlookup_default.cpp)
endif()

target_include_directories(libSocks5proxy PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
