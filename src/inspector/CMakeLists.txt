# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(inspector INTERFACE)

target_sources(inspector INTERFACE
    inspector.cpp
    inspector.h

    tools/qquickinspector.cpp
    tools/qquickinspector.h
    tools/qqitemwrapper.h
    tools/qqitemwrapper.cpp
    tools/commandhandler.cpp
    tools/commandhandler.h
    tools/command.h

    tools/hotreloader.cpp
    tools/hotreloader.h

    server/factory.cpp
    server/factory.h
    server/websockettransport.cpp
    server/websockettransport.h
    server/websocketserver.cpp
    server/websocketserver.h
    legacy/commands.h
    legacy/commands.cpp
)
target_link_libraries(inspector INTERFACE Qt6::WebChannel)

target_sources(mozillavpn-sources INTERFACE
    legacy/commands.h
    legacy/commands.cpp
)
