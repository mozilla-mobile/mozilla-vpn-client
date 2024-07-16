# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

qt_add_executable(socksproxy 
    main.cpp
)

target_link_libraries(socksproxy PUBLIC 
    Qt6::Core 
    Qt6::Network
    libSocks5proxy
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/socksproxy.exe
    DESTINATION .
)

add_dependencies(mozillavpn socksproxy)
