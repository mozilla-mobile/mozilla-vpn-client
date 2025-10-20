# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

mz_add_new_module(
    TARGET_NAME mz_webextension
    SOURCES
        baseadapter.h
        baseadapter.cpp
        connection.cpp
        connection.h
        server.cpp
        server.h
        webextbridge.cpp
        webextbridge.h
        webextcommand.cpp
        webextcommand.h
        webexthandler.cpp
        webexthandler.h
        webextreader.cpp
        webextreader.h
    INCLUDE_DIRECTORIES 
        ${CMAKE_CURRENT_SOURCE_DIR}
    TEST_SOURCES
        tests/testbaseadapter.cpp
        tests/testbaseadapter.h
        tests/testconnection.cpp
        tests/testconnection.h
        tests/testserver.cpp
        tests/testserver.h
    EXTRA_DEPENDENCIES
        mzutils
    QT_DEPENDENCIES
        Core
        Network
)

if(WIN32)
    target_sources(mz_webextension PRIVATE webexthandler_windows.cpp)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    target_sources(mz_webextension PRIVATE webexthandler_linux.cpp)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    target_sources(mz_webextension PRIVATE webexthandler_macos.mm)

    find_library(FW_APPKIT AppKit)
    target_link_libraries(mz_webextension PRIVATE ${FW_APPKIT})
endif()
