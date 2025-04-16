# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

qt_add_executable(webext-bridge 
    main.cpp
    webextbridge.cpp
    webextbridge.h
    webexthandler.cpp
    webexthandler.h
    webextreader.cpp
    webextreader.h
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    target_sources(webext-bridge PRIVATE webexthandler_linux.cpp)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    enable_language(OBJC)
    target_sources(webext-bridge PRIVATE webexthandler_macos.mm)
endif()

target_link_libraries(webext-bridge PUBLIC Qt6::Core Qt6::Network)
