# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

qt_add_library(libSocks5proxy STATIC)
set_target_properties(libSocks5proxy PROPERTIES FOLDER "Libs")
target_link_libraries(libSocks5proxy PUBLIC Qt6::Core Qt6::Network)

target_sources(libSocks5proxy PRIVATE
    socks5.h
    socks5.cpp
    socks5client.cpp
    socks5client.h
)


target_include_directories(libSocks5proxy PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
