# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(crashreporter STATIC)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets WebSockets)
find_package(Qt6 REQUIRED COMPONENTS Qml Quick)
target_include_directories(crashreporter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(crashreporter PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::WebSockets
    Qt6::Widgets
    Qt6::Qml
    Qt6::Quick
)
target_link_libraries(crashreporter PRIVATE nebula translations)

# Crash reporter source files
target_sources(crashreporter PRIVATE
    crashclient.cpp
    crashclient.h
    crashreporter.cpp
    crashreporter.h
    crashreporterapp.cpp
    crashreporterapp.h
    crashreporterfactory.cpp
    crashreporterfactory.h
    crashserverclient.cpp
    crashserverclient.h
    crashserverclientfactory.cpp
    crashserverclientfactory.h
    crashui.cpp
    crashui.h
    crashuploader.cpp
    crashuploader.h
)

# Crash reporter UI resources
target_sources(crashreporter PRIVATE
    crash_resources.qrc
    crashui.qrc
)

# Windows Crash reporter sources
if(WIN32)
    target_sources(crashreporter PRIVATE
        platforms/windows/wincrashreporter.cpp
        platforms/windows/windowscrashclient.cpp
        platforms/windows/wincrashreporter.h
        platforms/windows/windowscrashclient.h
    )
endif()
