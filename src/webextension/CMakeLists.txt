# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(webextension STATIC
    baseadapter.h
    baseadapter.cpp
    connection.cpp
    connection.h
    server.cpp
    server.h
)
target_link_libraries(webextension PUBLIC 
    Qt6::Core
    Qt6::Network
)

target_sources(mozillavpn-sources INTERFACE
    vpnadapter.h
    vpnadapter.cpp
)
target_compile_definitions(mozillavpn-sources INTERFACE MVPN_WEBEXTENSION)
