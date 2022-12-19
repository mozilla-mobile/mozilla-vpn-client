# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(crashreporter INTERFACE)

set_property(TARGET crashreporter PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR})

# Crash reporter source files
target_sources(crashreporter INTERFACE
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
target_sources(crashreporter INTERFACE
    crash_resources.qrc
    crashui.qrc
)

# Windows Crash reporter sources
if(WIN32)
    target_sources(crashreporter INTERFACE
        platforms/windows/wincrashreporter.cpp
        platforms/windows/windowscrashclient.cpp
        platforms/windows/wincrashreporter.h
        platforms/windows/windowscrashclient.h
    )
endif()
